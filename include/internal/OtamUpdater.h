#ifndef OTAM_UPDATER_H
#define OTAM_UPDATER_H

#include <HTTPClient.h>
#include <Update.h>
#include <functional>
#include "internal/OtamLogger.h"

class OtamUpdater
{
public:
    // Define the type for the callback functions
    using CallbackType = std::function<void()>;

    // Variables to hold the callback functions
    CallbackType otaSuccessCallback;
    CallbackType otaErrorCallback;

    void onOtaSuccess(CallbackType successCallback);
    void onOtaError(CallbackType errorCallback);
    void runESP32Update(HTTPClient &http);
};

#endif // OTAM_UPDATER_H