#ifndef OTAM_CONFIG_H
#define OTAM_CONFIG_H

#include <Arduino.h>

struct OtamFirmwareConfig {
    String firmwareId = "";
    String firmwareVersion = "";
};

struct OtamConfig {
    String apiKey = "";      // user's api key
    String url = "";         // base otam api url
    String deviceId = "";    // auto-generated if not provided
    String deviceName = "";  // initial name, name set in otam app takes precedence
    String logLevel = "LOG_LEVEL_INFO";
    bool regenerateDeviceId = false;  // if true, device id will be regenerated on every boot
    OtamFirmwareConfig firmwareConfig;
};

#endif  // OTAM_CONFIG_H