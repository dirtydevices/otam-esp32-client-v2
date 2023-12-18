#include "internal/OtamUpdater.h"

void OtamUpdater::handleError(String error) {
    OtamLogger::error(error);
    if (otaErrorCallback)  // Check if the callback has been set
    {
        otaErrorCallback(error);  // Call the callback
    }
}

void OtamUpdater::onOtaSuccess(CallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

void OtamUpdater::onOtaError(ErrorCallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

void OtamUpdater::runESP32Update(HTTPClient& http) {
    try {
        int contentLength = http.getSize();

        bool canBegin = Update.begin(contentLength);
        if (canBegin) {
            WiFiClient* client = http.getStreamPtr();

            // Write the downloaded binary to flash memory
            size_t written = Update.writeStream(*client);

            if (written == contentLength) {
                OtamLogger::info("Written : " + String(written) + " successfully");
            } else {
                OtamLogger::info("Written only : " + String(written) + "/" + String(contentLength) +
                                 ". Retry?");
            }

            if (Update.end()) {
                OtamLogger::info("OTA done!");
                if (Update.isFinished()) {
                    OtamLogger::info("Update successfully completed. Rebooting.");
                    if (otaSuccessCallback)  // Check if the callback has been set
                    {
                        otaSuccessCallback();  // Call the callback
                    }
                } else {
                    handleError("Update did not finish, something went wrong!");
                }
            } else {
                handleError("Error Occurred. Error #: " + String(Update.getError()));
            }
        } else {
            handleError("Not enough space to begin OTA");
        }

    } catch (const std::runtime_error& e) {
        handleError(e.what());
    } catch (const std::exception& e) {
        handleError(e.what());
    } catch (...) {
        handleError("Unknown error occurred");
    }
}