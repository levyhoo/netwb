#include "Clog.h"
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/transcoder.h>

Clog::Clog(void)
{
}

Clog::~Clog(void)
{
}

void Clog::init(string strLoggerCfgFile, bool bWatch)
{
    if (bWatch)
    {
        log4cxx::PropertyConfigurator::configureAndWatch(strLoggerCfgFile);
    }
    else
    {
        log4cxx::PropertyConfigurator::configure(strLoggerCfgFile);
    }
}

void Clog::enableLog(bool bEnabled)
{
    m_bEnabled = bEnabled;
}

bool Clog::logEnabled() const
{
    return m_bEnabled;
}

void Clog::doLog(const string & strLoggerName, ELogLevel level, const string & strMessage)
{
    if (m_bEnabled)
    {
        LOG4CXX_LOG(log4cxx::Logger::getLogger(strLoggerName), log4cxx::Level::toLevel(LOG4CXX_LEVEL_INT[level]), strMessage.c_str());
    }
}



void StdLogNew(const log4cxx::LoggerPtr& logger, const log4cxx::LevelPtr& level, const char * const szFormat, ...)
{
    va_list v;
    va_start(v, szFormat);

    static const int c_sMaxBuf = 4096;
    char szBuffer[c_sMaxBuf];
    memset(szBuffer, 0, c_sMaxBuf * sizeof(char));
    int nBuf = vsnprintf(szBuffer, c_sMaxBuf, szFormat, v);

    if (nBuf < 0)
    {
        szBuffer[c_sMaxBuf - 1] = 0;
    }
    if (nBuf < c_sMaxBuf)
    {
        log4cxx::helpers::MessageBuffer oss_; 
        logger->forcedLog(level, oss_.str(oss_ << szBuffer), LOG4CXX_LOCATION); 
    }
    va_end(v);
}
