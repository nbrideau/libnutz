
#include "list.h"
#include <stdio.h>
#include <strings.h>



template<typename T>
List<T>::List(bool threadsafe, bool memmanaged) 
: m_mtx() {
    m_nodes = 0; 
    m_head = m_tail = m_curr = NULL;
    m_threadsafe = threadsafe;
    m_memmanaged = memmanaged;
    m_isstring   = false;
    return;
}

template<typename T>
List<T>::~List(void) {
    if (m_memmanaged) 
        Purge();
    else 
        Empty();
    return;
}



template<typename T>
void List<T>::Lock(void) {
    m_mtx.Lock(); 
    return;
}

template<typename T>
void List<T>::UnLock(void)       { m_mtx.UnLock(); }

template<typename T>
void List<T>::SetString(void)    { m_isstring = true; } 

template<typename T>
unsigned int List<T>::Size(void) { 
    unsigned int rv;
    if (m_threadsafe) m_mtx.Lock();
    rv = m_nodes;
    if (m_threadsafe) m_mtx.UnLock();
    return rv; 
}

template<typename T>
void List<T>::Empty(void) {
    if (m_threadsafe) m_mtx.Lock();
    llnode<T> * node = m_head;
    while (node) {
        m_curr = node;
        node = node->next;
        delete m_curr;
        m_nodes--;
    }
    m_nodes = 0; 
    m_head = m_tail = m_curr = NULL;
    if (m_threadsafe) m_mtx.UnLock();
    return;
}

template<typename T>
void List<T>::Purge(void) {
    if (m_threadsafe) m_mtx.Lock();
    llnode<T> * node = m_head;
    while (node) {
        m_curr = node;
        node = node->next;
        if (m_memmanaged) {
            if (m_isstring) delete [] m_curr->data;
            else delete m_curr->data;
        }
        delete m_curr;
        m_nodes--;
    }
    m_nodes = 0; 
    m_head = m_tail = m_curr = NULL;
    if (m_threadsafe) m_mtx.UnLock();
    return;
}

template<typename T>
void List<T>::AddHead(T * data) {
    llnode<T> * node = new llnode<T>;
    bzero(node, sizeof(llnode<T>));
    node->data = data;
    if (m_threadsafe) m_mtx.Lock();
    m_nodes++;
    if (!m_head) {
        m_head = node;
        m_tail = node;
    }
    else {
        node->next   = m_head;
        m_head->prev = node;
        m_head = node;
    }
    m_curr = node;
    if (m_threadsafe) m_mtx.UnLock();
    return;
}

template<typename T>
void List<T>::AddTail(T * data) {
    llnode<T> * node = new llnode<T>;
    bzero(node, sizeof(llnode<T>));
    node->data = data;
    if (m_threadsafe) m_mtx.Lock();
    m_nodes++;
    if (!m_tail) {
        m_head = node;
        m_tail = node;
    } 
    else {
        m_tail->next = node;
        node->prev   = m_tail;
        m_tail = node;
    }
    m_curr = node;
    if (m_threadsafe) m_mtx.UnLock();
    return;
}

template<typename T>
void List<T>::AddCurr(T * data) {
    if (!m_head || !m_curr || m_curr == m_head) 
        return AddHead(data);
    if (m_curr == m_tail) 
        return AddTail(data);
    llnode<T> * node = new llnode<T>;
    bzero(node, sizeof(llnode<T>));
    node->data = data;
    if (m_threadsafe) m_mtx.Lock();
    m_nodes++;
    node->next = m_curr;
    if ((node->prev = m_curr->prev)) 
        m_curr->prev->next = node;
    m_curr->prev = node;
    if (m_threadsafe) m_mtx.UnLock();
    return;
}

