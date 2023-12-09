#include <HttpClient.h>
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

    OtamHttpResponse logDeviceMessage(String message)
    {
        // Create the payload
        String payload = "{\"message\":\"" + message + "\"}";

        // Send the log entry
        OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, payload);

        if (response.httpCode >= 200 && response.httpCode < 300)
        {
            OtamLogger::debug("Device message logged successfully: " + message);
        }
        else
        {
            throw std::runtime_error("Device message logging failed");
        }

        return response;
    }

    boolean hasPendingUpdate()
    {
        OtamHttpResponse response = OtamHttp::get(otamDevice->deviceStatusUrl);
        if (response.payload == "UPDATE_PENDING")
        {
            OtamLogger::verbose("Firmware update available");
            return true;
        }
        else
        {
            OtamLogger::verbose("No firmware update available");
            return false;
        }
    }

    void doFirmwareUpdate()
    {
        OtamLogger::info("Firmware update started");

        HTTPClient http;

        OtamLogger::debug("Downloading firmware from: " + otamDevice->deviceDownloadUrl);

        http.begin(otamDevice->deviceDownloadUrl);

        OtamLogger::debug("HTTP GET: " + otamDevice->deviceDownloadUrl);

        int httpCode = http.GET();

        OtamLogger::debug("HTTP GET response code: " + String(httpCode));

        if (httpCode == HTTP_CODE_NO_CONTENT) // Status 204: Firmware already up to date
        {
            OtamLogger::info("No new firmware available");
            return;
        }
        else if (httpCode == HTTP_CODE_OK) // Status 200: Download available
        {
            OtamLogger::info("New firmware available");
            OtamUpdater::runESP32Update(http);
        }
        else
        {
            OtamLogger::error("Firmware download failed, error: " + String(httpCode));
            throw std::runtime_error("Firmware download failed.");
        }
    }
};