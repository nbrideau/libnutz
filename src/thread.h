
#ifndef _THREAD_H_INCLUDED
#define _THREAD_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>
#include "logger.h"
#include "mutex.h"
#include "libnutz.h"

// Signal events
class Condition : public ClassLogger {
    public:
        Condition(bool set = false);
        ~Condition(void);
    
    public:
        void Set(void);
        void UnSet(void);
        bool IsSet(void);

        // 100000's of a second
        // 0 = forever which is a really long time to wait
        bool Wait(unsigned int nsec = 0);
    
    private:
        pthread_cond_t  m_cnd;
        bool            m_set;
#ifndef DOXYGEN_SHOULD_SKIP_THIS // FIXME 
        Mutex           m_mtx;
#endif
};


// Only supports one thread/
// Possibly elaborate by incorportaing erverData extensions?

// Spawns threads to do shit
class Thread : public ClassLogger {
    public:
        Thread(bool detached = false);
        virtual ~Thread(void);

    public:
        bool Start(void);
        bool Stop(void);
        bool Kill(void);

        void Pause(unsigned int secs = 0);
        void Resume(void);

        bool IsRunning(void);
        bool IsPaused(void);

        virtual void Run(void) = 0;

#ifndef THREAD_NO_DETACH    // FIXME
        void Detach(void);
#endif

    protected:
        // Pause routine
        bool Loop(void);

    private:
        static void * thread_run(void * me);

    protected:
#ifndef THREAD_NO_DETACH
        bool            m_detached;
#endif
        Mutex           m_lock;
        unsigned int    m_untill;
        bool            m_running;

        pthread_t       m_thread;
};

#endif
