#include "OtamClient.h"

// Subscribe to the OTA success callback
void OtamClient::onOtaSuccess(CallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

// Subscribe to the OTA error callback
void OtamClient::onOtaError(ErrorCallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

// Set the log level
void OtamClient::setLogLevel(String logLevel) {
    // Default value if nothing passed in
    if (logLevel.isEmpty()) {
        logLevel = "LOG_LEVEL_INFO";
    }

    if (logLevel == "LOG_LEVEL_SILLY") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_SILLY");
        OtamLogger::setLogLevel(LOG_LEVEL_SILLY);
    } else if (logLevel == "LOG_LEVEL_DEBUG") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_DEBUG");
        OtamLogger::setLogLevel(LOG_LEVEL_DEBUG);
    } else if (logLevel == "LOG_LEVEL_VERBOSE") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_VERBOSE");
        OtamLogger::setLogLevel(LOG_LEVEL_VERBOSE);
    } else if (logLevel == "LOG_LEVEL_HTTP") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_HTTP");
        OtamLogger::setLogLevel(LOG_LEVEL_HTTP);
    } else if (logLevel == "LOG_LEVEL_INFO") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_INFO");
        OtamLogger::setLogLevel(LOG_LEVEL_INFO);
    } else if (logLevel == "LOG_LEVEL_WARN") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_WARN");
        OtamLogger::setLogLevel(LOG_LEVEL_WARN);
    } else if (logLevel == "LOG_LEVEL_ERROR") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_ERROR");
        OtamLogger::setLogLevel(LOG_LEVEL_ERROR);
    } else {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_NONE");
        OtamLogger::setLogLevel(LOG_LEVEL_NONE);
    }
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
    String payload = "{\"deviceStatus\":\"UPDATE_FAILED\",\"firmwareFileId\":" +
                     String(firmwareUpdateValues.firmwareFileId) +
                     ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) +
                     ",\"firmwareVersion\":\"" + firmwareUpdateValues.firmwareVersion +
                     "\",\"logMessage\":\"" + logMessage + "\"}";

    OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);
    if (response.httpCode >= 200 && response.httpCode < 300) {
        OtamLogger::debug("OTA update error sent to server");
    } else {
        OtamLogger::error("OTA update error failed to send to server");
    }
}

OtamClient::OtamClient(const OtamConfig& config) {
    Serial.println("");
    clientOtamConfig = config;
    OtamHttp::apiKey = config.apiKey;
    setLogLevel(config.logLevel);
    OtamLogger::info("OTAM client loaded");
    FirmwareUpdateValues firmwareValuesInStore = readFirmwareValuesFromStore();
    OtamLogger::verbose("Firmware file ID in store: " + String(firmwareValuesInStore.firmwareFileId));

    // Removing this since if uploaded by pc after an ota update
    // these details will be wrong and could be confusing
    // if (firmwareValuesInStore.firmwareFileId != 0) {
    //     Serial.println("");
    //     OtamLogger::info("OTAM Firmware Details");
    //     OtamLogger::info("---------------------");
    //     OtamLogger::info("Firmware name: " + firmwareValuesInStore.firmwareName);
    //     OtamLogger::info("Fimrware version: " + firmwareValuesInStore.firmwareVersion);
    //     Serial.println("");
    // }
}

// Initialize the OTAM client
void OtamClient::initialize() {
    OtamLogger::verbose("Initializing OTAM client");

    try {
        // Create the device
        otamDevice = new OtamDevice(clientOtamConfig);
        deviceInitialized = true;
    } catch (const std::runtime_error& e) {
        OtamLogger::error("An OTAM error occurred: " + String(e.what()));
        throw std::runtime_error("Could not initalize OTAM Device");
    }

    // If firmware update status success, publish to success callback
    String firmwareUpdateStatus = OtamStore::readFirmwareUpdateStatusFromStore();
    OtamLogger::debug("OtamClient Contrcutor: Store -> Firmware update status: " + firmwareUpdateStatus);
    if (firmwareUpdateStatus == "UPDATE_SUCCESS") {
        OtamLogger::verbose(
            "Firmware update status is UPDATE_SUCCESS, calling OTA success "
            "callback");
        FirmwareUpdateValues firmwareUpdateSuccessValues = readFirmwareValuesFromStore();

        // Clear the firmware update status
        OtamStore::writeFirmwareUpdateStatusToStore("NONE");

        // Check if the callback has been set
        if (otaSuccessCallback) {
            // Call the callback with parameters
            otaSuccessCallback(firmwareUpdateSuccessValues);
        }
    }
}

// Log a message to the device log api
OtamHttpResponse OtamClient::logDeviceMessage(String message) {
    // Create the payload
    String payload = "{\"message\":\"" + message + "\"}";

    // Send the log entry
    OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, payload);

    if (response.httpCode >= 200 && response.httpCode < 300) {
        OtamLogger::verbose("Device message logged successfully: " + message);
    } else {
        throw std::runtime_error("Device message logging failed");
    }

    return response;
}

