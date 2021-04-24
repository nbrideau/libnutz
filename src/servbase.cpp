
#include "servbase.h"
#include "exception.h"
#include "libnutz.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


/*****************************************************************************
 *
 *****************************************************************************/
ServerConPool::ServerConPool(void) 
: Thread(true), m_threads() {
    SetClassName("ServerConPool");
    //Thread::Start();
    return;
}

ServerConPool::~ServerConPool(void) {
    // Stop cleaning thread
    Thread::Stop();
    // Shutdown all other threads
    Shutdown();
    while (m_threads.Size()) usleep(10000); // FIXME
    return;
}


// Note: This is too much like gsthread
bool ServerConPool::Start(void *(*pthread_func)(void *), ServerCon * arg) {
    int rv;
    pthread_t tid;
    pthread_attr_t attr;
    
    // Start housecleaning thread
    if (!Thread::IsRunning()) Thread::Start();
    
    if ((rv = pthread_attr_init(&attr))) {
        /* ENOMEM Insufficient memory exists to initialize the  thread  attributes
              object.  */
        LogSysError("::ServerConPool pthread_attr_init");
        return false;
    }

    /* 8M Stacks OMG!!!! 8388608 Room for 256 threads
    size_t stacksize; 2G / 8M = 256
    pthread_attr_getstacksize(&attr, &stacksize);
    Log(LL_INFO, "Stacksize: %d", stacksize);
    */
    // Well lets try shrinking this to 256K
    // Should give us room for 8192 threads
    if ((rv = pthread_attr_setstacksize(&attr, 262144))) {
        /* 
            EINVAL The value of detachstate was not valid
        */
        LogSysError("::ServerConPool pthread_attr_setstacksize");
        pthread_attr_destroy(&attr);
        return false;

    }

    int dstate = PTHREAD_CREATE_JOINABLE;
#ifndef THREAD_NO_DETACH
    dstate = PTHREAD_CREATE_DETACHED;
#endif
    if ((rv = pthread_attr_setdetachstate(&attr, dstate))) {
        /* 
            EINVAL The value of detachstate was not valid
        */
        LogSysError("::ServerConPool pthread_attr_setdetachstate");
        pthread_attr_destroy(&attr);
        return false;
    }


    // Ensure thread does not exit before added to list
    m_threads.Lock();
    arg->m_lock.Lock();
    arg->m_running = true;
    // Hand in a pointer to "this"
    if ((rv = pthread_create(&tid, &attr, pthread_func, (void *)arg))) {
        m_threads.RemoveCurr();
        m_threads.UnLock();
        /**
            EAGAIN The  system  lacked  the  necessary  resources to create another
              thread, or the system-imposed  limit  on  the  total  number  of
              threads in a process {PTHREAD_THREADS_MAX} would be exceeded.

            EINVAL The value specified by attr is invalid.

            EPERM  The  caller  does  not  have  appropriate  permission to set the
              required scheduling parameters or scheduling policy.
        */
        //Log(LL_ERROR, "PTHREAD_THREAD_MAX:%d", PTHREAD_THREAD_MAX);
        LogSysError("::ServerConPool pthread_create");
        pthread_attr_destroy(&attr);
        arg->m_running = false;
        arg->m_lock.UnLock();
        return false;
    }
    pthread_attr_destroy(&attr);
    arg->SetThreadId(tid);
    m_threads.AddTail(arg);
    m_threads.UnLock();
    arg->m_lock.UnLock();

    return true;
}

// Threadpool houskeeping routine
// NB No longer Used XXX?
void ServerConPool::Run(void) {

    // Hack to make sure the threads all get cleaned up on shutdown.
    m_threads.Lock();
    ServerCon * node = m_threads.GetHead();
    while (node) {
        // Shutdown threads cleanup make sure socket not still open
        if (!node->IsRunning() && !node->IsConnected()) {
            Log(LL_DEBUG, "Removing connection. %u left.", m_threads.Size());
            m_threads.RemoveCurr();
        }
        // XXX Stopping thread causes crash???
        // Stop stale connections 60 seconds
        else if ((time(NULL) - node->LastUpdate()) > 60) {
            //Log(LL_ERROR, "Stopping connection.");
            //node->Stop();
        }
        node = m_threads.GetNext();
    }
    m_threads.UnLock();
    Thread::Pause(1);
    return;
}

