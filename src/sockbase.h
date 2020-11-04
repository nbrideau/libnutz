#ifndef _SOCKBASE_H_INCLUDED
#define _SOCKBASE_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "addr.h"
#include "logger.h"

// TODO Where to put this functionality?
// TODO Recv/Send Frame Header Length Delimited (size_t length, size bytes)
// TODO Recv/Send Frame Footer Token  Delimited (char = '\0')
//
// TODO Callbacks on receiving messages?

class SockBase : public ClassLogger {
    public:
        SockBase(void);
        SockBase(Addr & addr);
        virtual ~SockBase(void);

    public:
        void     SetAddr(Addr & addr);
        Addr *   GetAddr(void);
        int      SockId(void);
        void     SetTimeout(int secs);

    public:
        virtual bool Close(void);
        virtual bool Create(void);
        virtual bool Connect(void);
        
        virtual bool Bind(void);
        virtual bool Setup(void);   // Server one shot
        virtual bool Listen(void);
        
        virtual bool Accept(SockBase * sock);

        virtual bool IsConnected(void);
        time_t       LastUpdate(void);
    
    public:
        // Override these to send and receive messages
        virtual bool RecvPacket(void  ** msg) = 0;
        virtual bool SendPacket(void   * msg) = 0;
        virtual void DeletePacket(void * msg) = 0; /**< Memory management */

        // 

        // Actual sending and receiving
        virtual int  Send(const byte * buff, size_t bytes);
        virtual int  Recv(byte * buff, size_t bytes);

    protected:
        Addr    m_addr;
        int     m_timeout; // seconds
        time_t  m_lastupdate;

    private:
        int     m_sock;
};
 
#endif
