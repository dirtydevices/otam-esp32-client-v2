#include "internal/OtamUtils.h"
#include "internal/OtamStore.h"
#include "internal/OtamHttp.h"
#include "internal/OtamUpdater.h"

class OtamDevice
{
private:
    void setDeviceId(OtamConfig config)
    {
        // If no id passed in, try to read from store
        deviceId = !config.deviceId.isEmpty() ? config.deviceId : OtamStore::readDeviceIdFromStore();

        // If still no id, generate one
        deviceId = !deviceId.isEmpty() ? deviceId : OtamUtils::generatePseudoGUID();

        // Write the device ID to the store
        OtamStore::writeDeviceIdToStore(deviceId);
    }
    void registerDevice()
    {
        // Create the payload
        String payload = "{\"deviceId\":\"" + deviceId + "\",\"deviceName\":\"" + deviceName + "\"}";

        // Register the device
        String response = OtamHttp::post(deviceRegisterUrl, payload);

        // Log the response
        Serial.println("Device registration response: " + response);
    }

public:
    String deviceId;
    String deviceName;
    String deviceUrl;
    String deviceStatusUrl;
    String deviceRegisterUrl;
    String deviceDownloadUrl;
    OtamDevice(OtamConfig config)
    {
        try
        {
            // Creates a device id if one is not passed in and stores it in NVS
            setDeviceId(config);

            // Set the device name
            deviceName = config.deviceName;

            // Set the device URL
            deviceUrl = config.url + "/devices/" + config.deviceId;

            // Set the device status URL
            deviceStatusUrl = this->deviceUrl + "/status";

            // Set the device register URL
            deviceRegisterUrl = this->deviceUrl + "/register";

            // Set the device download URL
            deviceDownloadUrl = this->deviceUrl + "/download";

            // Register device with OTAM server
            registerDevice();
        }
        catch (const std::exception &e)
        {
            // Log the error
            Serial.println(e.what());
            throw "Failed to initialize device with OTAM config";
        }
    }
};
