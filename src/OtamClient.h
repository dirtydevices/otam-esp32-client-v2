#include <HttpClient.h>
#include "internal/OtamConfig.h"
#include "internal/OtamLogger.h"
#include "internal/OtamHttp.h"
#include "internal/OtamDevice.h"

class OtamClient
{
private:
    OtamDevice *otamDevice;
    bool updateStarted = false;
    int firmwareUpdateFileId;
    int firmwareUpdateId;
    String firmwareUpdateName = "";
    String firmwareUpdateVersion = "";

public:
    // Define the type for the callback functions
    using CallbackType = std::function<void(int, int, String, String)>;

    // Variables to hold the callback functions
    CallbackType otaSuccessCallback;
    CallbackType otaErrorCallback;

    // Subscribe to the OTA success callback
    void onOtaSuccess(CallbackType successCallback)
    {
        otaSuccessCallback = successCallback;
    }

    // Subscribe to the OTA error callback
    void onOtaError(CallbackType errorCallback)
    {
        otaErrorCallback = errorCallback;
    }

    // Constructor
    OtamClient()
    {
        // If firmware update status success, publish to success callback
        if (OtamStore::readFirmwareUpdateStatusFromStore() == "UPDATE_SUCCESS")
        {
            OtamLogger::debug("Firmware update status is UPDATE_SUCCESS, calling OTA success callback");
            int firmwareFileId = OtamStore::readFirmwareUpdateFileIdFromStore();
            int firmwareId = OtamStore::readFirmwareUpdateIdFromStore();
            String firmwareName = OtamStore::readFirmwareUpdateNameFromStore();
            String firmwareVersion = OtamStore::readFirmwareUpdateVersionFromStore();
            if (otaSuccessCallback) // Check if the callback has been set
            {
                otaSuccessCallback(firmwareFileId, firmwareId, firmwareName, firmwareVersion); // Call the callback with parameters
            }
        }
    }

    // Set the log level
    void setLogLevel(OtamLogLevel logLevel)
    {
        Serial.println("Setting log level to: " + String(logLevel));
        OtamLogger::setLogLevel(logLevel);
    }

    // Initialize the OTAM client
    void initialize(OtamConfig config)
    {
        OtamLogger::debug("Initializing OTAM client");

        // Create the device
        otamDevice = new OtamDevice(config);
    }

    // Log a message to the device log api
    OtamHttpResponse logDeviceMessage(String message)
    {
        // Create the payload
        String payload = "{\"message\":\"" + message + "\"}";

        // Send the log entry
        OtamHttpResponse response = OtamHttp::post(otamDevice->deviceLogUrl, payload);

        if (response.httpCode >= 200 && response.httpCode < 300)
        {
            OtamLogger::debug("Device message logged successfully: " + message);
        }
        else
        {
            throw std::runtime_error("Device message logging failed");
        }

        return response;
    }

    // Check if a firmware update is available
    boolean hasPendingUpdate()
    {
        if (updateStarted)
        {
            OtamLogger::debug("hasPendingUpdate() called, Firmware update already started");
            return false;
        }

        OtamHttpResponse response = OtamHttp::get(otamDevice->deviceStatusUrl);
        cJSON *parsed = OtamUtils::parseJSON(response.payload);
        String deviceStatus = OtamUtils::getJSONValue(parsed, "deviceStatus");

        if (deviceStatus == "UPDATE_PENDING")
        {
            OtamLogger::verbose("Firmware update available");
            firmwareUpdateFileId = OtamUtils::getJSONValue(parsed, "firmwareFileId").toInt();
            firmwareUpdateId = OtamUtils::getJSONValue(parsed, "firmwareId").toInt();
            firmwareUpdateName = OtamUtils::getJSONValue(parsed, "firmwareName");
            firmwareUpdateVersion = OtamUtils::getJSONValue(parsed, "firmwareVersion");
            OtamLogger::verbose("Firmware update file ID: " + String(firmwareUpdateFileId));
            OtamLogger::verbose("Firmware update ID: " + String(firmwareUpdateId));
            OtamLogger::verbose("Firmware update name: " + firmwareUpdateName);
            OtamLogger::verbose("Firmware update version: " + firmwareUpdateVersion);
            delete parsed;
            return true;
        }
        else
        {
            OtamLogger::verbose("No firmware update available");
            delete parsed;
            return false;
        }
    }

