#include <HttpClient.h>

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
            String resPayload = http.getString();
            http.end();
            OtamLogger::error("HTTP POST failed, error: " + String(httpCode));
            OtamLogger::error("HTTP POST payload: " + resPayload);
            throw std::runtime_error("HTTP GET failed, error: " + std::to_string(httpCode));
        }
    }

    static OtamHttpResponse post(String url, String payload)
    {
        HTTPClient http;

        http.begin(url);

        OtamLogger::verbose("HTTP POST: " + url);

        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);

        OtamLogger::verbose("HTTP POST response code: " + String(httpCode));

        if (httpCode >= 200 && httpCode < 300)
        {
            String resPayload = http.getString();

            if (!resPayload.isEmpty())
            {
                OtamLogger::verbose("HTTP POST response payload: " + resPayload);
            }

            http.end();

            return {httpCode, payload : resPayload};
        }
        else
        {
            String resPayload = http.getString();
            http.end();
            OtamLogger::error("HTTP POST failed, error: " + String(httpCode));
            OtamLogger::error("HTTP POST payload: " + resPayload);
            throw std::runtime_error("HTTP POST failed, error: " + std::to_string(httpCode));
        }
    }
};