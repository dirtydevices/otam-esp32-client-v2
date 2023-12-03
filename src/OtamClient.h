#include <HTTPClient.h>
#include <Update.h>
#include <OtamConfig.h>
#include <OtamDevice.h>

class OtamClient
{
private:
    String baseUrl = "";
    OtamConfig otamConfig;
    String deviceUrl = "";

public:
    OtamClient(OtamConfig config)
    {
        otamConfig = config;

        // Set the URL
        baseUrl = config.url;

        // Set the device ID, generate one if not provided
        otamConfig.deviceId = OtamDevice::readOrGenerateDeviceID(config.deviceId);

        // Output the device ID
        Serial.println("Device ID: " + otamConfig.deviceId);

        // Set the device URL
        deviceUrl = baseUrl + "/device/" + otamConfig.deviceId;

        // Set the device name, use a default value if not provided
        otamConfig.initialzeDeviceName = config.initialzeDeviceName.isEmpty() ? "DefaultDeviceName" : config.initialzeDeviceName;
    }

    boolean hasPendingUpdate()
    {
        Serial.println("Checking for firmware update");

        HTTPClient http;

        String deviceStatusUrl = deviceUrl + "/status?deviceId=" + otamConfig.deviceId;

        Serial.println("deviceStatus url: " + deviceStatusUrl);

        http.begin(deviceStatusUrl);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();

            Serial.println("Payload: " + payload);

            // Get the update available flag
            if (payload == "UPDATE_PENDING")
            {
                Serial.println("Firmware update available");
                return true;
            }
            else
            {
                Serial.println("No firmware update available");
                return false;
            }
        }
        else
        {
            Serial.println("HTTP request failed");
        }

        http.end();
        Serial.println("Firmware check finished");
        return false;
    }

    void doFirmwareUpdate()
    {
        Serial.println("Firmware update started");

        HTTPClient http;

        String downloadUrl = deviceUrl + "/download";

        Serial.println("Firmware download url: " + downloadUrl);

        http.begin(downloadUrl);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_NO_CONTENT) // Status 204: Firmware already up to date
        {
            Serial.println("No new firmware available");
            return;
        }
        else if (httpCode == HTTP_CODE_OK) // Status 200: Download available
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
                        Serial.println("NOTE: restart has been commented out");
                        // ESP.restart();
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
};