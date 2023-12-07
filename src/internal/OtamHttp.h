#include <HTTPClient.h>

class OtamHttp
{
public:
    static String get(String url)
    {
        HTTPClient http;

        http.begin(url);

        OtamLogger::debug("HTTP GET: " + url);

        int httpCode = http.GET();

        OtamLogger::debug("HTTP GET response code: " + String(httpCode));

        if (httpCode == HTTP_CODE_OK)
        {
            OtamLogger::debug("HTTP GET successful");

            String payload = http.getString();

            OtamLogger::debug("HTTP GET response payload: " + payload);

            http.end();
            return payload;
        }
        else
        {
            http.end();
            throw std::runtime_error("HTTP GET failed, error: " + std::to_string(httpCode));
        }
    }

    static String post(String url, String payload)
    {
        HTTPClient http;

        OtamLogger::debug("HTTP POST: " + url);

        http.begin(url);

        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);

        OtamLogger::debug("HTTP POST response code: " + String(httpCode));

        if (httpCode >= 200 && httpCode < 300)
        {
            String payload = http.getString();

            OtamLogger::debug("HTTP POST response payload: " + payload);

            http.end();
            return payload;
        }
        else
        {
            http.end();
            throw std::runtime_error("HTTP POST failed, error: " + std::to_string(httpCode));
        }
    }
};