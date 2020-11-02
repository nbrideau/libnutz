#ifndef _LLIST_H_INCLUDED
#define _LLIST_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mutex.h"

/** Linked list node struct */
template <class T>
struct llnode {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    llnode<T> * next;         /**< Pointer to next node */
    llnode<T> * prev;         /**< Pointer to previous node */
#endif
    T         * data;         /**< Pointer to nodes data */
};


/**
    @class GSList
    
    Simple doubly linked list class.
 */
template <class T>
class List { 
    public:
        List(bool threadsafe = true, bool memmanaged = true);
        virtual ~List(void);
    
    public:
        // UTILITY METHODS
        void Lock(void);
        void UnLock(void);
        /** Use delete [] */
        void SetString(void);

        unsigned int Size(void);
        void Empty(void);
        void Purge(void);

    public:
        // ADD METHODS
        void AddHead(T * data);
        void AddTail(T * data);
        void AddCurr(T * data);
        

    public:
        // REMOVE METHODS
        bool RemoveHead(void);
        bool RemoveTail(void);
        bool RemoveCurr(void);
 
    public:
        // GET METHODS
        T * GetHead(void);
        T * GetTail(void);
        T * GetCurr(void);
        T * GetNext(void);
        T * GetPrev(void);
        T * GetAt(unsigned int idx);
        T * operator[](unsigned int idx);
        void Swap(unsigned int a, unsigned int b);

    private:
        bool            m_isstring;     /*< use delete [] */
        bool            m_memmanaged; /**< Manage memory or not */
        bool            m_threadsafe; /**< Threadsafe or not */

        unsigned int    m_nodes;    /**< Number of nodes */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
        llnode<T>     * m_head;     /**< Pointer to first node */
        llnode<T>     * m_tail;     /**< Pointer to last node */
        llnode<T>     * m_curr;     /**< Last accessed node */
        Mutex           m_mtx;      /**< Mutex object */
#else 
        T             * m_head;
#endif        
        
};

#endif
