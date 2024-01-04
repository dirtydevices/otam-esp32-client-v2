#include "internal/OtamHttp.h"
#include <HttpClient.h>

String OtamHttp::apiKey;

OtamHttpResponse OtamHttp::get(String url) {
    HTTPClient http;

    try {
        http.begin(url);
        http.addHeader("x-api-key", apiKey);

        int httpCode = http.GET();

        // Serial.println("HTTP GET [" + String(httpCode) + "] - " + url);

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();

            // if (!payload.isEmpty()) {
            //     Serial.println("HTTP GET response payload: " + payload);
            // }

            http.end();

            return {httpCode, payload};
        } else {
            String resPayload = http.getString();
            // Serial.println("HTTP POST failed, error: " + String(httpCode));
            // Serial.println("HTTP POST payload: " + resPayload);
            throw std::runtime_error("HTTP GET failed, error: " + std::to_string(httpCode));
        }
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

        // Serial.println("HTTP POST [" + String(httpCode) + "] - " + url);

        if (httpCode >= 200 && httpCode < 300) {
            String resPayload = http.getString();

            // if (!resPayload.isEmpty()) {
            //     Serial.println("HTTP POST response payload: " + resPayload);
            // }

            http.end();

            return {httpCode, payload: resPayload};
        } else {
            String resPayload = http.getString();
            // Serial.println("HTTP POST failed, error: " + String(httpCode));
            // Serial.println("HTTP POST payload: " + resPayload);
            throw std::runtime_error("HTTP POST failed, error: " + std::to_string(httpCode));
        }
    } catch (const std::exception& e) {
        http.end();
        // Serial.println("Exception in HTTP POST: " + String(e.what()));
        throw std::runtime_error("Exception in HTTP POST Request");
    }
}