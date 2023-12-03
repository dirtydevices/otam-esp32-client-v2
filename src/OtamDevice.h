#include <OtamConfig.h>
#include <OtamUtils.h>
#include <OtamStore.h>

class OtamDevice
{
private:
    static String deviceName;
    static String deviceId;
    static String deviceUrl;
    static void registerDevice(OtamConfig config)
    {
        Serial.println("Registering device");

        HTTPClient http;
    }

public:
    static String deviceStatusUrl;
    static String deviceDownloadUrl;
    static void initialize(OtamConfig config)
    {
        try
        {
            // If no id passed in, try to read from store
            String deviceId = !config.deviceId.isEmpty() ? config.deviceId : OtamStore::readDeviceIdFromStore();

            // If still no id, generate one
            deviceId = !deviceId.isEmpty() ? deviceId : OtamUtils::generatePseudoGUID();

            // Update the OTAM Config
            config.deviceId = deviceId;

            // Write the device ID to the store
            OtamStore::writeDeviceIdToStore(deviceId);
        }
        catch (const std::exception &e)
        {
            // Log the error
            Serial.println("Failed to initialize device with OTAM config");
            Serial.println(e.what());
        }
    }
};
