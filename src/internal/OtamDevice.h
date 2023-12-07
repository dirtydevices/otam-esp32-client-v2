#include "internal/OtamLogDb.h"
#include "internal/OtamUtils.h"
#include "internal/OtamStore.h"
#include "internal/OtamUpdater.h"

class OtamDevice
{
private:
    void writeIdToStore(String id)
    {
        OtamStore::writeDeviceIdToStore(id);
        Serial.println("Device id written to store: " + id);
    }

    void setDeviceId(OtamConfig config)
    {
        deviceId = OtamStore::readDeviceIdFromStore();

        if (!config.deviceId.isEmpty())
        {
            if (deviceId != config.deviceId)
            {
                OtamLogger::info("Device id passed by config: " + config.deviceId + "Writing to store.");
                writeIdToStore(config.deviceId);
            }
            else
            {
                OtamLogger::debug("Device id already in store: " + deviceId);
            }
        }
        else if (deviceId.isEmpty())
        {
            deviceId = OtamUtils::generatePseudoGUID();
            OtamLogger::info("Generated new device id: " + deviceId);
            writeIdToStore(deviceId);
        }
        else
        {
            OtamLogger::debug("Device id already in store: " + deviceId);
        }
    }
    void registerDevice()
    {
        Serial.println("Registering device with OTAM server");

        // Create the payload
        String payload = "{\"deviceName\":\"" + deviceName + "\"}";

        // Register the device
        String response = OtamHttp::post(deviceRegisterUrl, payload);

        // Log the response
        OtamLogger::info("Device registration response: " + response);
    }

public:
    String deviceId;
    String deviceName;
    String deviceUrl;
    String deviceLogUrl;
    String deviceStatusUrl;
    String deviceRegisterUrl;
    String deviceDownloadUrl;
    OtamLogDb *logDb;
    OtamDevice(OtamConfig config)
    {
        // Creates a device id if one is not passed in and stores it in NVS
        setDeviceId(config);

        // Set the device name
        deviceName = config.deviceName;

        // Set the device URL
        deviceUrl = config.url + "/devices/" + deviceId;

        // Set the device log URL
        deviceLogUrl = this->deviceUrl + "/log";

        // Set the device status URL
        deviceStatusUrl = this->deviceUrl + "/status";

        // Set the device register URL
        deviceRegisterUrl = this->deviceUrl + "/register";

        // Set the device download URL
        deviceDownloadUrl = this->deviceUrl + "/download";

        // Create the db logger
        logDb = new OtamLogDb(deviceLogUrl);

        // Register device with OTAM server
        registerDevice();
    }
};