    // Perform the firmware update
    void doFirmwareUpdate()
    {
        updateStarted = true;

        OtamLogger::info("Firmware update started");

        HTTPClient http;

        OtamLogger::debug("Downloading firmware from: " + otamDevice->deviceDownloadUrl);

        http.begin(otamDevice->deviceDownloadUrl);

        OtamLogger::debug("HTTP GET: " + otamDevice->deviceDownloadUrl);

        int httpCode = http.GET();

        OtamLogger::debug("HTTP GET response code: " + String(httpCode));

        if (httpCode == HTTP_CODE_NO_CONTENT) // Status 204: Firmware already up to date
        {
            updateStarted = false;
            OtamLogger::info("No new firmware available");
            return;
        }
        else if (httpCode == HTTP_CODE_OK) // Status 200: Download available
        {
            OtamLogger::info("New firmware available");
            OtamUpdater *otamUpdater = new OtamUpdater();
            otamUpdater->onOtaSuccess([this]()
                                      {
                OtamLogger::info("OTA success callback called");
                
                OtamLogger::debug("Notifying OTAM server of successful update");
                OtamLogger::debug("Firmware update file ID: " + String(firmwareUpdateFileId));
                OtamLogger::debug("Firmware update ID: " + String(firmwareUpdateId));
                OtamLogger::debug("Firmware update name: " + firmwareUpdateName);
                OtamLogger::debug("Firmware update version: " + firmwareUpdateVersion);
                
                // Create the payload with status and file ID
                String payload = "{\"deviceStatus\":\"UPDATE_SUCCESS\",\"firmwareFileId\":" + String(firmwareUpdateFileId) + ",\"firmwareId\":" + String(firmwareUpdateId) + ",\"firmwareVersion\":\"" + firmwareUpdateVersion + "\"}";
                

                // Update device on the server
                OtamHttpResponse response = OtamHttp::post(otamDevice->deviceStatusUrl, payload);

                // Store the updated firmware file id
                OtamStore::writeFirmwareUpdateFileIdToStore(firmwareUpdateFileId);
                OtamLogger::debug("Firmware update file ID stored: " + String(firmwareUpdateFileId));

                // Store the updated firmware id 
                OtamStore::writeFirmwareUpdateIdToStore(firmwareUpdateId);
                OtamLogger::debug("Firmware update ID stored: " + String(firmwareUpdateId));
                
                // Store the updated firmware name
                OtamStore::writeFirmwareUpdateNameToStore(firmwareUpdateName);
                OtamLogger::debug("Firmware update name stored: " + firmwareUpdateName);
                
                // Store the updated firmware version
                OtamStore::writeFirmwareUpdateVersionToStore(firmwareUpdateVersion);
                OtamLogger::debug("Firmware update version stored: " + firmwareUpdateVersion);


                // Store firmware update status
                OtamStore::writeFirmwareUpdateStatusToStore("UPDATE_SUCCESS");
                OtamLogger::debug("Firmware update status stored: UPDATE_SUCCESS");

                // Restart the device
                ESP.restart(); });

            otamUpdater->onOtaError([this]()
                                    {
                                        OtamLogger::error("OTA error callback called");
                                        updateStarted = false; });
            otamUpdater->runESP32Update(http);
        }
        else
        {
            updateStarted = false;
            OtamLogger::error("Firmware download failed, error: " + String(httpCode));
            throw std::runtime_error("Firmware download failed.");
        }
    }
};