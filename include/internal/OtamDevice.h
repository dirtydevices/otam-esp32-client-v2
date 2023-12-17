#ifndef OTAM_DEVICE_H
#define OTAM_DEVICE_H

#include "internal/OtamConfig.h"
#include "internal/OtamLogDb.h"
#include "internal/OtamStore.h"
#include "internal/OtamUpdater.h"
#include "internal/OtamUtils.h"
#include "internal/OtamHttp.h"
#include "internal/OtamLogger.h"

class OtamDevice
{
private:
    void writeIdToStore(String id);
    void initialize(OtamConfig config);

public:
    String deviceId;
    String deviceName;
    String deviceUrl;
    String deviceLogUrl;
    String deviceStatusUrl;
    String deviceInitializeUrl;
    String deviceDownloadUrl;
    OtamLogDb *logDb;
    OtamDevice(OtamConfig config);
};

#endif // OTAM_DEVICE_H