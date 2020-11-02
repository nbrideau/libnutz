
#ifndef _EXCEPTION_H_INCLUDED
#define _EXCEPTION_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "logger.h"

typedef enum {
    GE_NONE,        // 0
    GE_ERROR,       // 1 close connection
    GE_FATAL,       // 2 shutdown server
} ExceptionType;

/**
 * @class Exception
 * Base exception handling routine
 * @todo Something
 */
class Exception :  public ClassLogger {
    public:
        Exception(void);
        Exception(const char * msg, ExceptionType severity = GE_ERROR);
        ~Exception(void);

    public:
        const char * Msg(void);
        ExceptionType Severity(void) { return m_type; }

    private:
        ExceptionType m_type;
        char          * m_msg;

};

#endif
