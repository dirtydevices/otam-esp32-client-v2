#include "OtamClient.h"

// Subscribe to the OTA download progress callback
void OtamClient::onOtaDownloadProgress(StringCalbackType progressCallback) {
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

// Set the log level
void OtamClient::setLogLevel(String logLevel) {
    // Default value if nothing passed in
    if (logLevel.isEmpty()) {
        logLevel = "LOG_LEVEL_INFO";
    }

    if (logLevel == "LOG_LEVEL_SILLY") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_SILLY");
        Otam::Logger::setLogLevel(Otam::LogLevel::SILLY);
    } else if (logLevel == "LOG_LEVEL_DEBUG") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_DEBUG");
        Otam::Logger::setLogLevel(Otam::LogLevel::DEBUG);
    } else if (logLevel == "LOG_LEVEL_VERBOSE") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_VERBOSE");
        Otam::Logger::setLogLevel(Otam::LogLevel::VERBOSE);
    } else if (logLevel == "LOG_LEVEL_HTTP") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_HTTP");
        Otam::Logger::setLogLevel(Otam::LogLevel::HTTP);
    } else if (logLevel == "LOG_LEVEL_INFO") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_INFO");
        Otam::Logger::setLogLevel(Otam::LogLevel::INFO);
    } else if (logLevel == "LOG_LEVEL_WARN") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_WARN");
        Otam::Logger::setLogLevel(Otam::LogLevel::WARN);
    } else if (logLevel == "LOG_LEVEL_ERROR") {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_ERROR");
        Otam::Logger::setLogLevel(Otam::LogLevel::ERROR);
    } else {
        Serial.println("OTAM: Setting log level to: LOG_LEVEL_NONE");
        Otam::Logger::setLogLevel(Otam::LogLevel::NONE);
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
        Otam::Logger::debug("OTA update error sent to server");
    } else {
        Otam::Logger::error("OTA update error failed to send to server");
    }
}

OtamClient::OtamClient(const OtamConfig& config) {
    Serial.println("");
    clientOtamConfig = config;
    OtamHttp::apiKey = config.apiKey;
    setLogLevel(config.logLevel);
    Otam::Logger::info("OTAM client loaded");
    FirmwareUpdateValues firmwareValuesInStore = readFirmwareValuesFromStore();
    Otam::Logger::verbose("Firmware file ID in store: " + String(firmwareValuesInStore.firmwareFileId));

    // Removing this since if uploaded by pc after an ota update
    // these details will be wrong and could be confusing
    // if (firmwareValuesInStore.firmwareFileId != 0) {
    //     Serial.println("");
    //     Otam::Logger::info("OTAM Firmware Details");
    //     Otam::Logger::info("---------------------");
    //     Otam::Logger::info("Firmware name: " + firmwareValuesInStore.firmwareName);
    //     Otam::Logger::info("Fimrware version: " + firmwareValuesInStore.firmwareVersion);
    //     Serial.println("");
    // }
}

