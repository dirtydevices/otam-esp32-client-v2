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

    static void writeFirmwareUpdateFileIdToStore(int firmwareUpdateFileId)
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (preferences.putInt("file_id", firmwareUpdateFileId) == 0)
        {
            throw std::runtime_error("Failed to write firmware file update ID to NVS");
        }

        preferences.end();
    }

    static int readFirmwareUpdateFileIdFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        int firmwareUpdateFileId = preferences.getInt("file_id");
        preferences.end();
        return firmwareUpdateFileId;
    }

    static void writeFirmwareUpdateIdToStore(int firmwareUpdateId)
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (preferences.putInt("firmware_id", firmwareUpdateId) == 0)
        {
            throw std::runtime_error("Failed to write firmware update ID to NVS");
        }

        preferences.end();
    }

    static int readFirmwareUpdateIdFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        int firmwareUpdateId = preferences.getInt("firmware_id");
        preferences.end();
        return firmwareUpdateId;
    }

    static void writeFirmwareUpdateNameToStore(String firmwareUpdateName)
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (!preferences.putString("firmware_name", firmwareUpdateName))
        {
            throw std::runtime_error("Failed to write firmware update name to NVS");
            preferences.end();
            return;
        }

        preferences.end();
    }

    static String readFirmwareUpdateNameFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        String firmwareUpdateName;

        if (preferences.isKey("firmware_name"))
        {
            firmwareUpdateName = preferences.getString("firmware_name"); // Try to read the device ID from NVS
        }

        preferences.end();
        return firmwareUpdateName;
    }

    static void writeFirmwareUpdateVersionToStore(String firmwareUpdateVersion)
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (!preferences.putString("fw_version", firmwareUpdateVersion))
        {
            throw std::runtime_error("Failed to write firmware update version to NVS");
            preferences.end();
            return;
        }

        preferences.end();
    }

    static String readFirmwareUpdateVersionFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        String firmwareUpdateVersion;

        if (preferences.isKey("fw_version"))
        {
            firmwareUpdateVersion = preferences.getString("fw_version"); // Try to read the device ID from NVS
        }

        preferences.end();
        return firmwareUpdateVersion;
    }

    static void writeFirmwareUpdateStatusToStore(String firmwareUpdateStatus)
    {
        Preferences preferences;

        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        if (!preferences.putString("fw_status", firmwareUpdateStatus))
        {
            throw std::runtime_error("Failed to write firmware update status to NVS");
            preferences.end();
            return;
        }

        preferences.end();
    }

    static String readFirmwareUpdateStatusFromStore()
    {
        Preferences preferences;
        if (!preferences.begin("otam-store", false))
        {
            throw std::runtime_error("Failed to initialize NVS");
        }

        String firmwareUpdateStatus;

        if (preferences.isKey("fw_status"))
        {
            firmwareUpdateStatus = preferences.getString("fw_status"); // Try to read the device ID from NVS
        }

        preferences.end();
        return firmwareUpdateStatus;
    }
};
