#include "internal/OtamConfig.h"
#include "internal/OtamLogger.h"
#include "internal/OtamHttp.h"
#include "internal/OtamDevice.h"

class OtamClient
{
private:
    OtamDevice *otamDevice;

public:
    void setLogLevel(OtamLogLevel logLevel)
    {
        Serial.println("Setting log level to: " + String(logLevel));
        OtamLogger::setLogLevel(logLevel);
    }

    void initialize(OtamConfig config)
    {
        OtamLogger::debug("Initializing OTAM client");

        // Create the device
        otamDevice = new OtamDevice(config);
    }

    void logDeviceMessage(String message)
    {
        // Create the payload
        String payload = "{\"message\":\"" + message + "\"}";

        // Send the log entry
        String response = OtamHttp::post(otamDevice->deviceLogUrl, payload);

        if (response == "OK")
        {
            Serial.println("Device message logged successfully: " + message);
        }
        else
        {
            throw std::runtime_error("Device message logging failed");
        }
    }

    boolean hasPendingUpdate()
    {
        String response = OtamHttp::get(otamDevice->deviceStatusUrl);
        if (response == "UPDATE_PENDING")
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

    void doFirmwareUpdate()
    {
        Serial.println("Firmware update started");

        HTTPClient http;

        http.begin(otamDevice->deviceDownloadUrl);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_NO_CONTENT) // Status 204: Firmware already up to date
        {
            Serial.println("No new firmware available");
            return;
        }
        else if (httpCode == HTTP_CODE_OK) // Status 200: Download available
        {
            OtamUpdater::runESP32Update(http);
        }
        else
        {
            throw std::runtime_error("Firmware download failed, error: " + httpCode);
        }
    }
};