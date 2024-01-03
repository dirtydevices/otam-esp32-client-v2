#include "internal/OtamLogger.h"
#include <Arduino.h>

OtamLogLevel OtamLogger::LOG_LEVEL = OtamLogLevel::LOG_LEVEL_NONE;

void OtamLogger::setLogLevel(OtamLogLevel logLevel) {
    LOG_LEVEL = logLevel;
}

void OtamLogger::silly(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_SILLY) {
        Serial.println("OTAM [SILLY]: " + message);
    }
}

void OtamLogger::debug(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_DEBUG) {
        Serial.println("OTAM [DEBUG]: " + message);
    }
}

void OtamLogger::verbose(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_VERBOSE) {
        Serial.println("OTAM [VERBOSE]: " + message);
    }
}

void OtamLogger::http(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_VERBOSE) {
        Serial.println("OTAM [HTTP]: " + message);
    }
}

void OtamLogger::info(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_INFO) {
        Serial.println("OTAM [INFO]: " + message);
    }
}

void OtamLogger::warn(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_WARN) {
        Serial.println("OTAM [WARN]: " + message);
    }
}

void OtamLogger::error(String message) {
    if (LOG_LEVEL >= OtamLogLevel::LOG_LEVEL_ERROR) {
        Serial.println("OTAM [ERROR]: " + message);
    }
}