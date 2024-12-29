#ifndef OTAM_DEVICE_H
#define OTAM_DEVICE_H

#include "internal/OtamConfig.h"
#include "internal/OtamHttp.h"
#include "internal/OtamStore.h"
#include "internal/OtamUpdater.h"
#include "internal/OtamUtils.h"

class OtamDevice {
   private:
    void writeIdToStore(String id);
    void initialize(OtamConfig config);

   public:
    String deviceGuid;
    String deviceLogUrl;
    String hasUpdateUrl;
    String deviceInitializeUrl;
    String deviceFirmwareFileUrl;
    explicit OtamDevice(OtamConfig config);
};

#endif  // OTAM_DEVICE_H