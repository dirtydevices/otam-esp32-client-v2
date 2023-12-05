#include "internal/OtamHttp.h"

class OtamLogDb
{
private:
    String url;

public:
    OtamLogDb(String url)
    {
        this->url = url;
    }

    void debug(String message)
    {
        // Log the message with log_type as debug
        String payload = "{\"message\":\"" + message + "\",\"type\":\"debug\"}";
        OtamHttp::post(url, payload);
    }

    void info(String message)
    {
        // Log the message with log_type as info
        String payload = "{\"message\":\"" + message + "\",\"type\":\"info\"}";
        OtamHttp::post(url, payload);
    }

    void error(String message)
    {
        // Log the message with log_type as error
        String payload = "{\"message\":\"" + message + "\",\"type\":\"error\"}";
        OtamHttp::post(url, payload);
    }
};
