
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

void  ProtoSock::DeletePacket(void * msg) {
    // FIXME XXX
    //delete (google::Message *)msg;
}

/*****************************************************************************
 * SEND
 *****************************************************************************/
bool ProtoSock::SendPacket(void * send) {
    /*
    std::string msg;
    if (!send) return false;
    if (!((google::Message *)send)->SerializeToString(&msg)) 
        return false;

#ifdef DEBUG_PACKETS
    Log(LL_DEBUG, "SEND: <%s>", msg->GetFirstChild()->Name()); FIXME
#endif

    //Log(LL_DEBUG, "[SENDING] %s", msg.c_str());
    // TODO Send serialized length followed by serialized buffer
    int rv = Send(msg->GetBuff(), msg->GetBuffLen());
    if (rv != (int)msg->GetBuffLen()) {
        //Log(LL_ERROR, "%d != %d", rv, (int)msg->GetBuffLen());
        delete (google::Message *)send;
        throw Exception("[::SendPacket] Packet not sent");
    }
    delete (google::Message *)send;
    */
    return true;
}


/*****************************************************************************
 * RECV
 *****************************************************************************/
bool ProtoSock::RecvPacket(void ** recv) {
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
            //Log(LL_ERROR, "Message %s", m_buff);
            throw Exception("[::RecvPacket] Packet too large");
        }
        if (recved < 0) 
            throw Exception("[::RecvPacket] Error reading from socket");
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

