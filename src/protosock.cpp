
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
 * TODO Use buffer class?
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
    //Log(LL_DEBUG, "[SENDING] %s", buff.c_str()); // PP output? DebugString()?
    
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
    bool done = false;
    int  recved = 0;
    unsigned int  i;
    *recv = NULL;
    size_t size;
    
    // Receive length
    recved = Recv((byte *)size, sizeof(size_t));
    if (recved < 0) {
        throw Exception("[::RecvMsg] Error reading from socket");
    }
    if (recved == 0) {
        return false;
    }

    // Receive buffer TODO Something different.
    byte * buff = new byte[size]; // FIXME Memory!!!
    recved = Recv(buff, size);
    if (recved < 0) {
        throw Exception("[::RecvMsg] Error reading from socket");
    }
    if (recved == 0) {
        return false;
    }
    if (recved < size) {
        // TODO short reads. Put in a while loop...
    }

    // Build protobuf message
    // TODO
    // google::protobuf::Message * msg = new google::protobuf::Message()

    return true;
}

