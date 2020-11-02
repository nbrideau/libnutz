
#include "exception.h"
#include "utils.h"

const char * gs_severity[] = {
    "[NONE]",
    "[ERROR]",
    "[FATAL]"
};

Exception::Exception(void) : ClassLogger("Exception") {
    m_msg  = NULL;
    m_type = GE_ERROR;
    return;
}

Exception::Exception(const char * msg, ExceptionType severity) 
: ClassLogger("Exception") {
    m_msg  = nstrdup(msg);
    m_type = severity;
    //Log(LL_ERROR, "[Exception]%s %s", gs_severity[severity], m_msg);
    return;
}

Exception::~Exception(void) {
    delete [] m_msg;
    return;
}

const char * Exception::Msg(void) { 
    return m_msg; 
}

