#include "OtamClient.h"

// Subscribe to the OTA download progress callback
void OtamClient::onOtaDownloadProgress(NumberCallbackType progressCallback) {
    otaDownloadProgressCallback = progressCallback;
}

// Subscribe to the OTA before download callback
void OtamClient::onOtaBeforeDownload(EmptyCallbackType beforeDownloadCallback) {
    otaBeforeDownloadCallback = beforeDownloadCallback;
}

// Subscribe to the OTA after download callback
void OtamClient::onOtaAfterDownload(EmptyCallbackType afterDownloadCallback) {
    otaAfterDownloadCallback = afterDownloadCallback;
}

// Subscribe to the OTA before reboot callback
void OtamClient::onOtaBeforeReboot(EmptyCallbackType beforeRebootCallback) {
    otaBeforeRebootCallback = beforeRebootCallback;
}

// Subscribe to the OTA success callback
void OtamClient::onOtaSuccess(EmptyCallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

// Subscribe to the OTA error callback
void OtamClient::onOtaError(ErrorCallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

void OtamClient::sendOtaUpdateError(String message) {
    Serial.println("Sending OTA update error: " + message);
    String logMessage = "Update failed - Firmware ID: " + String(firmwareUpdateValues.firmwareId) +
                        " - Version: " + firmwareUpdateValues.firmwareVersion + " - Error: " + message;
    OtamHttp::post(otamDevice->deviceStatusUrl,
                   "{\"deviceStatus\":\"UPDATE_FAILED\", \"logMessage\":\"" + logMessage + "\"}");
}

OtamClient::OtamClient(const OtamConfig& config) {
    clientOtamConfig = config;
    OtamHttp::apiKey = config.apiKey;
}

// Check if the device has been initialized
bool OtamClient::isInitialized() {
    return deviceInitialized;
}

// Initialize the OTAM client
void OtamClient::initialize() {
    if (!deviceInitialized) {
        // Serial.println("Initializing OTAM client");

        // Create the device
        otamDevice = new OtamDevice(clientOtamConfig);
        deviceInitialized = true;

        // If firmware update status success, publish success callback
        String firmwareUpdateStatus = OtamStore::readFirmwareUpdateStatusFromStore();
        // Serial.println("OtamClient Contrcutor: Store -> Firmware update status: " + firmwareUpdateStatus);
        if (firmwareUpdateStatus == "DEVICE_FLASHHED") {
            // Clear the firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("NONE");

            // Check if the callback has been set
            if (otaSuccessCallback) {
                // Call the callback with parameters
                otaSuccessCallback();
            }
        }
    }
}

// Log a message to the device log api
OtamHttpResponse OtamClient::logDeviceMessage(String message) {
    // Send the log entry
    OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, "{\"message\":\"" + message + "\"}");

    // Return the response
    return response;
}

// Check if a firmware update is available
const char* OtamClient::hasFirmwareUpdate() {
    OtamHttpResponse response = OtamHttp::post(
        otamDevice->hasUpdateUrl, "{\"deviceId\":\"" + clientOtamConfig.deviceId +
                                      "\",\"firmwareId\":" + clientOtamConfig.firmwareId +
                                      ",\"firmwareVersion\":\"" + clientOtamConfig.firmwareVersion + "\"}");

    if (response.httpCode == 200 && response.payload.length() < sizeof(hasUpdateCache)) {
        strncpy(hasUpdateCache, response.payload.c_str(), sizeof(hasUpdateCache) - 1);
        hasUpdateCache[sizeof(hasUpdateCache) - 1] = '\0';  // Ensure null-termination

        // If the response is not empty, prepend the clientOtamConfig.url
        if (strlen(hasUpdateCache) > 0) {
            String updatedResponse = clientOtamConfig.url + String(hasUpdateCache);
            strncpy(hasUpdateCache, updatedResponse.c_str(), sizeof(hasUpdateCache) - 1);
            hasUpdateCache[sizeof(hasUpdateCache) - 1] = '\0';
        }

        return hasUpdateCache;
    } else {
        Serial.println("Error");
        hasUpdateCache[0] = '\0';  // Indicate no update or error
        return nullptr;
    }
}

void OtamClient::doFirmwareUpdateWithFileId(int firmwareFileId) {
    // Build download url with firmwareFileId
    // ota/download/firmwareFileId
    String downloadUrlStr = clientOtamConfig.url + "/ota/download/" + String(firmwareFileId);
    const char* downloadUrl = downloadUrlStr.c_str();

    Serial.println("Downloading firmware from: " + downloadUrlStr);

    doFirmwareUpdateWithUrl(downloadUrl);
}

// Perform the firmware update
void OtamClient::doFirmwareUpdateWithUrl(const char* downloadUrl) {
    if (!otamDevice) {
        initialize();
    }

    Serial.println("Starting firmware update with URL: " + String(downloadUrl));

    updateStarted = true;

    String logMessage = "Starting firmware update.";

    OtamHttp::post(otamDevice->deviceStatusUrl,
                   "{\"deviceStatus\":\"UPDATE_STARTED\",\"logMessage\":\"" + logMessage + "\"}");

    HTTPClient http;

    http.begin(String(downloadUrl));
    http.addHeader("x-api-key", clientOtamConfig.apiKey);

    // Publish to the before download callback
    if (otaBeforeDownloadCallback) {
        otaBeforeDownloadCallback();
    }

    // Start the download
    int httpCode = http.GET();

    // Serial.println("HTTP GET response code: " + String(httpCode));

    if (httpCode == HTTP_CODE_OK) {
        OtamUpdater* otamUpdater = new OtamUpdater();

        // Subscribe to the OTA download progress callback
        otamUpdater->onOtaDownloadProgress([this](int progress) {
            if (otaDownloadProgressCallback) {
                otaDownloadProgressCallback(progress);
            }
        });

        // Subscribe to the OTA after download callback
        otamUpdater->onOtaAfterDownload([this]() {
            if (otaAfterDownloadCallback) {
                otaAfterDownloadCallback();
            }
        });

        otamUpdater->onOtaSuccess([this]() {
            Serial.println("OTAM: OTA success callback called");

            String logMessage = "Device has been flashed.";

            OtamHttp::post(otamDevice->deviceStatusUrl,
                           "{\"deviceStatus\":\"DEVICE_FLASHED\",\"logMessage\":\"" + logMessage + "\"}");

            // Store firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("DEVICE_FLASHED");

            // Publish to the on before reboot callback
            if (otaBeforeRebootCallback) {
                otaBeforeRebootCallback();
            }

            Serial.println("OTAM: Rebooting device");

            // Restart the device
            // ESP.restart();
            esp_deep_sleep_start();
        });

        otamUpdater->onOtaError([this](String error) {
            // Serial.println("OTA error callback called");
            updateStarted = false;
            if (otaErrorCallback) {
                otaErrorCallback(error);
            }
            sendOtaUpdateError(error);
        });
        otamUpdater->runESP32Update(http);
    } else {
        String error = "Firmware download failed, error: " + String(httpCode);
        updateStarted = false;
        if (otaErrorCallback) {
            otaErrorCallback(error);
        }
        sendOtaUpdateError(error);
    }
}