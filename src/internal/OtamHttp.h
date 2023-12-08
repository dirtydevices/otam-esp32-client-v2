#include <HTTPClient.h>

struct OtamHttpResponse
{
    int httpCode;
    String payload;
};

class OtamHttp
{
public:
    static OtamHttpResponse get(String url)
    {
        HTTPClient http;

        http.begin(url);

        OtamLogger::verbose("HTTP GET: " + url);

        int httpCode = http.GET();

        OtamLogger::verbose("HTTP GET response code: " + String(httpCode));

        if (httpCode == HTTP_CODE_OK)
        {
            OtamLogger::verbose("HTTP GET successful");

            String payload = http.getString();

            if (!payload.isEmpty())
            {
                OtamLogger::verbose("HTTP GET response payload: " + payload);
            }

            http.end();

            return {httpCode, payload};
        }
        else
        {
            http.end();

            throw std::runtime_error("HTTP GET failed, error: " + std::to_string(httpCode));
        }
    }

    static OtamHttpResponse post(String url, String payload)
    {
        HTTPClient http;

        OtamLogger::verbose("HTTP POST: " + url);

        http.begin(url);

        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);

        OtamLogger::verbose("HTTP POST response code: " + String(httpCode));

        if (httpCode >= 200 && httpCode < 300)
        {
            String payload = http.getString();

            if (!payload.isEmpty())
            {
                OtamLogger::verbose("HTTP POST response payload: " + payload);
            }

            http.end();

            return {httpCode, payload};
        }
        else
        {
            http.end();

            throw std::runtime_error("HTTP POST failed, error: " + std::to_string(httpCode));
        }
    }
};