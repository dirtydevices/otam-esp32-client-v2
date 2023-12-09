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
        OtamLogger::debug("Device id written to store: " + id);
    }

    void setDeviceId(OtamConfig config)
    {
        deviceId = OtamStore::readDeviceIdFromStore();

        if (deviceId)
        {
            OtamLogger::verbose("Device id read from store: " + deviceId);
        }

        if (!config.deviceId.isEmpty())
        {
            if (deviceId != config.deviceId)
            {
                deviceId = config.deviceId;
                OtamLogger::info("Device id passed by config: " + config.deviceId + ". Writing to store.");
                writeIdToStore(config.deviceId);
            }
            else
            {
                OtamLogger::debug("Device id already in store: " + deviceId);
            }
        }
        else if (deviceId.isEmpty() || config.regenerateDeviceId == true)
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
        OtamLogger::info("Registering device with OTAM server");

        // Create the payload
        String payload = "{\"deviceName\":\"" + deviceName + "\"}";

        // Register the device
        OtamHttpResponse response = OtamHttp::post(deviceRegisterUrl, payload);

        // Check the response code
        if (response.httpCode == 204)
        {
            OtamLogger::debug("Device already registered and nothing to update.");
        }
        else if (response.httpCode >= 200 && response.httpCode < 300)
        {
            // Log the response
            OtamLogger::info(response.payload);
        }
        else
        {
            OtamLogger::error("Device registration failed with status code " + String(response.httpCode));
            OtamLogger::error(response.payload);
            throw std::runtime_error("Registration failed.");
        }
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
