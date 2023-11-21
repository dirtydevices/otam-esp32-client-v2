#include <HTTPClient.h>
#include <Update.h>
#include <OtamDevice.h>

struct OTAMConfig
{
    String url;
    String firmwareId;
    String deviceId = "";
    String deviceName = "";
    String currentFirmwareVersion = "";
};

struct FirmwareUpdateInfo
{
    String targetFirmwareTag = "";
    String targetFirmwareVersion = "";
};

// Global variable to hold the configuration
OTAMConfig otamConfig;

// Firmware url base
String firmware_url = "";

void otamInit(OTAMConfig config)
{
    // Set the URL
    otamConfig.url = config.url;

    // Set the device ID, generate one if not provided
    otamConfig.deviceId = readOrGenerateDeviceID(config.deviceId);

    // Set the device name, use a default value if not provided
    otamConfig.deviceName = config.deviceName.isEmpty() ? "DefaultDeviceName" : config.deviceName;

    // Output the device ID
    Serial.println("Device ID: " + otamConfig.deviceId);

    // Set the current firmware version
    otamConfig.currentFirmwareVersion = config.currentFirmwareVersion;

    // Set the firmware ID
    otamConfig.firmwareId = config.firmwareId;

    // Additional initialization procedures can be added here
    firmware_url = otamConfig.url + "/firmware/" + otamConfig.firmwareId;
}

void doFirmwareUpgrade(const FirmwareUpdateInfo &updateInfo)
{
    Serial.println("Firmware update started");

    HTTPClient http;

    String download_url = firmware_url + "/download" + "?targetFirmwareTag=" + updateInfo.targetFirmwareTag;
    download_url += "&currentFirmwareVersion=" + otamConfig.currentFirmwareVersion;

    Serial.println("Firmware download url: " + download_url);

    http.begin(download_url);

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