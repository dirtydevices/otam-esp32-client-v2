#ifndef OTAM_CLIENT_H
#define OTAM_CLIENT_H

#include <HttpClient.h>
#include "internal/OtamConfig.h"
#include "internal/OtamDevice.h"
#include "internal/OtamHttp.h"

#define HAS_UPDATE_CACHE_SIZE 256  // Defined buffer size

struct FirmwareUpdateValues {
    int firmwareId;
    String firmwareVersion;
};

class OtamClient {
   private:
    OtamConfig clientOtamConfig;
    OtamDevice* otamDevice;
    bool deviceInitialized = false;
    bool updateStarted = false;
    FirmwareUpdateValues firmwareUpdateValues;
    void sendOtaUpdateError(String logMessage);
    char hasUpdateCache[HAS_UPDATE_CACHE_SIZE];  // Fixed-size buffer

   public:
    explicit OtamClient(const OtamConfig& config);
    using EmptyCallbackType = std::function<void()>;
    using NumberCallbackType = std::function<void(int)>;
    using ErrorCallbackType = std::function<void(String)>;
    NumberCallbackType otaDownloadProgressCallback;
    EmptyCallbackType otaBeforeDownloadCallback;
    EmptyCallbackType otaAfterDownloadCallback;
    EmptyCallbackType otaBeforeRebootCallback;
    EmptyCallbackType otaSuccessCallback;
    ErrorCallbackType otaErrorCallback;
    void onOtaDownloadProgress(NumberCallbackType progressCallback);
    void onOtaBeforeDownload(EmptyCallbackType beforeDownloadCallback);
    void onOtaAfterDownload(EmptyCallbackType afterDownloadCallback);
    void onOtaBeforeReboot(EmptyCallbackType beforeRebootCallback);
    void onOtaSuccess(EmptyCallbackType successCallback);
    void onOtaError(ErrorCallbackType errorCallback);
    bool isInitialized();
    void initialize();
    OtamHttpResponse logDeviceMessage(String message);
    const char* hasFirmwareUpdate();
    void doFirmwareUpdate(const char* downloadUrl);
};

#endif  // OTAM_CLIENT_H
