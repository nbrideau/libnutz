
#include "servbase.h"
#include "exception.h"
#include "libnutz.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

/*****************************************************************************
 *
 *****************************************************************************/
ServerData::ServerData(ServBase * server) {
    SetClassName("ServerData");
    m_serv = server;
    m_sock = m_serv->Create();
    return;
}

ServerData::~ServerData(void) {
    delete m_sock;
    return;
}

ServBase * ServerData::Server(void) {
    return m_serv;
}

SockBase * ServerData::Socket(void) {
    return m_sock;
}

Addr * ServerData::GetAddr(void) {
    return m_sock->GetAddr();
}

pthread_t ServerData::ThreadId(void) {
    return m_thread;
}

void ServerData::SetThreadId(pthread_t tid) {
#ifndef THREAD_NO_DETACH
    m_detached  = true;
#endif
    m_thread    = tid;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
ThreadPool::ThreadPool(void) 
: Thread(true), m_threads() {
    SetClassName("ThreadPool");
    //Thread::Start();
    return;
}

ThreadPool::~ThreadPool(void) {
    // Stop cleaning thread
    Thread::Stop();
    // Shutdown all other threads
    Shutdown();
    while (m_threads.Size()) usleep(1000000);
    return;
}


// Note: This is too much like gsthread
bool ThreadPool::Start(void *(*pthread_func)(void *), ServerData * arg) {
    int rv;
    pthread_t tid;
    pthread_attr_t attr;
    
    // Start housecleaning thread
    if (!Thread::IsRunning()) Thread::Start();
    
    if ((rv = pthread_attr_init(&attr))) {
        /* ENOMEM Insufficient memory exists to initialize the  thread  attributes
              object.  */
        LogSysError("::ThreadPool pthread_attr_init");
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
        LogSysError("::ThreadPool pthread_attr_setstacksize");
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
        LogSysError("::ThreadPool pthread_attr_setdetachstate");
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
        LogSysError("::ThreadPool pthread_create");
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

// NB No longer Used
// Threadpool houskeeping routine
void ThreadPool::Run(void) {

    // Hack to make sure the threads all get cleaned up on shutdown.
    m_threads.Lock();
    ServerData * node = m_threads.GetHead();
    while (node) {
        // Shutdown threads cleanup make sure socket not still open
        if (!node->IsRunning() && !node->IsConnected()) {
            Log(LL_DEBUG, "Removing connection. %u left.", m_threads.Size());
            m_threads.RemoveCurr();
        }
        // Stopping thread causes crash???
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

void ThreadPool::Shutdown(void) {
    m_threads.Lock();
    ServerData * node = m_threads.GetHead();
    while (node) {
        node->Stop();
        node = m_threads.GetNext();
    }
    m_threads.UnLock();
    return;
}

int ThreadPool::GetNumThreads(void) {
    return m_threads.Size();
}

ServerData * ThreadPool::GetThread(pthread_t tid) {
    m_threads.Lock();
    ServerData * node = m_threads.GetHead();
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

bool ThreadPool::Stop(pthread_t tid) {
    bool rv;
    ServerData * thread;
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
bool ThreadPool::RemoveThread(pthread_t tid) {
    bool rv;
    ServerData * thread;
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

bool ThreadPool::Kill(pthread_t tid) {
    bool rv;
    ServerData * thread;
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

void ThreadPool::Pause(pthread_t tid) {
    ServerData * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Pause();
}

void ThreadPool::Pause(pthread_t tid, unsigned int secs) {
    ServerData * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Pause(secs);
}

void ThreadPool::Resume(pthread_t tid) {
    ServerData * thread;
    if (!(thread = GetThread(tid))) return;
    return thread->Resume();
}

bool ThreadPool::IsPaused(pthread_t tid) {
    ServerData * thread;
    if (!(thread = GetThread(tid))) return false;
    return thread->IsPaused();
}

bool ThreadPool::IsStarted(pthread_t tid) {
    ServerData * thread;
    if (!(thread = GetThread(tid))) return false;
    return thread->IsRunning();
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
    ServerData * data = NULL; // Data to pass to new thread
    
    // Build server data
    data = ServerDataFactory();

    // Accept new connection
    if (m_sock->Accept(data->Socket())) {
        Log(LL_DEBUG, "New connection from %s:%hu . Total: %d", 
                data->GetAddr()->Ip(), data->GetAddr()->Port()
                , m_pool.GetNumThreads());

        // Check # connected clients
        if (m_pool.GetNumThreads() < m_maxclients) {
            // Spawn thread and start reading
            // Add to thread pool
            if (!m_pool.Start(accept_thread, data)) {
                Log(LL_ERROR, "[::Start] Error spawning thread");
                delete data;
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
            data->OnMaxClients();
            delete data;
        }
    }
    else {
        delete data;
    }       

    return; 
}

// Server running loop
// Called after accept
void * ServBase::accept_thread(void * arg) {
    void * msg = NULL;
    // Setup gserverdata thread data
    ServerData * data = (ServerData *)arg;
    if (!data)  return NULL;

    // Set timeout
    data->SetTimeout(60);

    // Wait to finish startup
    data->OnConnect();
    
    while (data->Loop() && data->Socket()->IsConnected()) {           
        try {
            // timeout
            if (!data->RecvPacket(&msg)) {
                // 10 minute disconnect
                if ((time(NULL) - data->LastUpdate()) > 900) {
                    data->Server()->Log(LL_DEBUG, "15 minute timeout disconnecting.");
                    break;
                }
                continue;
            }
            if (msg == NULL) 
                data->Server()->Log(LL_DEBUG, "No packet!");
            else {
                data->ProcessMsg(msg);
                data->DeletePacket(msg);
            }
        } catch (Exception & e) {
            if (e.Severity() != GE_NONE) 
                data->Server()->Log(LL_ERROR, "Shutting down socket: %s", e.Msg());
            break;
        } catch (...) {
            data->Server()->Log(LL_ERROR, "Unknown exception caught");
            break;
        }
    } 
    data->OnDisconnect();
    data->Close();
    data->Log(LL_DEBUG, "Closing connection from %s:%hu", data->GetAddr()->Ip(), data->GetAddr()->Port());

    // Force removal from thread pool
#ifndef THREAD_NO_DETACH
    data->Server()->m_pool.RemoveThread(data->m_thread);
#endif
    return NULL;
}

