#include "internal/OtamConfig.h"
#include "internal/OtamHttp.h"
#include "internal/OtamDevice.h"

class OtamClient
{
private:
    OtamDevice *otamDevice;

public:
    OtamClient(OtamConfig config)
    {
        otamDevice = new OtamDevice(config);
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
            throw "Firmware download failed, error: " + httpCode;
        }
    }
};