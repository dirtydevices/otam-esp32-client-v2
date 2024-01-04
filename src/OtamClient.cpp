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
void OtamClient::onOtaSuccess(SuccessCallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

// Subscribe to the OTA error callback
void OtamClient::onOtaError(ErrorCallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

// Read firmware values from store
FirmwareUpdateValues OtamClient::readFirmwareValuesFromStore() {
    FirmwareUpdateValues firmwareValuesInStore;
    firmwareValuesInStore.firmwareFileId = OtamStore::readFirmwareUpdateFileIdFromStore();
    firmwareValuesInStore.firmwareId = OtamStore::readFirmwareUpdateIdFromStore();
    firmwareValuesInStore.firmwareName = OtamStore::readFirmwareUpdateNameFromStore();
    firmwareValuesInStore.firmwareVersion = OtamStore::readFirmwareUpdateVersionFromStore();
    return firmwareValuesInStore;
}

void OtamClient::sendOtaUpdateError(String logMessage) {
    OtamHttp::post(otamDevice->deviceStatusUrl,
                   "{\"deviceStatus\":\"UPDATE_FAILED\",\"firmwareFileId\":" +
                       String(firmwareUpdateValues.firmwareFileId) + ",\"firmwareId\":" +
                       String(firmwareUpdateValues.firmwareId) + ",\"firmwareVersion\":\"" +
                       firmwareUpdateValues.firmwareVersion + "\",\"logMessage\":\"" + logMessage + "\"}");

    // OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);
    // if (response.httpCode >= 200 && response.httpCode < 300) {
    //     Serial.println("OTA update error sent to server");
    // } else {
    //     Serial.println("OTA update error failed to send to server");
    // }
}

OtamClient::OtamClient(const OtamConfig& config) {
    clientOtamConfig = config;
    OtamHttp::apiKey = config.apiKey;
    FirmwareUpdateValues firmwareValuesInStore = readFirmwareValuesFromStore();
}

// Check if the device has been initialized
bool OtamClient::isInitialized() {
    return deviceInitialized;
}

// Initialize the OTAM client
void OtamClient::initialize() {
    if (!deviceInitialized) {
        // Serial.println("Initializing OTAM client");
        try {
            // Create the device
            otamDevice = new OtamDevice(clientOtamConfig);
            deviceInitialized = true;

            // If firmware update status success, publish to success callback
            String firmwareUpdateStatus = OtamStore::readFirmwareUpdateStatusFromStore();
            // Serial.println("OtamClient Contrcutor: Store -> Firmware update status: " + firmwareUpdateStatus);
            if (firmwareUpdateStatus == "UPDATE_SUCCESS") {
                // Serial.println(
                //     "Firmware update status is UPDATE_SUCCESS, calling OTA success "
                //     "callback");
                FirmwareUpdateValues firmwareUpdateSuccessValues = readFirmwareValuesFromStore();

                // Clear the firmware update status
                OtamStore::writeFirmwareUpdateStatusToStore("NONE");

                // Check if the callback has been set
                if (otaSuccessCallback) {
                    // Call the callback with parameters
                    otaSuccessCallback(firmwareUpdateSuccessValues);
                }
            }
        } catch (const std::runtime_error& e) {
            // Serial.println("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Device");
        }
    }
}

// Log a message to the device log api
OtamHttpResponse OtamClient::logDeviceMessage(String message) {
    // Send the log entry
    OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, "{\"message\":\"" + message + "\"}");

    return response;
}

// Check if a firmware update is available
boolean OtamClient::hasPendingUpdate() {
    if (!deviceInitialized) {
        try {
            // Serial.println("hasPendingUpdate() called but OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            // Serial.println("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    if (!updateStarted) {
        // Get the device status from the server
        OtamHttpResponse response = OtamHttp::get(otamDevice->deviceStatusUrl);

        // Parse the response
        cJSON* parsed = OtamUtils::parseJSON(response.payload);

        // Get the device status from the response
        String deviceStatus = OtamUtils::getJSONValue(parsed, "deviceStatus");

        // Check if the device status is UPDATE_PENDING
        if (deviceStatus == "UPDATE_PENDING") {
            // Serial.println("Firmware update available");
            firmwareUpdateValues.firmwareFileId = OtamUtils::getJSONValue(parsed, "firmwareFileId").toInt();
            firmwareUpdateValues.firmwareId = OtamUtils::getJSONValue(parsed, "firmwareId").toInt();
            firmwareUpdateValues.firmwareName = OtamUtils::getJSONValue(parsed, "firmwareName");
            firmwareUpdateValues.firmwareVersion = OtamUtils::getJSONValue(parsed, "firmwareVersion");
            // Serial.println("Firmware update file ID: " + String(firmwareUpdateValues.firmwareFileId));
            // Serial.println("Firmware update ID: " + String(firmwareUpdateValues.firmwareId));
            // Serial.println("Firmware update name: " + firmwareUpdateValues.firmwareName);
            // Serial.println("Firmware update version: " + firmwareUpdateValues.firmwareVersion);
            delete parsed;
            return true;
        } else {
            // Serial.println("No firmware update available");
            delete parsed;
            return false;
        }
    }
}

// Perform the firmware update
void OtamClient::doFirmwareUpdate() {
    if (!otamDevice) {
        try {
            // Serial.println("doFirmwareUpdate() called, OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            // Serial.println("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    updateStarted = true;

    // Serial.println("Firmware update started");

    HTTPClient http;

    // Serial.println("Getting device firmware file url from: " + otamDevice->deviceFirmwareFileUrl);

    // Get the device status from the server
    OtamHttpResponse response = OtamHttp::get(otamDevice->deviceFirmwareFileUrl);

    if (response.httpCode != 200 || response.payload == "") {
        String error = "Firmware file url request failed, error: " + String(response.httpCode);
        updateStarted = false;
        if (otaErrorCallback) {
            otaErrorCallback(firmwareUpdateValues, error);
        }
        sendOtaUpdateError(error);
        return;
    }

    // Serial.println("Downloading firmware file bin from: " + response.payload);

    http.begin(response.payload);
    http.addHeader("x-api-key", clientOtamConfig.apiKey);

    // Serial.println("HTTP GET: " + response.payload);

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
            // Serial.println("OTA success callback called");

            // Serial.println("Notifying OTAM server of successful update");

            // Update device on the server
            OtamHttpResponse response =
                OtamHttp::post(otamDevice->deviceStatusUrl,
                               "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" +
                                   String(firmwareUpdateValues.firmwareFileId) +
                                   ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) +
                                   ",\"firmwareVersion\":\"" + firmwareUpdateValues.firmwareVersion + "\"}");

            // Store the updated firmware file id
            OtamStore::writeFirmwareUpdateFileIdToStore(firmwareUpdateValues.firmwareFileId);
            // Serial.println("Firmware update file ID stored: " +
            //                       String(firmwareUpdateValues.firmwareFileId));

            // Store the updated firmware id
            OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateValues.firmwareId);
            // Serial.println("Firmware update ID stored: " + String(firmwareUpdateValues.firmwareId));

            // Store the updated firmware name
            OtamStore::writeFirmwareUpdateNameToStore(firmwareUpdateValues.firmwareName);
            // Serial.println("Firmware update name stored: " + firmwareUpdateValues.firmwareName);

            // Store the updated firmware version
            OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateValues.firmwareVersion);
            // Serial.println("Firmware update version stored: " + firmwareUpdateValues.firmwareVersion);

            // Store firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
            // Serial.println("Firmware update status stored: UPDATE_SUCCESS");

            // Publish to the on before reboot callback
            if (otaBeforeRebootCallback) {
                otaBeforeRebootCallback();
            }

            // Restart the device
            ESP.restart();
        });

        otamUpdater->onOtaError([this](String error) {
            // Serial.println("OTA error callback called");
            updateStarted = false;
            if (otaErrorCallback) {
                otaErrorCallback(firmwareUpdateValues, error);
            }
            sendOtaUpdateError(error);
        });
        otamUpdater->runESP32Update(http);
    } else {
        String error = "Firmware download failed, error: " + String(httpCode);
        updateStarted = false;
        if (otaErrorCallback) {
            otaErrorCallback(firmwareUpdateValues, error);
        }
        sendOtaUpdateError(error);
    }
}