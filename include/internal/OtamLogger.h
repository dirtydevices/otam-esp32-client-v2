#ifndef OTAM_LOGGER_H
#define OTAM_LOGGER_H

#include <Arduino.h>

enum OtamLogLevel { LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_VERBOSE };

class OtamLogger {
   public:
    static OtamLogLevel LOG_LEVEL;

    static void setLogLevel(OtamLogLevel logLevel);
    static void verbose(String message);
    static void debug(String message);
    static void info(String message);
    static void warn(String message);
    static void error(String message);
};

#endif  // OTAM_LOGGER_H