template<typename T>
bool List<T>::RemoveHead(void) {
    if (m_threadsafe) m_mtx.Lock();
    if (!m_nodes) {
        if (m_threadsafe) m_mtx.UnLock();
        return false;
    }
    m_nodes--;
    if (m_head->next) {
        if (m_curr == m_head) 
            m_curr = m_head->next;
        m_head = m_head->next;
        if (m_memmanaged)
            delete m_head->prev->data;
        delete m_head->prev;
        m_head->prev = NULL;
    }
    else {
        if (m_memmanaged)
            delete m_head->data;
        delete m_head;
        m_head = NULL;
        m_tail = NULL;
        m_curr = NULL;
    }
    if (m_threadsafe) m_mtx.UnLock();
    return true;

}

template<typename T>
bool List<T>::RemoveTail(void) {
    if (m_threadsafe) m_mtx.Lock();
    if (!m_nodes) {
        if (m_threadsafe) m_mtx.UnLock();
        return false;
    }
    m_nodes--;
    if (m_tail->prev) {
        if (m_curr == m_tail) 
            m_curr = m_tail->prev;
        m_tail = m_tail->prev;
        if (m_memmanaged)
            delete m_tail->next->data;
        delete m_tail->next;
        m_tail->next = NULL;
    } 
    else {
        if (m_memmanaged)
            delete m_tail->data;
        delete m_tail;
        m_head = NULL;
        m_tail = NULL;
        m_curr = NULL;
    }
    if (m_threadsafe) m_mtx.UnLock();
    return true;
}

template<typename T>
bool List<T>::RemoveCurr(void) {
    llnode<T> * node;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_nodes) {
        if (m_threadsafe) m_mtx.UnLock();
        return false;
    }
    if (m_curr == m_head) {
        if (m_threadsafe) m_mtx.UnLock();
        return RemoveHead();
    }
    if (m_curr == m_tail) {
        if (m_threadsafe) m_mtx.UnLock();
        return RemoveTail();
    }
    m_nodes--;
    node = m_curr;
    m_curr->next->prev = m_curr->prev;
    m_curr->prev->next = m_curr->next;
    m_curr = m_curr->prev;
    if (m_memmanaged)
        delete node->data;
    delete node;
    if (m_threadsafe) m_mtx.UnLock();
    return true;
}

template<typename T>
T * List<T>::GetHead(void) {
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_head) {
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    m_curr = m_head;
    rv = m_head->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::GetTail(void) {
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_tail) {
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    m_curr = m_tail;
    rv = m_tail->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::GetCurr(void) {
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_curr) {
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    rv = m_curr->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::GetNext(void) {
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_curr || !m_curr->next) { 
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    m_curr = m_curr->next;
    rv = m_curr->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::GetPrev(void) {
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    if (!m_curr || !m_curr->prev) {
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    m_curr = m_curr->prev;
    rv = m_curr->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::GetAt(unsigned int idx) {
    unsigned int i = 0;
    T * rv;
    if (m_threadsafe) m_mtx.Lock();
    m_curr = m_head;
    while (m_curr && i++ < idx) 
        m_curr = m_curr->next;
    if (!m_curr) {
        if (m_threadsafe) m_mtx.UnLock();
        return NULL;
    }
    rv = m_curr->data;
    if (m_threadsafe) m_mtx.UnLock();
    return rv;
}

template<typename T>
T * List<T>::operator[](unsigned int idx) {
    return GetAt(idx);
}

template<typename T>
void List<T>::Swap(unsigned int a, unsigned int b) {
    unsigned int i;
    T         * nodet;
    llnode<T> * nodea;
    llnode<T> * nodeb;
    if (m_threadsafe) m_mtx.Lock();
    if (a >= m_nodes || b >= m_nodes) return;   

    // Find node a and node b
    nodea = m_head;
    nodeb = m_head;
    i = 0;
    while (nodea && i++ < a) 
        nodea = nodea->next;
    i = 0;
    while (nodeb && i++ < b) 
        nodeb = nodeb->next;

    if (!nodea || !nodeb) return;

    // Swap
    nodet       = nodea->data;
    nodea->data = nodeb->data;
    nodeb->data = nodet;

    if (m_threadsafe) m_mtx.UnLock();
    return;
}

