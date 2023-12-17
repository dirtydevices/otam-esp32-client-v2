#include "internal/OtamUpdater.h"

void OtamUpdater::onOtaSuccess(CallbackType successCallback) {
    otaSuccessCallback = successCallback;
}

void OtamUpdater::onOtaError(CallbackType errorCallback) {
    otaErrorCallback = errorCallback;
}

void OtamUpdater::runESP32Update(HTTPClient& http) {
    int contentLength = http.getSize();

    bool canBegin = Update.begin(contentLength);
    if (canBegin) {
        WiFiClient* client = http.getStreamPtr();

        // Write the downloaded binary to flash memory
        size_t written = Update.writeStream(*client);

        if (written == contentLength) {
            OtamLogger::info("Written : " + String(written) + " successfully");
        } else {
            OtamLogger::info("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
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
                OtamLogger::error("Update not finished? Something went wrong!");
                if (otaErrorCallback)  // Check if the callback has been set
                {
                    otaErrorCallback();  // Call the callback
                }
            }
        } else {
            OtamLogger::error("Error Occurred. Error #: " + String(Update.getError()));
            if (otaErrorCallback)  // Check if the callback has been set
            {
                otaErrorCallback();  // Call the callback
            }
        }
    } else {
        OtamLogger::error("Not enough space to begin OTA");
        if (otaErrorCallback)  // Check if the callback has been set
        {
            otaErrorCallback();  // Call the callback
        }
    }
}