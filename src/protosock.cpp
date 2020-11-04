
#include <google/protobuf/message.h>
#include "protosock.h"
#include "exception.h"

//#define DEBUG_PACKETS

/*****************************************************************************
 *
 *****************************************************************************/
ProtoSock::ProtoSock(void) {
    SetClassName("ProtoSock");
    m_brecv = 0;
    return;
}


/*****************************************************************************
 *
 *****************************************************************************/
ProtoSock::ProtoSock(Addr & addr) 
: SockBase(addr) {
    SetClassName("ProtoSock");
    m_brecv = 0;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
ProtoSock::~ProtoSock(void) {
    return;
}

void  ProtoSock::DeleteMsg(void * msg) {
    delete (google::protobuf::Message *)msg;
}

/*****************************************************************************
 * SEND
 * TODO Serialize and send as a single buffer?
 * TODO Use google::protobuf namespace
 * TODO Use something smaller than long unsigned int. 2 bytes? 
 *****************************************************************************/
bool ProtoSock::SendMsg(void * send) {
    std::string buff;
    google::protobuf::Message * msg = (google::protobuf::Message *)send;
    if (!msg) return false;
    
    if (!msg->SerializeToString(&buff)) {
        delete msg;
        return false;
    }

#ifdef DEBUG_PACKETS
    Log(LL_DEBUG, "SEND: <%s>", buff.c_str()); // FIXME Just send message type
#endif
    //Log(LL_DEBUG, "[SENDING] %s", buff.c_str()); // PP output?
    
    // Send the length the the buffer
    size_t size = msg->ByteSizeLong();
    if (Send((byte *)&size, sizeof(size_t)) != sizeof(size_t) 
        || Send((byte *)buff.c_str(), size) != size) {
        //Log(LL_ERROR, "%d != %d", rv, (int)msg->GetBuffLen());
        delete (google::protobuf::Message *)send;
        throw Exception("[::SendMsg] Msg not sent");
    }


    delete (google::protobuf::Message *)send;
    return true;
}


/*****************************************************************************
 * RECV
 *****************************************************************************/
bool ProtoSock::RecvMsg(void ** recv) {
    // TODO Recieve length + buffer[length] unserialize...
    /*
    bool done = false;
    int  recved = 0;
    unsigned int  i;
    *recv = NULL;

    // Make sure we did not get multiple messages in a single read
    for (i = 0; i < m_brecv; i++) {
        if (m_buff[i] == '\0') 
            return BuildMsg(recv, i);
    }
   
    // Lots of pointer arithmatic below
    while (!done) {
        recved = Recv(m_buff + m_brecv, MAX_PACKET_SIZE - m_brecv);
        if (m_brecv + recved >= MAX_PACKET_SIZE) {
            //m_buff[m_brecv + 1] = '\0';
            //Log(LL_ERROR, "Msg %s", m_buff);
            throw Exception("[::RecvMsg] Msg too large");
        }
        if (recved < 0) 
            throw Exception("[::RecvMsg] Error reading from socket");
        if (recved == 0)
            return false;

        // How many bytes we have recieved so far
        m_brecv += recved;

        // Check last few bytes for NULL terminator
        for (i = m_brecv - recved; i < m_brecv; i++) {
            if (m_buff[i] == '\0') 
                return BuildMsg(recv, i);
        }
    }
    */
    return false;
}

