#include <HTTPClient.h>

class OtamHttp
{
public:
    static String get(String url)
    {
        HTTPClient http;

        http.begin(url);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();

            return payload;
        }
        else
        {
            Serial.println("HTTP GET failed, error: " + httpCode);
            return "";
        }
    }

    static String post(String url, String payload)
    {
        HTTPClient http;

        http.begin(url);

        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(payload);

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();

            return payload;
        }
        else
        {
            Serial.println("HTTP POST failed, error: " + httpCode);
            return "";
        }
    }
};