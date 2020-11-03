
#ifndef _PROTOSOCK_H_INCLUDED
#define _PROTOSOCK_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "addr.h"
#include "sockbase.h"


// FIXME
#define MAX_PACKET_SIZE 1024

class ProtoSock : public SockBase {
    public:
        ProtoSock(void);
        ProtoSock(Addr & addr);
        virtual ~ProtoSock(void);

    public:
        virtual bool RecvPacket(void ** msg);
        virtual bool SendPacket(void *  msg);
        virtual void DeletePacket(void * msg);

    private:
        byte        m_buff[MAX_PACKET_SIZE];
        size_t      m_brecv;
};
 

#endif
