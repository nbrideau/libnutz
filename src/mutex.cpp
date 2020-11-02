
#include "mutex.h"
#include <cerrno>
#include <cstdio>
#include <string.h>
/*****************************************************************************
 *
 *****************************************************************************/
Mutex::Mutex(bool reentrant) {
    pthread_mutexattr_t attr;
    m_auto  = false;
    m_mtx   = new pthread_mutex_t;
    if (pthread_mutexattr_init(&attr)) {
        /* Log
            EINVAL The value specified by attr is invalid.
            ENOMEM Insufficient  memory  exists  to initialize the mutex attributes object.
        */
        printf("[Mutex::Mutex] MUTEX ATTR INIT FAILURE! %s\n", strerror(errno));
        return;
    }
    if (reentrant) {
        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) {
            /* Log Cant include logger from here
                EINVAL The value type is invalid.
            */
            printf("[Mutex::Mutex] MUTEX ATTR SETTYPE FAILURE! %s\n", strerror(errno));
            return;
        }
    }
    if (pthread_mutex_init(m_mtx, &attr)) {
        /*  Log Cant include logger from here
            EAGAIN The system lacked the necessary resources (other than memory) to
              initialize another mutex.

            ENOMEM Insufficient memory exists to initialize the mutex.

            EPERM  The caller does not have the privilege to perform the operation.

            EBUSY  The implementation has detected an attempt to  reinitialize  the
              object  referenced  by  mutex, a previously initialized, but not
              yet destroyed, mutex.

            EINVAL The value specified by attr is invalid.
        */
        printf("[Mutex::Mutex] MUTEX INIT FAILURE! %s\n", strerror(errno));
        return;
    }
    pthread_mutexattr_destroy(&attr);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Mutex::Mutex(Mutex & mtx) {
    m_auto  = true;
    m_mtx   = mtx.m_mtx;
    Lock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Mutex::~Mutex(void) {
    if (m_auto) {
        UnLock();
        return;
    }
    if (pthread_mutex_destroy(m_mtx)) {
        /*  Log Cant include logger from here
            EBUSY  The implementation has detected an attempt to destroy the object
              referenced by mutex while it is locked or referenced (for  exam-
              ple,   while   being   used  in  a  pthread_cond_timedwait()  or
              pthread_cond_wait()) by another thread.

            EINVAL The value specified by mutex is invalid.
        */
        printf("[Mutex::~Mutex] MUTEX DESTROY FAILURE! %s\n", strerror(errno));
    }
    delete m_mtx;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Mutex::Lock(void) {
    if (pthread_mutex_lock(m_mtx)) {
        /*  Log Cant include logger from here
            EINVAL The  mutex  was  created  with the protocol attribute having the
              value PTHREAD_PRIO_PROTECT and the calling thread's priority  is
              higher than the mutex's current priority ceiling.

            EINVAL The value specified by mutex does not refer  to  an  initialized
              mutex object.

            EAGAIN The  mutex  could  not be acquired because the maximum number of
              recursive locks for mutex has been exceeded.
            
            EDEADLK (wont happen for reentrant mutexes)
              The current thread already owns the mutex.
        */
        printf("[Mutex::Lock] LOCK FAILURE! %s\n", strerror(errno));
        return;
    }
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Mutex::UnLock(void) {
    int rv;
    if ((rv = pthread_mutex_unlock(m_mtx))) {
        /*  Log Cant include logger from here
            EBUSY  The mutex could not be acquired because it was already locked.

            EINVAL The value specified by mutex does not refer  to  an  initialized
              mutex object.

            EAGAIN The  mutex  could  not be acquired because the maximum number of
              recursive locks for mutex has been exceeded.
            
            EPERM  The current thread does not own the mutex.
        */
        switch (rv) {
            case EBUSY:  
                printf("[Mutex::UnLock] The mutex could not be acquired because it was already locked.\n");
                break;
            case EINVAL:
                printf("[Mutex::UnLock] The value specified by mutex does not refer to an initialized mutex object.\n");
                break;
            case EAGAIN:
                printf("[Mutex::UnLock] The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.\n");
                break;
            case EPERM:
                printf("[Mutex::UnLock] The current thread does not own the mutex.\n");
                break;
        }
    }
    return;
}
