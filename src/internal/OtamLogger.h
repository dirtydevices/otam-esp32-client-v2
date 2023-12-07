#include <Arduino.h>

enum OtamLogLevel
{
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};

class OtamLogger
{
public:
    static OtamLogLevel LOG_LEVEL;

    static void setLogLevel(OtamLogLevel logLevel)
    {
        LOG_LEVEL = logLevel;
    }
    static void verbose(String message)
    {
        if (LOG_LEVEL >= LOG_LEVEL_VERBOSE)
        {
            Serial.println("OTAM [VERBOSE]: " + message);
        }
    }
    static void debug(String message)
    {
        if (LOG_LEVEL >= LOG_LEVEL_DEBUG)
        {
            Serial.println("OTAM [DEBUG]: " + message);
        }
    }
    static void info(String message)
    {
        if (LOG_LEVEL >= LOG_LEVEL_INFO)
        {
            Serial.println("OTAM [INFO]: " + message);
        }
    }
    static void warn(String message)
    {
        if (LOG_LEVEL >= LOG_LEVEL_WARN)
        {
            Serial.println("OTAM [WARN]: " + message);
        }
    }
    static void error(String message)
    {
        if (LOG_LEVEL >= LOG_LEVEL_ERROR)
        {
            Serial.println("OTAM [ERROR]: " + message);
        }
    }
};

OtamLogLevel OtamLogger::LOG_LEVEL = LOG_LEVEL_INFO;