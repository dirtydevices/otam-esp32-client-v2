#include "internal/OtamLogDb.h"

OtamLogDb::OtamLogDb(String url) {
    this->url = url;
}

void OtamLogDb::debug(String message) {
    // Log the message with log_type as debug
    String payload = "{\"message\":\"" + message + "\",\"type\":\"debug\"}";
    OtamHttp::post(url, payload);
}

void OtamLogDb::info(String message) {
    // Log the message with log_type as info
    String payload = "{\"message\":\"" + message + "\",\"type\":\"info\"}";
    OtamHttp::post(url, payload);
}

void OtamLogDb::error(String message) {
    // Log the message with log_type as error
    String payload = "{\"message\":\"" + message + "\",\"type\":\"error\"}";
    OtamHttp::post(url, payload);
}