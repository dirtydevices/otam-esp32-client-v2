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
                Serial.println("Written : " + String(written) + " successfully");
            }
            else
            {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
            }

            if (Update.end())
            {
                Serial.println("OTA done!");
                if (Update.isFinished())
                {
                    Serial.println("Update successfully completed. Rebooting.");
                    ESP.restart();
                }
                else
                {
                    Serial.println("Update not finished? Something went wrong!");
                }
            }
            else
            {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            Serial.println("Not enough space to begin OTA");
        }
    }
}