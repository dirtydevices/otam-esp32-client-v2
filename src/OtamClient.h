#include <HttpClient.h>
#include "internal/OtamConfig.h"
#include "internal/OtamDevice.h"
#include "internal/OtamHttp.h"
#include "internal/OtamLogger.h"

struct FirmwareUpdateValues {
    int firmwareFileId;
    int firmwareId;
    String firmwareName;
    String firmwareVersion;
};

class OtamClient {
   private:
    OtamDevice* otamDevice;
    bool updateStarted = false;
    FirmwareUpdateValues firmwareUpdateValues;

   public:
    // Define the type for the callback functions
    using CallbackType = std::function<void(FirmwareUpdateValues)>;

    // Variables to hold the callback functions
    CallbackType otaSuccessCallback;
    CallbackType otaErrorCallback;

    // Subscribe to the OTA success callback
    void onOtaSuccess(CallbackType successCallback) { otaSuccessCallback = successCallback; }

    // Subscribe to the OTA error callback
    void onOtaError(CallbackType errorCallback) { otaErrorCallback = errorCallback; }

    // Set the log level
    void setLogLevel(OtamLogLevel logLevel) {
        Serial.println("Setting log level to: " + String(logLevel));
        OtamLogger::setLogLevel(logLevel);
    }

    // Initialize the OTAM client
    void initialize(OtamConfig config) {
        OtamLogger::debug("Initializing OTAM client");

        // Create the device
        otamDevice = new OtamDevice(config);

        // If firmware update status success, publish to success callback
        String firmwareUpdateStatus = OtamStore::readFirmwareUpdateStatusFromStore();
        OtamLogger::verbose("OtamClient Contrcutor: Store -> Firmware update status: " + firmwareUpdateStatus);
        if (firmwareUpdateStatus == "UPDATE_SUCCESS") {
            OtamLogger::debug(
                "Firmware update status is UPDATE_SUCCESS, calling OTA success "
                "callback");
            FirmwareUpdateValues firmwareUpdateSuccessValues;
            firmwareUpdateSuccessValues.firmwareFileId = OtamStore::readFirmwareUpdateFileIdFromStore();
            firmwareUpdateSuccessValues.firmwareId = OtamStore::readFirmwareUpdateIdFromStore();
            firmwareUpdateSuccessValues.firmwareName = OtamStore::readFirmwareUpdateNameFromStore();
            firmwareUpdateSuccessValues.firmwareVersion = OtamStore::readFirmwareUpdateVersionFromStore();

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
    OtamHttpResponse logDeviceMessage(String message) {
        // Create the payload
        String payload = "{\"message\":\"" + message + "\"}";

        // Send the log entry
        OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, payload);

        if (response.httpCode >= 200 && response.httpCode < 300) {
            OtamLogger::debug("Device message logged successfully: " + message);
        } else {
            throw std::runtime_error("Device message logging failed");
        }

        return response;
    }

    // Check if a firmware update is available
    boolean hasPendingUpdate() {
        if (updateStarted) {
            OtamLogger::debug("hasPendingUpdate() called, Firmware update already started");
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
            OtamLogger::verbose("Firmware update available");
            firmwareUpdateValues.firmwareFileId = OtamUtils::getJSONValue(parsed, "firmwareFileId").toInt();
            firmwareUpdateValues.firmwareId = OtamUtils::getJSONValue(parsed, "firmwareId").toInt();
            firmwareUpdateValues.firmwareName = OtamUtils::getJSONValue(parsed, "firmwareName");
            firmwareUpdateValues.firmwareVersion = OtamUtils::getJSONValue(parsed, "firmwareVersion");
            OtamLogger::verbose("Firmware update file ID: " + String(firmwareUpdateValues.firmwareFileId));
            OtamLogger::verbose("Firmware update ID: " + String(firmwareUpdateValues.firmwareId));
            OtamLogger::verbose("Firmware update name: " + firmwareUpdateValues.firmwareName);
            OtamLogger::verbose("Firmware update version: " + firmwareUpdateValues.firmwareVersion);
            delete parsed;
            return true;
        } else {
            OtamLogger::verbose("No firmware update available");
            delete parsed;
            return false;
        }
    }

    // Perform the firmware update
    void doFirmwareUpdate() {
        updateStarted = true;

        OtamLogger::info("Firmware update started");

        HTTPClient http;

        OtamLogger::debug("Downloading firmware from: " + otamDevice->deviceDownloadUrl);

        http.begin(otamDevice->deviceDownloadUrl);

        OtamLogger::debug("HTTP GET: " + otamDevice->deviceDownloadUrl);

        int httpCode = http.GET();

        OtamLogger::debug("HTTP GET response code: " + String(httpCode));

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

                OtamLogger::debug("Notifying OTAM server of successful update");

                // Create the payload with status and file ID
                String payload = "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" + String(firmwareUpdateValues.firmwareFileId) +
                                 ",\"firmwareId\":" + String(firmwareUpdateValues.firmwareId) + ",\"firmwareVersion\":\"" +
                                 firmwareUpdateValues.firmwareVersion + "\"}";

                // Update device on the server
                OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);

                // Store the updated firmware file id
                OtamStore::writeFirmwareUpdateFileIdToStore(firmwareUpdateValues.firmwareFileId);
                OtamLogger::debug("Firmware update file ID stored: " + String(firmwareUpdateValues.firmwareFileId));

                // Store the updated firmware id
                OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateValues.firmwareId);
                OtamLogger::debug("Firmware update ID stored: " + String(firmwareUpdateValues.firmwareId));

                // Store the updated firmware name
                OtamStore::writeFirmwareUpdateNameToStore(firmwareUpdateValues.firmwareName);
                OtamLogger::debug("Firmware update name stored: " + firmwareUpdateValues.firmwareName);

                // Store the updated firmware version
                OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateValues.firmwareVersion);
                OtamLogger::debug("Firmware update version stored: " + firmwareUpdateValues.firmwareVersion);

                // Store firmware update status
                OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
                OtamLogger::debug("Firmware update status stored: UPDATE_SUCCESS");

                // Restart the device
                ESP.restart();
            });

            otamUpdater->onOtaError([this]() {
                OtamLogger::error("OTA error callback called");
                updateStarted = false;
            });
            otamUpdater->runESP32Update(http);
        } else {
            updateStarted = false;
            OtamLogger::error("Firmware download failed, error: " + String(httpCode));
            throw std::runtime_error("Firmware download failed.");
        }
    }
};