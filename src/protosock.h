
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
        virtual bool RecvMsg(void ** msg);
        virtual bool SendMsg(void *  msg);

        // XXX I can't remember why exactly I did this.
        // Could have been a memory leak but I remember
        // something about library or thread boundaries.
        virtual void DeleteMsg(void * msg); 

    private:
        byte        m_buff[MAX_PACKET_SIZE];
        size_t      m_brecv;
};
 

#endif
