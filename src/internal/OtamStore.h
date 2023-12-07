#include <Preferences.h>

class OtamStore
{
public:
    static String readDeviceIdFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        String deviceId;

        if (preferences.isKey("device_id"))
        {
            deviceId = preferences.getString("device_id"); // Try to read the device ID from NVS
        }

        preferences.end();
        return deviceId;
    }

    static void writeDeviceIdToStore(String deviceId)
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (!preferences.putString("device_id", deviceId))
        {
            throw std::runtime_error("Failed to write device ID to NVS");
            preferences.end();
            return;
        }

        preferences.end();
    }
};
