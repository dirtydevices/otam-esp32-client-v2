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
    OtamConfig clientOtamConfig;
    bool deviceInitialized = false;
    OtamDevice* otamDevice;
    bool updateStarted = false;
    FirmwareUpdateValues firmwareUpdateValues;
    FirmwareUpdateValues readFirmwareValuesFromStore();
    void sendOtaUpdateError(String logMessage);

   public:
    OtamClient(const OtamConfig& config);
    using CallbackType = std::function<void(FirmwareUpdateValues)>;
    using ErrorCallbackType = std::function<void(FirmwareUpdateValues, String)>;
    CallbackType otaSuccessCallback;
    ErrorCallbackType otaErrorCallback;
    void onOtaSuccess(CallbackType successCallback);
    void onOtaError(ErrorCallbackType errorCallback);
    void setLogLevel(String logLevel);
    void initialize();
    OtamHttpResponse logDeviceMessage(String message);
    boolean hasPendingUpdate();
    void doFirmwareUpdate();
};

#endif  // OTAM_CLIENT_H
