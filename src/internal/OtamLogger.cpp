#include "internal/OtamLogger.h"
#include <Arduino.h>

// Use the Otam namespace
namespace Otam {

// Initialize LOG_LEVEL
LogLevel Logger::LOG_LEVEL = LogLevel::NONE;

void Logger::setLogLevel(LogLevel logLevel) {
    LOG_LEVEL = logLevel;
}

void Logger::silly(String message) {
    if (LOG_LEVEL >= LogLevel::SILLY) {
        Serial.println("OTAM [SILLY]: " + message);
    }
}

void Logger::debug(String message) {
    if (LOG_LEVEL >= LogLevel::DEBUG) {
        Serial.println("OTAM [DEBUG]: " + message);
    }
}

void Logger::verbose(String message) {
    if (LOG_LEVEL >= LogLevel::VERBOSE) {
        Serial.println("OTAM [VERBOSE]: " + message);
    }
}

void Logger::http(String message) {
    if (LOG_LEVEL >= LogLevel::VERBOSE) {
        Serial.println("OTAM [HTTP]: " + message);
    }
}

void Logger::info(String message) {
    if (LOG_LEVEL >= LogLevel::INFO) {
        Serial.println("OTAM [INFO]: " + message);
    }
}

void Logger::warn(String message) {
    if (LOG_LEVEL >= LogLevel::WARN) {
        Serial.println("OTAM [WARN]: " + message);
    }
}

void Logger::error(String message) {
    if (LOG_LEVEL >= LogLevel::ERROR) {
        Serial.println("OTAM [ERROR]: " + message);
    }
}

}  // namespace Otam