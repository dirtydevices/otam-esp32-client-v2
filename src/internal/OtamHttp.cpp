#include "internal/OtamHttp.h"
#include <HttpClient.h>

String OtamHttp::apiKey;

OtamHttpResponse OtamHttp::get(String url) {
    HTTPClient http;

    http.begin(url);
    http.addHeader("x-api-key", apiKey);

    try {
        int httpCode = http.GET();
        String response = http.getString();

        http.end();

        return {httpCode, response};
    } catch (const std::exception& e) {
        http.end();
        // Serial.println("Exception in HTTP GET: " + String(e.what()));
        throw std::runtime_error("Exception in HTTP GET Request");
    }
}

OtamHttpResponse OtamHttp::post(String url, String payload) {
    HTTPClient http;

    try {
        http.begin(url);
        http.addHeader("x-api-key", apiKey);
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);
        String response = http.getString();

        http.end();

        return {httpCode, payload: response};
    } catch (const std::exception& e) {
        http.end();
        // Serial.println("Exception in HTTP POST: " + String(e.what()));
        throw std::runtime_error("Exception in HTTP POST Request");
    }
}