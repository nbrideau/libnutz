
#ifndef _MUTEX_H_INCLUDED
#define _MUTEX_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>

#define AUTOLOCK(a) Mutex alock(a);

// Mutual Exclusion
/** 
 * @brief 
 */
class Mutex {
    public:
        Mutex(bool reentrant = true);
        // Auto lock
        Mutex(Mutex & mtx);
        ~Mutex(void);

    public:
        void Lock(void);
        void UnLock(void);
        
        //int  GetStatus(void) { return rv; }

    private:    
        //int                 rv;       /**< Result for last system call */
        bool                m_auto;     /**< If we are an "auto lock" or not */

    // Condition needs me
    friend class GSCondition;
    protected:
        pthread_mutex_t *   m_mtx;      /**< Mutex pointer */
};

#endif
