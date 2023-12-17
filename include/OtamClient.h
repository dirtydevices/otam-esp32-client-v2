#ifndef OTAM_CLIENT_H
#define OTAM_CLIENT_H

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
    using CallbackType = std::function<void(FirmwareUpdateValues)>;
    CallbackType otaSuccessCallback;
    CallbackType otaErrorCallback;
    void onOtaSuccess(CallbackType successCallback);
    void onOtaError(CallbackType errorCallback);
    void setLogLevel(String logLevel);
    void initialize(OtamConfig config);
    OtamHttpResponse logDeviceMessage(String message);
    boolean hasPendingUpdate();
    void doFirmwareUpdate();
};

#endif  // OTAM_CLIENT_H
