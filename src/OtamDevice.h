#include <Preferences.h>

String generatePseudoGUID()
{
    String guid = "otam_";
    static const char alphanum[] = "0123456789ABCDEF";

    for (int i = 0; i < 8; i++)
    {
        guid += alphanum[random(16)];
    }
    guid += "-";
    for (int i = 0; i < 4; i++)
    {
        guid += alphanum[random(16)];
    }
    guid += "-";
    for (int i = 0; i < 4; i++)
    {
        guid += alphanum[random(16)];
    }

    return guid;
}

String readOrGenerateDeviceID(const String &newDeviceID = "")
{
    Preferences preferences;
    if (!preferences.begin("my-app", false))
    {
        Serial.println("Failed to initialize NVS");
        return "";
    }

    String deviceId;

    if (!newDeviceID.isEmpty())
    {
        deviceId = newDeviceID; // Device Id provided, use it
    }
    else if (preferences.isKey("device_id"))
    {
        deviceId = preferences.getString("device_id"); // Try to read the device ID from NVS
    }
    else
    {
        deviceId = generatePseudoGUID(); // Simple random generator for example purposes
    }

    // Write the new or provided device ID to NVS
    if (!preferences.putString("device_id", deviceId))
    {
        Serial.println("Failed to write device ID to NVS");
        preferences.end();
        return "";
    }

    preferences.end();
    return deviceId;
}