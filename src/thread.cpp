
#include "thread.h"
#include "libnutz.h"
#include <unistd.h>
#include <signal.h>
#include <strings.h>

// Note: When going through the errors some will be impossible due to approach.


/*****************************************************************************
 *
 *****************************************************************************/
Condition::Condition(bool set) 
: ClassLogger("Condition"), m_mtx(false) {
    m_set = true;
    if (pthread_cond_init (&m_cnd, NULL)) {
        /*
            EAGAIN The system lacked the necessary resources (other than memory) to
              initialize another condition variable.
           ENOMEM Insufficient memory exists to initialize the condition variable.
        */
        LogSysError("::Condition pthread_cond_init");
    }
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Condition::~Condition(void) {
    if (pthread_cond_destroy(&m_cnd)) {
        /* EBUSY  The implementation has detected an attempt to destroy the object
              referenced  by  cond  while it is referenced (for example, while
              being used in a pthread_cond_wait() or pthread_cond_timedwait())
              by another thread.
            EINVAL The value specified by cond is invalid.
        */
        LogSysError("::~Condition pthread_cond_destroy");
    }
    return;
}


/*****************************************************************************
 *
 *****************************************************************************/
void Condition::Set(void) {
    m_mtx.Lock();
    m_set = true;
    if (pthread_cond_broadcast(&m_cnd)) {
        /* Log
          EINVAL The value cond does not refer to an initialized condition  vari-
              able.
        */
        LogSysError("::Set pthread_cond_broadcast");
    }
    m_mtx.UnLock();

    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Condition::UnSet(void) {
    m_mtx.Lock();
    m_set = false;
    m_mtx.UnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Condition::IsSet(void) {
    bool rv;
    m_mtx.Lock();
    rv = m_set;
    m_mtx.UnLock();
    return rv;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Condition::Wait(unsigned int nsec) {
    m_mtx.Lock();
    if (m_set) {
        m_mtx.UnLock();
        return true;
    }
    // Just a wait
    if (!nsec) {
        if (pthread_cond_wait(&m_cnd, m_mtx.m_mtx)) {
            /* 
                EINVAL The value specified by cond, mutex, or abstime is invalid.
                EINVAL Different     mutexes     were     supplied    for    concurrent
                    pthread_cond_timedwait() or  pthread_cond_wait()  operations  on
                    the same condition variable.
                EPERM  The mutex was not owned by the current thread at the time of the call.
            */
            LogSysError("::Wait pthread_cond_wait");
        }
    }
    // Timed Wait
    else {
        int rv;
        struct timespec ts;
        bzero(&ts, sizeof(struct timespec));
        ts.tv_nsec = nsec;
        if ((rv = pthread_cond_timedwait(&m_cnd, m_mtx.m_mtx, &ts))) {
            /* 
                ETIMEDOUT The  time  specified  by abstime to pthread_cond_timedwait() has
                    passed.
                EINVAL The value specified by cond, mutex, or abstime is invalid.
                EINVAL Different     mutexes     were     supplied    for    concurrent
                    pthread_cond_timedwait() or  pthread_cond_wait()  operations  on
                    the same condition variable.
                EPERM  The mutex was not owned by the current thread at the time of the call.
            */
            if (rv == ETIMEDOUT) {
                return false;
            }
            LogSysError("::Wait pthread_cond_timedwait");
        }
    }
    m_mtx.UnLock();
    return true;
}

/*****************************************************************************
 * THREAD
 *****************************************************************************/
Thread::Thread(bool detached) 
: ClassLogger("Condition") {
#ifndef THREAD_NO_DETACH
    m_detached = detached;
#endif
    m_thread  = 0;
    m_running = false;
    Resume();
    return;
}

Thread::~Thread(void) {
    Stop();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::Start(void) {
    int rv;
    pthread_attr_t attr;
    int detachstate = PTHREAD_CREATE_JOINABLE;
    
    if (IsRunning()) {
        return false;
    }

    m_lock.Lock();
    m_running = false;
    Resume();

    if ((rv = pthread_attr_init(&attr))) {
        /*  
            ENOMEM Insufficient memory exists to initialize the  thread  attributes
              object.
        */
        LogSysError("::Start pthread_attr_init");
        m_lock.UnLock();
        return false;
    }


    /* XXX
      8M Stacks OMG!!!! 8388608 Room for 256 threads
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
        LogSysError("::Start pthread_attr_setstacksize");
        pthread_attr_destroy(&attr);
        m_lock.UnLock();
        return false;
    }


#ifndef THREAD_NO_DETACH
    if (m_detached) detachstate = PTHREAD_CREATE_DETACHED;
#endif
    if ((rv = pthread_attr_setdetachstate(&attr, detachstate))) {
        /*  
            EINVAL The value of detachstate was not valid
        */
        LogSysError("::Start pthread_attr_setdetachstate");
        pthread_attr_destroy(&attr);
        m_lock.UnLock();
        return false;
    }
    
    // Hand in a pointer to "this"
    m_running = true;
    if ((rv = pthread_create(&m_thread, &attr, thread_run, this))) {
        pthread_attr_destroy(&attr);
        /*
            EAGAIN The  system  lacked  the  necessary  resources to create another
              thread, or the system-imposed  limit  on  the  total  number  of
              threads in a process {PTHREAD_THREADS_MAX} would be exceeded.
            EINVAL The value specified by attr is invalid.
            EPERM  The  caller  does  not  have  appropriate  permission to set the
              required scheduling parameters or scheduling policy.
        */
        LogSysError("::Start pthread_create");
        m_thread  = 0;
        m_running = false;
        m_lock.UnLock();
        return false;
    }
    pthread_attr_destroy(&attr);
    m_lock.UnLock();
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::Stop(void) {
    void ** rc;
    if (!m_thread)  return true;
    m_lock.Lock();
    m_running = false;
    Resume();
#ifndef THREAD_NO_DETACH
    if (!m_detached) {
#endif
        if (pthread_join(m_thread, rc)) {
            m_lock.UnLock();
            return false;
        }
#ifndef THREAD_NO_DETACH
    }
#endif
    m_thread    = 0;
    m_lock.UnLock();
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::Kill(void) {
    if (!m_thread) return true;
    m_lock.Lock();
    m_running = false;
    Resume();
    if (pthread_kill(m_thread, SIGKILL)) {
        m_lock.UnLock();
        return false;
    }
    
    
    m_thread    = 0;
    m_lock.UnLock();
    return true;
} 


/*****************************************************************************
 * Pause for X seconds
 *****************************************************************************/
void Thread::Pause(unsigned int secs) {
    m_lock.Lock();
    m_untill = time(NULL) + secs;
    m_lock.UnLock();
    return;
}



/*****************************************************************************
 *
 *****************************************************************************/
void Thread::Resume(void) {
    m_lock.Lock();
    m_untill = 0;
    m_lock.UnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::IsPaused(void) {
    bool rv = false;
    m_lock.Lock();
    if (m_untill) rv = true;
    m_lock.UnLock();
    return rv;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::IsRunning(void) {
    m_lock.Lock();
    if (!m_thread || !m_running) {
        m_lock.UnLock();
        return false;
    }
    m_lock.UnLock();
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool Thread::Loop(void) {
    if (!IsRunning()) return false;
    
    // This probably is not accurate at all
    while (IsPaused()) {
        usleep(1000000);
        
        // If seconds we only pause X secs
        m_lock.Lock();
        if (time(NULL) > (int)m_untill) {
            m_untill = 0;
            Resume();
        }
        m_lock.UnLock();

        // Make sure we were not stopped
        if (!IsRunning()) return false;
    }
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
void * Thread::thread_run(void * me) {
    Thread * self = (Thread *)me;
    // Make sure we are still running
    while (self->Loop()) self->Run();
    return NULL;
}


#ifndef THREAD_NO_DETACH
void Thread::Detach(void) {
    if (!m_thread)                return;
    if (pthread_detach(m_thread)) return;
    m_detached = true;
    return;
}
#endif
