#ifndef _BUFFER_H_INCLUDED
#define _BUFFER_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "logger.h"

typedef unsigned char byte;


/**
    @class Buffer
    Buffer class for serializing and marshalling data
*/
class Buffer : public ClassLogger {
    public:
        Buffer(void);
        Buffer(byte * buff, int size);
        virtual ~Buffer(void);

    public:
        virtual int Size(void);
        byte  * Get(void);
        void    Set(byte * buff, int size);
        void    Reset(void);
    
    public:
        virtual Buffer & operator << (const char *);
        virtual Buffer & operator << (uint64_t);
        virtual Buffer & operator << (int64_t);
        virtual Buffer & operator << (uint32_t);
        virtual Buffer & operator << (int32_t);
        virtual Buffer & operator << (uint16_t);
        virtual Buffer & operator << (int16_t);
        virtual Buffer & operator << (char);
        virtual Buffer & operator << (float);
        virtual Buffer & operator << (double);
        virtual Buffer & operator << (bool);
        virtual Buffer & operator << (struct timeval);
#ifdef NEED_TIME_T_FUNCS
        /* XXX: Letting g++ handle the casting from time_t to an int seems to fail
         * on 32-bit.  Add explicit time_t handlers when this is needed.
         * Bug in glibc?
         */
        virtual Buffer & operator << (time_t t) { return *this << (int64_t) t; }
#endif
        
        Buffer & operator >> (char **);
        Buffer & operator >> (uint64_t &);
        Buffer & operator >> (int64_t &);
        Buffer & operator >> (uint32_t &);
        Buffer & operator >> (int32_t &);
        Buffer & operator >> (uint16_t &);
        Buffer & operator >> (int16_t &);
        Buffer & operator >> (char &);
        Buffer & operator >> (float &);
        Buffer & operator >> (double &);
        Buffer & operator >> (bool &);
        Buffer & operator >> (struct timeval &);
#ifdef NEED_TIME_T_FUNCS
        Buffer & operator >> (time_t &t);
#endif

    protected:
        void    Rewind(void);
        void    Resize(int newsize);
        virtual void    Append(byte * buff, int size);

    protected:
        byte    * m_buff;       /**< Byte buffer */
        byte    * m_cursor;     /**< Cursor */

        int       m_size;       /**< Buffer size */
        int       m_sizemax;    /**< Actual buffer size */
};

#endif
