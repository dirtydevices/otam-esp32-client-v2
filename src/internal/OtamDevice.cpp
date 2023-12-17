#include "internal/OtamDevice.h"

void OtamDevice::writeIdToStore(String id) {
    OtamStore::writeDeviceIdToStore(id);
    OtamLogger::debug("Device id written to store: " + id);
}

void OtamDevice::initialize(OtamConfig config) {
    // writeIdToStore("");
    OtamLogger::info("Initializing device with OTAM server");

    // Read the device id from the store
    String deviceIdStore = OtamStore::readDeviceIdFromStore();

    if (deviceIdStore != "") {
        OtamLogger::debug("Device id read from store: " + deviceIdStore);
    }

    String initUrl = config.url + "/init-device";
    String payload = "{\"deviceName\":\"" + config.deviceName + "\", \"deviceIdStore\":\"" + deviceIdStore +
                     "\", \"deviceIdConfig\":\"" + config.deviceId +
                     "\", \"generateDeviceId\":" + String(config.regenerateDeviceId) + "}";

    // Call the init endpoint
    OtamHttpResponse response = OtamHttp::post(initUrl, payload);

    if (response.httpCode == 200) {
        // Device found in OTAM DB
        deviceId = response.payload;
    } else if (response.httpCode == 201) {
        // Created
        deviceId = response.payload;
        OtamLogger::info("New device created on OTAM server: " + deviceId);
    } else {
        // Error
        OtamLogger::error("Setting device id failed with status code " + String(response.httpCode));
        throw std::runtime_error("Set device id failed");
    }

    if (deviceId != deviceIdStore) {
        writeIdToStore(deviceId);
    } else {
        OtamLogger::debug("Device id already in store, no need to write: " + deviceId);
    }

    OtamLogger::info("Device has been initialized with OTAM server");
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
    deviceDownloadUrl = this->deviceUrl + "/download";

    // Create the db logger
    logDb = new OtamLogDb(deviceLogUrl);
}