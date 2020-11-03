
#include "protoserv.h"

/*****************************************************************************
 *
 *****************************************************************************/
ServProto::ServProto(Addr & addr, int maxclients) 
: ServBase(addr, maxclients) {
    SetClassName("ServProto");
    return;
}

ServProto::~ServProto(void) {
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
ProtoCon::ProtoCon(ServBase * serv) 
: ServerCon(serv) {
    SetClassName("ProtoCon");
    return;
}

ProtoCon::~ProtoCon(void) {
    return;
}


// Message pump
void ProtoCon::ProcessMsg(void * msg) {
    ServProto  * server  = (ServProto *)m_serv;
    
    /*
    google::Message  * message = (google::Message *)msg;
    google::Message  * rcmd    = NULL;
    // Message handling switch
    switch (message->GetCmdType()) {
        case GS_PING:
            rcmd = HandlePing((CmdPing *)message->GetCmd());
            break;
        default:
            break;
    }
    if (!rcmd) {
        Log(LL_ERROR, "Unhandled message in switch. %s",
            message->GetCmd()->GetCmdName(message->GetCmdType()));
        throw new Exception("Unhandled command");
    }
    ((ProtoSock *)m_sock)->SendPacket(new google::Message(rcmd, message->GetId()));
    */

    return;
}

