#include <Arduino.h>

struct OtamConfig
{
    String url = "";
    String deviceId = "";            // auto-generated if not provided
    String deviceName = "";          // initial name, name set in otam app takes precedence
    bool regenerateDeviceId = false; // if true, device id will be regenerated on every boot
};