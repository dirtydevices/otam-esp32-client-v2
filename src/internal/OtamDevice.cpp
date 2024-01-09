#include "internal/OtamDevice.h"

void OtamDevice::writeIdToStore(String id) {
    OtamStore::writeDeviceIdToStore(id);
    // Serial.println("Device id written to store: " + id);
}

void OtamDevice::initialize(OtamConfig config) {
    // writeIdToStore("");
    Serial.println("Initializing device with OTAM server");

    // Read the device id from the store
    String deviceIdStore = OtamStore::readDeviceIdFromStore();

    if (deviceIdStore != "") {
        Serial.println("Device id read from store: " + deviceIdStore);
    }

    String initUrl = config.url + "/init-device";
    String payload = "{\"deviceName\":\"" + config.deviceName + "\", \"deviceIdStore\":\"" + deviceIdStore +
                     "\", \"deviceIdConfig\":\"" + config.deviceId +
                     "\", \"generateDeviceId\":" + String(config.regenerateDeviceId) + "}";

    Serial.println("Calling http post with payload: " + payload);

    // Call the init endpoint
    OtamHttpResponse response = OtamHttp::post(initUrl, payload);

    Serial.println("Received response from server");

    if (response.httpCode == 200) {
        Serial.println("Status code 200");
        // Device found in OTAM DB
        deviceId = response.payload;
    } else if (response.httpCode == 201) {
        Serial.println("Status code 201");
        // Created
        deviceId = response.payload;
    } else {
        Serial.println("Error: Setting device id failed with status code " + String(response.httpCode));
        Serial.println("Error payload: " + response.payload);
        return;
    }

    if (deviceId != deviceIdStore && deviceId != "") {
        Serial.println("Device id has changed, writing to store");
        writeIdToStore(deviceId);
    }

    Serial.println("Device has been initialized with OTAM server");
}

OtamDevice::OtamDevice(OtamConfig config) {
    // Initialize device with OTAM server
    initialize(config);

    // Set the device name
    deviceName = config.deviceName;

    // Set the device URL
    deviceUrl = config.url + "/devices/" + deviceId;

    // Set the device log URL
    deviceLogUrl = this->deviceUrl + "/log";

    // Set the device status URL
    deviceStatusUrl = this->deviceUrl + "/status";

    // Set the device download URL
    deviceFirmwareFileUrl = this->deviceUrl + "/firmware-file-url";
}