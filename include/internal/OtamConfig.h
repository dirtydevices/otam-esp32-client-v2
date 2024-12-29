#ifndef OTAM_CONFIG_H
#define OTAM_CONFIG_H

#include <Arduino.h>

struct OtamConfig {
    String apiKey = "";           // user's api key
    String url = "";              // base otam api url
    String deviceId = "";         // device id
    int deviceProfileId;          // device profile id
    int firmwareId;               // firmware id
    String firmwareVersion = "";  // firmware version
};

#endif  // OTAM_CONFIG_H