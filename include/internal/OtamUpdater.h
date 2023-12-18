#ifndef OTAM_UPDATER_H
#define OTAM_UPDATER_H

#include <HTTPClient.h>
#include <Update.h>
#include <functional>
#include "internal/OtamLogger.h"

class OtamUpdater {
   private:
    void handleError(String error);

   public:
    // Define the type for the callback functions
    using CallbackType = std::function<void()>;
    using ErrorCallbackType = std::function<void(String)>;

    // Variables to hold the callback functions
    CallbackType otaSuccessCallback;
    ErrorCallbackType otaErrorCallback;

    void onOtaSuccess(CallbackType successCallback);
    void onOtaError(ErrorCallbackType errorCallback);
    void runESP32Update(HTTPClient& http);
};

#endif  // OTAM_UPDATER_H