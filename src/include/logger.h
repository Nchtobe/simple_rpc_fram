#pragma once

#include "lockqueue.h"
#include <string>

enum LogLevel
{
    INFO,  // normal
    ERROR, // error
};

// log system
class Logger
{
public:
    // get Logger object
    static Logger &GetInstance();
    // set loglevel
    void SetLogLevel(LogLevel level);
    // write log
    void Log(std::string msg);

private:
    int m_loglevel;
    LockQueue<std::string> m_lckQue;

    // Singleton
    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
};

// macro
#define LOG_INFO(logmsgformat, ...)                     \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLogLevel(INFO);                       \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0);

#define LOG_ERR(logmsgformat, ...)                      \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLogLevel(ERROR);                      \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0);
