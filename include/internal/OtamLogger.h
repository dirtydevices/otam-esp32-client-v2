#ifndef OTAM_LOGGER_H
#define OTAM_LOGGER_H

#include <Arduino.h>

namespace Otam {

enum class LogLevel { NONE, ERROR, WARN, INFO, HTTP, VERBOSE, DEBUG, SILLY };

class Logger {
   public:
    static LogLevel LOG_LEVEL;

    static void setLogLevel(LogLevel logLevel);
    static void silly(String message);
    static void debug(String message);
    static void verbose(String message);
    static void http(String message);
    static void info(String message);
    static void warn(String message);
    static void error(String message);
};

}  // namespace Otam

#endif  // OTAM_LOGGER_H