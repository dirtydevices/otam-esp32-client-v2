#include "internal/OtamUpdater.h"

void OtamUpdater::onOtaAfterDownload(CallbackType afterDownloadCallback) {
    otaAfterDownloadCallback = afterDownloadCallback;
}

void OtamUpdater::onOtaDownloadProgress(NumberCallbackType progressCallback) {
    otaDownloadProgressCallback = progressCallback;
}

void OtamUpdater::onOtaSuccess(CallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

void OtamUpdater::onOtaError(StringCallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

void OtamUpdater::runESP32Update(HTTPClient& http) {
    try {
        int contentLength = http.getSize();  // Get the firmware size
        // Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
        // Serial.printf("Total heap: %u\n", ESP.getHeapSize());
        // Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());
        // Serial.println("Starting OTA Update...");
        // Serial.println("Content Length: " + String(contentLength));

        // if (contentLength <= 0) {
        //     otaErrorCallback("Content length is invalid");
        //     return;
        // }

        bool canBegin = Update.begin(contentLength);  // Initialize OTA process
        if (canBegin) {
            // Serial.println("OTA Update initialized successfully.");
            WiFiClient* client = http.getStreamPtr();  // Get the client stream

            // Progress callback for logging the progress
            Update.onProgress([this](size_t progress, size_t total) {
                // Serial.printf("OTA Progress: %u of %u bytes\r\n", progress, total);
                otaDownloadProgressCallback(progress / (total / 100));  // Callback with percentage
            });

            // Write firmware data to flash
            size_t written = Update.writeStream(*client);
            // Serial.println("Bytes written to flash: " + String(written));

            // if (written != contentLength) {
            //     Serial.println("Warning: Written bytes do not match content length.");
            // }

            if (Update.end()) {
                // Download complete
                otaAfterDownloadCallback();
                if (Update.isFinished()) {
                    // Serial.println("OTA Update finished successfully.");
                    otaSuccessCallback();  // Call success callback
                } else {
                    // Serial.println("OTA Update failed to complete.");
                    otaErrorCallback("OTA Update did not finish, something went wrong!");
                }
            } else {
                // Log detailed error message
                String errorMessage = "OTA failed. Error #: " + String(Update.getError());

                // Print to Serial for more debugging info
                // Serial.print("Detailed Error: ");
                // Update.printError(Serial);  // Detailed error output

                // Manually create human-readable error messages
                switch (Update.getError()) {
                    case UPDATE_ERROR_WRITE:
                        errorMessage += " - Write error occurred.";
                        break;
                    case UPDATE_ERROR_ERASE:
                        errorMessage += " - Erase error.";
                        break;
                    case UPDATE_ERROR_READ:
                        errorMessage += " - Read error occurred.";
                        break;
                    case UPDATE_ERROR_SPACE:
                        errorMessage += " - Insufficient space.";
                        break;
                    case UPDATE_ERROR_SIZE:
                        errorMessage += " - Firmware size mismatch.";
                        break;
                    case UPDATE_ERROR_STREAM:
                        errorMessage += " - Stream read timeout.";
                        break;
                    case UPDATE_ERROR_MD5:
                        errorMessage += " - MD5 check failed.";
                        break;
                    case UPDATE_ERROR_MAGIC_BYTE:
                        errorMessage += " - Magic byte mismatch.";
                        break;
                    case UPDATE_ERROR_NO_PARTITION:
                        errorMessage += " - No partition available.";
                        break;
                    case UPDATE_ERROR_BAD_ARGUMENT:
                        errorMessage += " - Invalid argument.";
                        break;
                    default:
                        errorMessage += " - Unknown error.";
                        break;
                }

                // Serial.println(errorMessage);    // Print error message
                otaErrorCallback(errorMessage);  // Send error callback
            }
        } else {
            // Serial.println("Not enough space to begin OTA.");
            otaErrorCallback("Not enough space to begin OTA.");
        }

    } catch (const std::runtime_error& e) {
        otaErrorCallback(e.what());
    } catch (const std::exception& e) {
        otaErrorCallback(e.what());
    } catch (...) {
        otaErrorCallback("Unknown error occurred during OTA.");
    }
}