#ifndef __CLOG__H_
#define __CLOG__H_

#include <stdarg.h>
#include <log4cxx/logger.h>
#include <log4cxx/level.h>
#include "Singleton.h"
#include <string>
using namespace std;

enum ELogLevel
{
    LLV_TRACE,
    LLV_DEBUG,
    LLV_INFO,
    LLV_WARN,
    LLV_ERROR,
    LLV_FATAL,
    _C_LEVEL_COUNT
};

static const int LOG4CXX_LEVEL_INT[_C_LEVEL_COUNT] = {
    log4cxx::Level::TRACE_INT,
    log4cxx::Level::DEBUG_INT,
    log4cxx::Level::INFO_INT,
    log4cxx::Level::WARN_INT,
    log4cxx::Level::ERROR_INT,
    log4cxx::Level::FATAL_INT,
};


class  Clog : public Singleton<Clog>
{
public:
    Clog(void);
    ~Clog(void);

    void enableLog(bool bEnabled);
    bool logEnabled() const;
    void init(string strLoggerCfgFile, bool bWatch);

protected:
    bool m_bEnabled;
    void doLog(const string & strLoggerName, ELogLevel level, const string & strMessage);

};




void StdLogNew(const log4cxx::LoggerPtr& logger, const log4cxx::LevelPtr& level, const char * const szFormat, ... );

#define STDLOG(lv, szFormat, ...)    \
    if (Clog::getInstance()->logEnabled())    \
    {   \
        log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("StdFile");    \
        log4cxx::LevelPtr level = log4cxx::Level::toLevel(LOG4CXX_LEVEL_INT[lv]);   \
        if (logger->isEnabledFor(level))    \
        {   \
            StdLogNew(logger, level, szFormat,##__VA_ARGS__);\
        }   \
    }

#define LOG(lv, f)    \
    if (Clog::getInstance()->logEnabled())    \
{   \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("StdFile");    \
    log4cxx::LevelPtr level = log4cxx::Level::toLevel(LOG4CXX_LEVEL_INT[lv]);   \
    if (logger->isEnabledFor(level))    \
{   \
    ::log4cxx::helpers::MessageBuffer oss_; \
    string s(f); \
    logger->forcedLog(level, oss_.str(oss_ << s), LOG4CXX_LOCATION);    \
}   \
}

#define STD_LOG_DEBUG(log) LOG(LLV_DEBUG, log)
#define STD_LOG_INFO(log)  LOG(LLV_INFO, log)
#define STD_LOG_WARN(log)  LOG(LLV_WARN, log)
#define STD_LOG_ERROR(log) LOG(LLV_ERROR, log)
#define STD_LOG_FATAL(log) LOG(LLV_FATAL, log)
#define STD_LOG_TRACE(log) LOG(LLV_TRACE, log)

#endif
