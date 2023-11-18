#include <HTTPClient.h>
#include <Update.h>

void do_firmware_update(const String& firmwareUrl)
{
  Serial.println("Firmware update started");

  HTTPClient http;

  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
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
  else
  {
    Serial.println("HTTP request failed");
  }

  http.end();
  Serial.println("Firmware update finished");
}