void ServerConPool::Shutdown(void) {
    m_threads.Lock();
    ServerCon * node = m_threads.GetHead();
    while (node) {
        node->Stop();
        node = m_threads.GetNext();
    }
    m_threads.UnLock();
    return;
}

int ServerConPool::GetNumThreads(void) {
    return m_threads.Size();
}

ServerCon * ServerConPool::GetThread(pthread_t tid) {
    m_threads.Lock();
    ServerCon * node = m_threads.GetHead();
    while (node) {
        if (node->ThreadId() == tid) {
            m_threads.UnLock();
            return node;
        }
        node = m_threads.GetNext();
    }
    m_threads.UnLock();
    return NULL;
}

bool ServerConPool::Stop(pthread_t tid) {
    bool rv;
    ServerCon * thread;
    m_threads.Lock();
    if (!(thread = GetThread(tid))) {
        m_threads.UnLock();
        return false;
    }
    rv = thread->Stop();
    m_threads.UnLock();
    return rv;
}

#ifndef THREAD_NO_DETACH
bool ServerConPool::RemoveThread(pthread_t tid) {
    bool rv;
    ServerCon * thread;
    m_threads.Lock();
    if (!(thread = GetThread(tid))) {
        Log(LL_ERROR, "Attempting to remove thread which does not exist");
        m_threads.UnLock();
        return false;
    }
    rv = thread->Stop();
    m_threads.RemoveCurr();
    m_threads.UnLock();
    return rv;
}
#endif

bool ServerConPool::Kill(pthread_t tid) {
    bool rv;
    ServerCon * thread;
    m_threads.Lock();
    if (!(thread = GetThread(tid))) {
        m_threads.UnLock();
        return false;
    }
    rv = thread->Kill();
    m_threads.RemoveCurr();
    m_threads.UnLock();
    return rv;
}

void ServerConPool::Pause(pthread_t tid) {
    ServerCon * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Pause();
}

void ServerConPool::Pause(pthread_t tid, unsigned int secs) {
    ServerCon * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Pause(secs);
}

void ServerConPool::Resume(pthread_t tid) {
    ServerCon * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Resume();
}

bool ServerConPool::IsPaused(pthread_t tid) {
    ServerCon * thread;
    if (!(thread = GetThread(tid))) return false;
    return thread->IsPaused();
}

bool ServerConPool::IsStarted(pthread_t tid) {
    ServerCon * thread;
    if (!(thread = GetThread(tid))) return false;
    return thread->IsRunning();
}



/*****************************************************************************
 *
 *****************************************************************************/
ServerCon::ServerCon(ServBase * server) {
    SetClassName("ServerCon");
    m_serv = server;
    m_sock = m_serv->Create();
    return;
}

ServerCon::~ServerCon(void) {
    delete m_sock;
    return;
}

ServBase * ServerCon::Server(void) {
    return m_serv;
}

SockBase * ServerCon::Socket(void) {
    return m_sock;
}

Addr * ServerCon::GetAddr(void) {
    return m_sock->GetAddr();
}

pthread_t ServerCon::ThreadId(void) {
    return m_thread;
}

void ServerCon::SetThreadId(pthread_t tid) {
#ifndef THREAD_NO_DETACH
    m_detached  = true;
#endif
    m_thread    = tid;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
ServBase::ServBase(Addr & addr, int maxclients) 
: Thread(true), m_addr(addr), m_pool() {
    struct rlimit rlim;
    SetClassName("ServBase");
    m_sock = NULL;

    // Dump core
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);
    
    // Try upping the max open files 
    rlim.rlim_cur = 8192;
    rlim.rlim_max = 8192;
    setrlimit(RLIMIT_NOFILE, &rlim);    // -1 fail
    getrlimit(RLIMIT_NOFILE, &rlim);    // -1 fail

    if (!maxclients) 
        maxclients = (rlim.rlim_cur - 40);
    if (maxclients > (int)(rlim.rlim_cur - 40)) 
        maxclients = (rlim.rlim_cur - 40);

    m_maxclients = maxclients;
    Log(LL_INFO, "Server started %u clients max.", m_maxclients);
#ifdef SRV_FORCE_HALT
    m_dbgconnected = 0;
#endif
    return;
}