// Initialize the OTAM client
void OtamClient::initialize() {
    Otam::Logger::verbose("Initializing OTAM client");

    try {
        // Create the device
        otamDevice = new OtamDevice(clientOtamConfig);
        deviceInitialized = true;
    } catch (const std::runtime_error& e) {
        Otam::Logger::error("An OTAM error occurred: " + String(e.what()));
        throw std::runtime_error("Could not initalize OTAM Device");
    }

    // If firmware update status success, publish to success callback
    String firmwareUpdateStatus = OtamStore::readFirmwareUpdateStatusFromStore();
    Otam::Logger::debug("OtamClient Contrcutor: Store -> Firmware update status: " + firmwareUpdateStatus);
    if (firmwareUpdateStatus == "UPDATE_SUCCESS") {
        Otam::Logger::verbose(
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
        Otam::Logger::verbose("Device message logged successfully: " + message);
    } else {
        throw std::runtime_error("Device message logging failed");
    }

    return response;
}

// Check if a firmware update is available
boolean OtamClient::hasPendingUpdate() {
    if (!deviceInitialized) {
        try {
            Otam::Logger::verbose("hasPendingUpdate() called but OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            Otam::Logger::error("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    if (updateStarted) {
        Otam::Logger::verbose("hasPendingUpdate() called, Firmware update already started");
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
        Otam::Logger::debug("Firmware update available");
        firmwareUpdateValues.firmwareFileId = OtamUtils::getJSONValue(parsed, "firmwareFileId").toInt();
        firmwareUpdateValues.firmwareId = OtamUtils::getJSONValue(parsed, "firmwareId").toInt();
        firmwareUpdateValues.firmwareName = OtamUtils::getJSONValue(parsed, "firmwareName");
        firmwareUpdateValues.firmwareVersion = OtamUtils::getJSONValue(parsed, "firmwareVersion");
        Otam::Logger::debug("Firmware update file ID: " + String(firmwareUpdateValues.firmwareFileId));
        Otam::Logger::debug("Firmware update ID: " + String(firmwareUpdateValues.firmwareId));
        Otam::Logger::debug("Firmware update name: " + firmwareUpdateValues.firmwareName);
        Otam::Logger::debug("Firmware update version: " + firmwareUpdateValues.firmwareVersion);
        delete parsed;
        return true;
    } else {
        Otam::Logger::debug("No firmware update available");
        delete parsed;
        return false;
    }
}

// Perform the firmware update
void OtamClient::doFirmwareUpdate() {
    if (!otamDevice) {
        try {
            Otam::Logger::verbose("doFirmwareUpdate() called, OTAM client not initialized");
            initialize();
        } catch (const std::runtime_error& e) {
            Otam::Logger::error("An OTAM error occurred: " + String(e.what()));
            throw std::runtime_error("Could not initalize OTAM Client with server");
        }
    }

    updateStarted = true;

    Otam::Logger::info("Firmware update started");

    HTTPClient http;

    Otam::Logger::verbose("Getting device firmware file url from: " + otamDevice->deviceFirmwareFileUrl);

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

    Otam::Logger::verbose("Downloading firmware file bin from: " + response.payload);

    http.begin(response.payload);
    http.addHeader("x-api-key", clientOtamConfig.apiKey);

    Otam::Logger::verbose("HTTP GET: " + response.payload);

    int httpCode = http.GET();

    Otam::Logger::verbose("HTTP GET response code: " + String(httpCode));

    if (httpCode == HTTP_CODE_OK) {
        // Status 200 : Download available

        Otam::Logger::info("New firmware available");

        OtamUpdater* otamUpdater = new OtamUpdater();

        otamUpdater->onOtaSuccess([this]() {
            Otam::Logger::info("OTA success callback called");

            Otam::Logger::verbose("Notifying OTAM server of successful update");

            // Create the payload with status and file ID
            String payload = "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" +
                             String(firmwareUpdateValues.firmwareFileId) +
                             ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) +
                             ",\"firmwareVersion\":\"" + firmwareUpdateValues.firmwareVersion + "\"}";

            // Update device on the server
            OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);

            // Store the updated firmware file id
            OtamStore::writeFirmwareUpdateFileIdToStore(firmwareUpdateValues.firmwareFileId);
            Otam::Logger::verbose("Firmware update file ID stored: " +
                                  String(firmwareUpdateValues.firmwareFileId));

            // Store the updated firmware id
            OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateValues.firmwareId);
            Otam::Logger::verbose("Firmware update ID stored: " + String(firmwareUpdateValues.firmwareId));

            // Store the updated firmware name
            OtamStore::writeFirmwareUpdateNameToStore(firmwareUpdateValues.firmwareName);
            Otam::Logger::verbose("Firmware update name stored: " + firmwareUpdateValues.firmwareName);

            // Store the updated firmware version
            OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateValues.firmwareVersion);
            Otam::Logger::verbose("Firmware update version stored: " + firmwareUpdateValues.firmwareVersion);

            // Store firmware update status
            OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
            Otam::Logger::verbose("Firmware update status stored: UPDATE_SUCCESS");

            // Restart the device
            ESP.restart();
        });

        otamUpdater->onOtaError([this](String error) {
            Otam::Logger::debug("OTA error callback called");
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