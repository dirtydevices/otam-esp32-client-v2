#include <HTTPClient.h>
#include <Update.h>

class OtamUpdater
{
public:
    static void runESP32Update(HTTPClient &http)
    {
        int contentLength = http.getSize();

        bool canBegin = Update.begin(contentLength);
        if (canBegin)
        {
            WiFiClient *client = http.getStreamPtr();

            // Write the downloaded binary to flash memory
            size_t written = Update.writeStream(*client);

            if (written == contentLength)
            {
                OtamLogger::info("Written : " + String(written) + " successfully");
            }
            else
            {
                OtamLogger::info("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
            }

            if (Update.end())
            {
                OtamLogger::info("OTA done!");
                if (Update.isFinished())
                {
                    OtamLogger::info("Update successfully completed. Rebooting.");
                    ESP.restart();
                }
                else
                {
                    OtamLogger::error("Update not finished? Something went wrong!");
                }
            }
            else
            {
                OtamLogger::error("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            OtamLogger::error("Not enough space to begin OTA");
        }
    }
};