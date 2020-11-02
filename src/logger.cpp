#include "logger.h"
#include "utils.h"
#include <cstring>
#include <sys/time.h>
#include <sys/stat.h>

const char * loglevel[] = {
    "[NONE]",
    "[FATAL]",
    "[CRITICAL]",
    "[ERROR]",
    "[WARNING]",
    "[INFO]",
    "[DEBUG]",
    "[ALL]"
};
const char * logleveldb[] = {
    "NONE",
    "FATAL",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "ALL"
};

#define C_IS_SET(a) (m_config & a) == a

/*****************************************************************************
 *
 *****************************************************************************/
Logger::Logger(void) : m_lock()  {
    m_prefix = NULL;
    m_fp     = NULL;
    // Default config
    m_ll     = LL_DEBUG;
    m_config = LC_DATE | LC_THREADID | LC_LEVEL | LC_STDOUT | LC_THREADSAFE | LC_FLUSH;
    m_filename[0] = '\0';
    m_hostname = NULL;
    m_line.reserve(1024);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Logger::~Logger(void) {
    Close();
    delete [] m_hostname;
    delete [] m_prefix;
    return;
}


void Logger::SetHostname(const char * ip) {
    m_hostname = nstrdup(ip);
    return;
}


/*****************************************************************************
 *
 *****************************************************************************/
void Logger::SetMaxLogLevel(const char * ll) {
    for (int i = 0; i < LL_ALL; i++) {
        if (!strcmp(ll, logleveldb[i])) {
            SetMaxLogLevel((LogLevel)i);
            return;
        }
    }
    SetMaxLogLevel(LL_ALL);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::LogLock(void) {
    if (C_IS_SET(LC_THREADSAFE)) m_lock.Lock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::LogUnLock(void) {
    if (C_IS_SET(LC_THREADSAFE)) m_lock.UnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::SetPrefix(const char * filename) {
    LogLock();
    m_prefix = nstrdup(filename);
    Open();
    LogUnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::Open(void) {
    if (!m_prefix) return;
    Close();
    SetFilename();
    m_fp = fopen(m_filename, "a+");
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::Close(void) {
    if (!m_fp) return;
    fclose(m_fp);
    m_fp = NULL;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::Rotate(void) {
    struct stat logstat;
    if (!m_fp || !m_filename) return;

    // TODO Delete old logfiles?

    // File does not exist or larger than 100MB
    if (stat(m_filename, &logstat) || logstat.st_size > 104857600) {
        SetFilename();
        Open();
    }
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::SetFilename(void) {
    char tmp[56];
    struct stat logstat;
    struct tm stm;
    int     lognum = 1;
    time_t  timet;
    
    LogLock();
    if (!m_prefix) {
        LogUnLock();
        return;
    }

    timet= time(NULL);
    gmtime_r(&timet, &stm);
    
    strftime(tmp, 56, "_%Y-%m-%d", &stm); 
    snprintf(m_filename, 56, "%s%s.log",  m_prefix, tmp);

    // File does not exist
    if (stat(m_filename, &logstat)) {
        LogUnLock();
        return;
    }
  
    // Rotate at 10MB
    while (logstat.st_size > 10485760) {
        strftime(tmp, 56, "_%Y-%m-%d", &stm); 
        snprintf(m_filename, 56, "%s%s-%d.log", m_prefix, tmp, lognum);
        if (stat(m_filename, &logstat)) {
            LogUnLock();
            return;
        }
        lognum++;
    }
    LogUnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::Log(LogLevel ll, const char * msg, ...) {
    va_list   marker;
    LogLock();
    va_start(marker, msg);
    VLog(ll, msg, &marker);
    va_end(marker);
    LogUnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Logger::VLog(LogLevel ll, const char * msg, va_list * marker) {
    
    // Print to stdout or not
    //bool sout = ((C_IS_SET(LC_STDOUT)) && (ll <= LL_ERROR));
    bool sout = C_IS_SET(LC_STDOUT); //&& (ll <= LL_ERROR));
    //bool sout = false;
    
    // Max loging detail
    if (ll > m_ll) return;

    LogLock();
    
    // Build the logline
    vsnprintf(m_buff, MAX_LOG_SIZE, msg, *marker);   

    // Log to the file or stdout
    if (sout || m_fp) {
        struct timeval tv;
        struct tm stm;
        char buff[32];
        // Rotate if necessary
        m_line.clear();
        // Log date
        if (C_IS_SET(LC_DATE)) {
            gettimeofday(&tv, NULL);
            gmtime_r(&tv.tv_sec, &stm);
            strftime(buff, 32, "[%Y-%m-%d %T.", &stm);
            m_line += buff;
        
            // Millisecs
            snprintf(buff, 32, "%03ld]", tv.tv_usec / 1000);
            m_line += buff;
        }
            
        // Log error level
        if (C_IS_SET(LC_LEVEL)) 
            m_line += loglevel[ll];
                    
        // Log threadid
        if (C_IS_SET(LC_THREADID)) {
            snprintf(buff, 32, "[%lu]", pthread_self());
            m_line += buff;
        }

        m_line += m_buff;
        m_line += "\n";

        // Do the writing
        if (m_fp) {
            Rotate();
            fwrite(m_line.c_str(), m_line.length(), 1, m_fp);    
            if (C_IS_SET(LC_FLUSH)) fflush(m_fp);
        }
        if (sout) {
            printf("%s", m_line.c_str());
            if (C_IS_SET(LC_FLUSH)) fflush(stdout);
        }
    }
    LogUnLock();
    return;
}

/*===========================================================================*/
/*===========================================================================*/
Logger * GlobalLogger::m_logger = NULL;

/*****************************************************************************
 *
 *****************************************************************************/
void GlobalLogger::Initialize(Logger * logger) {
    GlobalLogger::m_logger = logger;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Logger * GlobalLogger::Get(void) {
    return GlobalLogger::m_logger;
}


const char * Logger::LogLevelString(LogLevel ll) {
    if (ll >= LL_FATAL && ll <= LL_DEBUG) 
        return loglevel[ll];
    return "";
}

LogLevel Logger::StringLogLevel(const char * ll) {
    if (!ll) 
        return LL_ALL;
    for (int i = LL_NONE; i <= LL_DEBUG; i++) {
        if (!strcmp(loglevel[i], ll) )
            return (LogLevel)i;
    }
    return LL_ALL;
}


/*****************************************************************************
 *
 *****************************************************************************/
void GlobalLogger::Log(LogLevel ll, const char * msg, ...) {
    va_list   marker;
    if (m_logger) {
        m_logger->LogLock();
        va_start(marker, msg);
        m_logger->VLog(ll, msg, &marker);
        va_end(marker);
        m_logger->LogUnLock();
    }
    return;
}

/*===========================================================================*/
/*===========================================================================*/

/*****************************************************************************
 *
 *****************************************************************************/
ClassLogger::ClassLogger(const char * clsname) {
    m_clsname = nstrdup(clsname);
}

/*****************************************************************************
 *
 *****************************************************************************/
ClassLogger::~ClassLogger(void) {
    delete [] m_clsname;
}

LogLevel ClassLogger::GetMaxLogLevel(void) {
    return GlobalLogger::Get()->GetMaxLogLevel();
}

/*****************************************************************************
 *
 *****************************************************************************/
void ClassLogger::Log(LogLevel ll, const char * msg, ...) {
    va_list marker;
    GlobalLogger::Get()->LogLock();
    va_start(marker, msg);
    if (!m_clsname) {
        GlobalLogger::Get()->VLog(ll, msg, &marker);
    }
    else {
        snprintf(m_buff, MAX_LOG_SIZE, "[%s] %s", m_clsname, msg);
        GlobalLogger::Get()->VLog(ll, m_buff, &marker);
    }
    va_end(marker);
    GlobalLogger::Get()->LogUnLock();
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void ClassLogger::LogSysError(const char * msg) {
    char logline[MAX_LOG_SIZE];
    char tmp[128];
    
    snprintf(logline, 128, "%s (%d): %s", msg, errno, strerror_r(errno, tmp, sizeof(char) * 128));
    Log(LL_ERROR, logline);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void ClassLogger::SetClassName(const char * name) {
    delete [] m_clsname;
    m_clsname = nstrdup(name);
    return;
}