ServBase::~ServBase(void) {
    Shutdown();
    delete m_sock;
    return;
}


bool ServBase::Shutdown(void) {
    m_running = false;
    // Wait for threads to finish
    m_pool.Shutdown();
    unsigned int i = m_pool.GetNumThreads();
    while (i) {
        Log(LL_INFO, "Shutting down %d clients...", i);
        usleep(1000000);
        i = m_pool.GetNumThreads();
    }   
    m_sock->Close();
    return true;
}

bool ServBase::Startup(void) {
    if (!m_sock->Setup()) return false;
    Start();
    return true;;
}

// Server accept loop
void ServBase::Run(void) {
    ServerCon * con = NULL; // Data to pass to new thread
    
    // Build server data
    con = ServerConFactory();

    // Accept new connection
    if (m_sock->Accept(con->Socket())) {
        Log(LL_DEBUG, "New connection from %s:%hu . Total: %d", 
                con->GetAddr()->Ip(), con->GetAddr()->Port()
                , m_pool.GetNumThreads());

        // Check # connected clients
        if (m_pool.GetNumThreads() < m_maxclients) {
            // Spawn thread and start reading
            // Add to thread pool
            if (!m_pool.Start(accept_thread, con)) {
                Log(LL_ERROR, "[::Start] Error spawning thread");
                delete con;
            }
            else {
#ifdef SRV_FORCE_HALT
                m_dbgconnected++;
                Log(LL_ERROR, "Connected %d", m_dbgconnected);
                if (m_dbgconnected >= SRV_FORCE_HALT) {
                    m_running = false;
                    Log(LL_ERROR, "Stopping Connected %d", m_dbgconnected);
                }
#endif        
            }
        }
        else {
            Log(LL_ERROR, "Max clients reached %u", m_maxclients);
            con->OnMaxClients();
            delete con;
        }
    }
    else {
        delete con;
    }       

    return; 
}

// Server running loop
// Called after accept
void * ServBase::accept_thread(void * arg) {
    void * msg = NULL;
    // Setup servercon thread con
    ServerCon * con = (ServerCon *)arg;
    if (!con)  return NULL;

    // Set timeout
    con->SetTimeout(60);

    // Wait to finish startup
    con->OnConnect();
    
    while (con->Loop() && con->Socket()->IsConnected()) {           
        try {
            // timeout
            if (!con->RecvMsg(&msg)) {
                // 10 minute disconnect
                if ((time(NULL) - con->LastUpdate()) > 900) {
                    con->Server()->Log(LL_DEBUG, "15 minute timeout disconnecting.");
                    break;
                }
                continue;
            }
            if (msg == NULL) 
                con->Server()->Log(LL_DEBUG, "No packet!");
            else {
                con->ProcessMsg(msg);
                con->DeleteMsg(msg);
            }
        } catch (Exception & e) {
            if (e.Severity() != GE_NONE) 
                con->Server()->Log(LL_ERROR, "Shutting down socket: %s", e.Msg());
            break;
        } catch (...) {
            con->Server()->Log(LL_ERROR, "Unknown exception caught");
            break;
        }
    } 
    con->OnDisconnect();
    con->Close();
    con->Log(LL_DEBUG, "Closing connection from %s:%hu", con->GetAddr()->Ip(), con->GetAddr()->Port());

    // Force removal from thread pool
#ifndef THREAD_NO_DETACH
    con->Server()->m_pool.RemoveThread(con->m_thread);
#endif
    return NULL;
}

