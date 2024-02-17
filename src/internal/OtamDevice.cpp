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
    // Create payload with deviceName and deviceId
    String payload =
        "{\"deviceName\": \"" + config.deviceName + "\", \"deviceIdStore\": \"" + deviceIdStore + "\"}";

    Serial.println("Calling http post with payload: " + payload);

    // Call the init endpoint
    OtamHttpResponse response = OtamHttp::post(initUrl, payload);

    Serial.println("Received response from server");

    if (response.httpCode == 200) {
        // Device found in OTAM DB
        Serial.println("Device Id returned from OTAM server: " + response.payload);
        // Set the device id
        deviceId = response.payload;
        // Write the device id to the store
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

    // Set the device URL
    deviceUrl = config.url + "/devices/" + deviceId;

    // Set the device log URL
    deviceLogUrl = this->deviceUrl + "/log";

    // Set the device status URL
    deviceStatusUrl = this->deviceUrl + "/status";

    // Set the device download URL
    deviceFirmwareFileUrl = this->deviceUrl + "/firmware-file-url";
}