#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <string>
#include <stdarg.h>

#include "mutex.h"

#define MAX_LOG_SIZE 512

typedef enum {
    LL_NONE,
    LL_FATAL,
    LL_CRITICAL,
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_DEBUG,
    LL_ALL
} LogLevel;

typedef enum  {
    LC_DATE         = 1 << 0,
    LC_THREADID     = 1 << 1,
    LC_LEVEL        = 1 << 2,
    LC_COMPONENT    = 1 << 3,
    LC_THREADSAFE   = 1 << 4,
    LC_STDOUT       = 1 << 5,
    LC_FLUSH        = 1 << 6,
    LC_NONE         = 1 << 31
} LogConfig;

/**
 Thinking I could make a singlton logger that does the work.
 Then we could have a logger class with can be inherited to
 allow one to set the component name and just call the log 
 methods.

 Should log
[DATE][PTHREADID][LEVEL][COMPONENT] MESSAGE

 */
class Logger {
    public:
        Logger(void);
        virtual ~Logger(void);

    public:
        void Log(LogLevel ll, const char * msg, ...);
        void VLog(LogLevel ll, const char * msg, va_list * marker);
        
        void LogLock(void);
        void LogUnLock(void);

        static const char * LogLevelString(LogLevel ll);
        static LogLevel StringLogLevel(const char * ll);

    public:
        void SetMaxLogLevel(const char * ll);
        LogLevel GetMaxLogLevel(void)           { return m_ll;      }
        void SetMaxLogLevel(LogLevel ll)        { m_ll = ll;        }
        void SetOptions(unsigned int config)    { m_config = config;}
        void SetPrefix(const char * filename);
    
        void SetHostname(const char * ip);

    private:
        void Open(void);
        void Close(void);
        void Rotate(void);
        void SetFilename(void);

    private:
        FILE *          m_fp;
        char *          m_prefix;
        char *          m_hostname;
        char            m_filename[56];
        char            m_buff[MAX_LOG_SIZE];
        std::string     m_line;
        Mutex           m_lock;
        LogLevel        m_ll;
        unsigned int    m_config;
};

// Singleton global logger class
// Must initialize with a built logger
// Must be threadsafe
class GlobalLogger {
    public:
        static void Initialize(Logger * logger);
        static Logger * Get(void);

    public:
        static void Log(LogLevel ll, const char * msg, ...);
    
    protected:
        GlobalLogger(void);
        GlobalLogger(const GlobalLogger &);
        GlobalLogger & operator = (const GlobalLogger &);
        ~GlobalLogger(void);

    private:
        static Logger * m_logger;
};
 
// Log classname then add log methods to inheriting class
// We may use this for global object counts....
class ClassLogger {
    public:
        ClassLogger(const char * clsname);
        ~ClassLogger(void);

    public:
        LogLevel GetMaxLogLevel(void);
        void Log(LogLevel ll, const char * msg, ...);
        void LogSysError(const char * msg);
        void SetClassName(const char * clasname);

    private:
        const char * m_clsname;
        char         m_buff[MAX_LOG_SIZE];
};

#endif