// Check if a firmware update is available
boolean OtamClient::hasPendingUpdate() {
    if (!deviceInitialized) {
        try {
            OtamLogger::verbose("hasPendingUpdate() called but OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            OtamLogger::error("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    if (updateStarted) {
        OtamLogger::verbose("hasPendingUpdate() called, Firmware update already started");
        return false;
    }

    // Get the device status from the server
    OtamHttpResponse response = OtamHttp::get(otamDevice->deviceStatusUrl);

    // Parse the response
    cJSON* parsed = OtamUtils::parseJSON(response.payload);

    // Get the device status from the response
    String deviceStatus = OtamUtils::getJSONValue(parsed, "deviceStatus");

    // Check if the device status is UPDATE_PENDING
    if (deviceStatus == "UPDATE_PENDING") {
        OtamLogger::debug("Firmware update available");
        firmwareUpdateValues.firmwareFileId = OtamUtils::getJSONValue(parsed, "firmwareFileId").toInt();
        firmwareUpdateValues.firmwareId = OtamUtils::getJSONValue(parsed, "firmwareId").toInt();
        firmwareUpdateValues.firmwareName = OtamUtils::getJSONValue(parsed, "firmwareName");
        firmwareUpdateValues.firmwareVersion = OtamUtils::getJSONValue(parsed, "firmwareVersion");
        OtamLogger::debug("Firmware update file ID: " + String(firmwareUpdateValues.firmwareFileId));
        OtamLogger::debug("Firmware update ID: " + String(firmwareUpdateValues.firmwareId));
        OtamLogger::debug("Firmware update name: " + firmwareUpdateValues.firmwareName);
        OtamLogger::debug("Firmware update version: " + firmwareUpdateValues.firmwareVersion);
        delete parsed;
        return true;
    } else {
        OtamLogger::debug("No firmware update available");
        delete parsed;
        return false;
    }
}

// Perform the firmware update
void OtamClient::doFirmwareUpdate() {
    if (!otamDevice) {
        try {
            OtamLogger::verbose("doFirmwareUpdate() called, OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            OtamLogger::error("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    updateStarted = true;

    OtamLogger::info("Firmware update started");

    HTTPClient http;

    OtamLogger::verbose("Getting device firmware file url from: " + otamDevice->deviceFirmwareFileUrl);

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

    OtamLogger::verbose("Downloading firmware file bin from: " + response.payload);

    http.begin(response.payload);
    http.addHeader("x-api-key", clientOtamConfig.apiKey);

    OtamLogger::verbose("HTTP GET: " + response.payload);

    int httpCode = http.GET();

    OtamLogger::verbose("HTTP GET response code: " + String(httpCode));

    if (httpCode == HTTP_CODE_NO_CONTENT) {
        // Status 204: Firmware already up to date

        updateStarted = false;
        OtamLogger::info("No new firmware available");
        return;
    } else if (httpCode == HTTP_CODE_OK) {
        // Status 200: Download available

        OtamLogger::info("New firmware available");

        OtamUpdater* otamUpdater = new OtamUpdater();

        otamUpdater->onOtaSuccess([this]() {
            OtamLogger::info("OTA success callback called");

            OtamLogger::verbose("Notifying OTAM server of successful update");

            // Create the payload with status and file ID
            String payload = "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" +
                             String(firmwareUpdateValues.firmwareFileId) +
                             ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) +
                             ",\"firmwareVersion\":\"" + firmwareUpdateValues.firmwareVersion + "\"}";

            // Update device on the server
            OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);

            // Store the updated firmware file id
            OtamStore::writeFirmwareUpdateFileIdToStore(firmwareUpdateValues.firmwareFileId);
            OtamLogger::verbose("Firmware update file ID stored: " +
                                String(firmwareUpdateValues.firmwareFileId));

            // Store the updated firmware id
            OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateValues.firmwareId);
            OtamLogger::verbose("Firmware update ID stored: " + String(firmwareUpdateValues.firmwareId));

            // Store the updated firmware name
            OtamStore::writeFirmwareUpdateNameToStore(firmwareUpdateValues.firmwareName);
            OtamLogger::verbose("Firmware update name stored: " + firmwareUpdateValues.firmwareName);

            // Store the updated firmware version
            OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateValues.firmwareVersion);
            OtamLogger::verbose("Firmware update version stored: " + firmwareUpdateValues.firmwareVersion);

            // Store firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
            OtamLogger::verbose("Firmware update status stored: UPDATE_SUCCESS");

            // Restart the device
            ESP.restart();
        });

        otamUpdater->onOtaError([this](String error) {
            OtamLogger::debug("OTA error callback called");
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