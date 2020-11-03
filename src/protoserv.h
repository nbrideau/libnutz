
#ifndef _PROTOSERV_H_INCLUDED
#define _PROTOSERV_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "servbase.h"
#include "protosock.h"

/** 
 */
class ServProto : public ServBase  {
    public:
        ServProto(Addr & addr, int maxclients = 0);
        virtual ~ServProto(void);
 
    public:
        // Class factories for overrides
        virtual SockBase * Create(void)  { return new ProtoSock(); }
};
 

class ProtoCon : public ServerCon {
    public:
        ProtoCon(ServBase * serv);
        virtual ~ProtoCon(void);
    
    // Message handling
    public:
        // Message pump
        virtual void ProcessMsg(void * msg);

        // May want to override these for cleanups etc...
        virtual void OnConnect(void)    {}
        virtual void OnDisconnect(void) {}
        virtual void OnMaxClients(void) {}
};

#endif
