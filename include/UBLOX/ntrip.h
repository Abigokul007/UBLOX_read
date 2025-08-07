#pragma once

#include <stdint.h>
#include <fstream>
#include <functional>
#include <iostream>

#include "UBLOX/parsers/rtcm.h"
#include "UBLOX/serial_interface.h"
#include "UBLOX/ntrip_utils.h"

namespace ntrip
{
class NTRIP : public SerialListener, public ntrip::NTRIPUtils
{
public:

    NTRIP(SerialInterface& ser, const std::string& host, int port,
            const std::string& mountpoint,
            const std::string& username,
            const std::string& password,
            double lat, double lon, double alt);

    ~NTRIP();

    // NTRIP read/write
    void read_cb(const uint8_t* buf, size_t size) override;

    inline void registerRTCMListener(RTCMListener* l) { rtcm_.registerListener(l); }

private:

    RTCM rtcm_;
    SerialInterface& ser_;

};

} 