#include "internal/OtamDevice.h"

void OtamDevice::writeIdToStore(String id) {
    OtamStore::writeDeviceGuidToStore(id);
    // Serial.println("Device id written to store: " + id);
}

void OtamDevice::initialize(OtamConfig config) {
    // writeIdToStore("");
    Serial.println("Initializing device with OTAM server");

    // Set the init url
    String initUrl = config.url + "/init-device";

    // Set the payload
    // With deviceId, deviceProfileId, firmwareId, firmwareVersion
    String payload =
        "{\"deviceId\":\"" + config.deviceId + "\",\"firmwareVersion\":\"" + config.firmwareVersion + "\"";

    if (config.firmwareId != 0) {
        payload += ",\"firmwareId\":" + String(config.firmwareId);
    }

    if (config.deviceProfileId != 0) {
        payload += ",\"deviceProfileId\":" + String(config.deviceProfileId);
    }

    payload += "}";

    Serial.println("Calling http post with payload: " + payload);

    // Call the init endpoint
    OtamHttpResponse response = OtamHttp::post(initUrl, payload);

    Serial.println("Received response from server");

    if (response.httpCode == 200) {
        // Device found in OTAM DB
        Serial.println("Device GUID returned from OTAM server: " + response.payload);
        // Set the device guid
        deviceGuid = response.payload;
        // Write the device guid to the store
        writeIdToStore(response.payload);
        // Log success
        Serial.println("Device has been initialized with OTAM server");
    } else {
        Serial.println("Error Status code: " + response.httpCode);
        Serial.println("Error Payload: " + response.payload);
    }
}

OtamDevice::OtamDevice(OtamConfig config) {
    // Initialize device with OTAM server
    initialize(config);

    // Set the device log URL
    deviceLogUrl = config.url + "/devices/" + deviceGuid + "/log";

    // Set the has update URL
    hasUpdateUrl = config.url + "/ota/has-update/device/get-first";
}