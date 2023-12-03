#include <Arduino.h>

struct OtamConfig
{
    String url;
    String deviceId = "";            // auto-generated if not provided
    String initialzeDeviceName = ""; // initial name, name set in otam app takes precedence
};