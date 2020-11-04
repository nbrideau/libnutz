
#ifndef _SERVBASE_H_INCLUDED
#define _SERVBASE_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "list.h"
#include "libnutz.h"
#include "thread.h"
#include "sockbase.h"

class ServerCon;

/** 
 * @brief A pool of worker threads spawned by accept 
 * This is a list of all our server connections to clients.
 */
class ServerConPool : public Thread {
    public:
        ServerConPool(void);
        virtual ~ServerConPool(void);

    public:
        bool Stop(pthread_t tid);
        bool Kill(pthread_t tid);
        bool Start(void *(*pthread_func)(void *), ServerCon * arg);
        void Pause(pthread_t tid);
        void Pause(pthread_t tid, unsigned int secs);
        void Resume(pthread_t tid);
        bool IsPaused(pthread_t tid);
        bool IsStarted(pthread_t tid);

        void Shutdown(void);
        int  GetNumThreads(void);
    
#ifndef THREAD_NO_DETACH
        bool    RemoveThread(pthread_t tid);
#endif
    protected:
        virtual void Run(void);

    protected:
        ServerCon * GetThread(pthread_t tid);

    protected:
        List<ServerCon> m_threads;  /**< List of threads */
};
 

class ServBase;
/** 
 * @class ServerCon
 *
 * This is a connection from a client to a server. 
 * It runs in it's own thread. Place all message handling
 * in a subclass of this. Real work happens here.
 *
 * Per thread storage and thread control
 */
class ServerCon : public Thread {
    friend class ServBase;
    friend class ServerConPool;
    public:
        ServerCon(ServBase * serv);
        virtual ~ServerCon(void);
    
    // Msg handling
    public:
        // Msg pump
        virtual void ProcessMsg(void * msg) = 0;

        // May want to override these for cleanups etc...
        virtual void OnConnect(void)    {}
        virtual void OnDisconnect(void) {}
        virtual void OnMaxClients(void) {}

    // Data access 
    public:
        ServBase   * Server(void);
        SockBase   * Socket(void);
        Addr       * GetAddr(void);
        pthread_t    ThreadId(void);

    // Socket Crap
    protected:
        bool Close(void)                { return m_sock->Close();          }
        void SetTimeout(int s)          { m_sock->SetTimeout(s);           }
        bool IsConnected(void)          { return m_sock->IsConnected();    }
        time_t LastUpdate(void)         { return m_sock->LastUpdate();     }
        bool RecvMsg(void ** msg)       { return m_sock->RecvMsg(msg);  }
        void DeleteMsg(void * msg)      { return m_sock->DeleteMsg(msg);}
        
    protected:
        // Setup thread and unpause
        void          SetThreadId(pthread_t tid);
        
    protected:
        void        Run(void) {} // Just using thread control 

    protected:
        ServBase   * m_serv;   /**< Base server */
        SockBase   * m_sock;   /**< Base socket */
};


/** 
 * @class ServBase 
 * @todo add conditionals to indicate shutdown
 * Base server class. All servers will be one of these  
 */
class ServBase : public Thread  {
    public:
        ServBase(Addr & addr, int maxclients = 0);
        virtual ~ServBase(void);
 
    public:
        bool Startup(void);
        bool Shutdown(void);
    public:
        // Class factories for overrides
        virtual SockBase * Create() = 0;
        virtual ServerCon * ServerConFactory(void) = 0;
        
    public:
        // Accept thread
        virtual void Run(void);

    public:
        Addr * GetAddr(void)   { return &m_addr; }
        int  GetNumClients(void) { return m_pool.GetNumThreads(); }

    private:
        // Server thread
        static void * accept_thread(void * me);
        void accept_exit(pthread_t tid);

     protected:
#ifdef SRV_FORCE_HALT
        int         m_dbgconnected;
#endif

        int         m_maxclients;  /**< Maximum allowed connections */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
        Addr        m_addr;        /**< Server address             */
        SockBase *  m_sock;        /**< Server socket              */
#endif
        ServerConPool m_pool;      /**< Connection pool */
};

#endif
