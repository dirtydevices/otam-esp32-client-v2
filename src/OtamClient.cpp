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

void OtamClient::sendOtaUpdateError(String logMessage) {
    Serial.println("Sending OTA update error: " + logMessage);
    // OtamHttp::post(otamDevice->deviceStatusUrl,
    //                "{\"deviceStatus\":\"UPDATE_FAILED\",\"firmwareFileId\":" +
    //                    String(firmwareUpdateValues.firmwareFileId) + ",\"firmwareId\":" +
    //                    String(firmwareUpdateValues.firmwareId) + ",\"firmwareVersion\":\"" +
    //                    firmwareUpdateValues.firmwareVersion + "\",\"logMessage\":\"" + logMessage + "\"}");
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
        if (firmwareUpdateStatus == "UPDATE_SUCCESS") {
            // Serial.println(
            //     "Firmware update status is UPDATE_SUCCESS, calling OTA success "
            //     "callback");

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
        hasUpdateCache[0] = '\0';  // Indicate no update or error
        return nullptr;
    }
}

// Perform the firmware update
void OtamClient::doFirmwareUpdate(const char* downloadUrl) {
    if (!otamDevice) {
        initialize();
    }

    updateStarted = true;

    // Serial.println("Firmware update started");

    HTTPClient http;

    // Serial.println("Getting device firmware file url from: " + otamDevice->deviceFirmwareFileUrl);

    // Removed the call that fetched the firmware file URL

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
        // Status 200 : Download available

        // Serial.println("New firmware available");

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

            // Serial.println("Notifying OTAM server of successful update");

            // Serial.println("OTAM: Updating device status on server with the following values:");
            // Serial.println("POST Url: " + otamDevice->deviceStatusUrl);
            // Serial.println("Firmware file ID: " + String(firmwareUpdateValues.firmwareFileId));
            // Serial.println("Firmware ID: " + String(firmwareUpdateValues.firmwareId));
            // Serial.println("Firmware name: " + firmwareUpdateValues.firmwareName);
            // Serial.println("Firmware version: " + firmwareUpdateValues.firmwareVersion);

            // Update device on the server
            // OtamHttpResponse response =
            //     OtamHttp::post(otamDevice->deviceStatusUrl,
            //                    "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" +
            //                        String(firmwareUpdateValues.firmwareFileId) +
            //                        ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) +
            //                        ",\"firmwareVersion\":\"" + firmwareUpdateValues.firmwareVersion + "\"}");

            // Store the updated firmware id
            // OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateValues.firmwareId);
            // Serial.println("Firmware update ID stored: " + String(firmwareUpdateValues.firmwareId));

            // Store the updated firmware version
            // OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateValues.firmwareVersion);
            // Serial.println("Firmware update version stored: " + firmwareUpdateValues.firmwareVersion);

            // Store firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
            // Serial.println("Firmware update status stored: UPDATE_SUCCESS");

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