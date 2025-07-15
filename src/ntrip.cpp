#include <UBLOX/ntrip.h>

namespace ntrip
{

NTRIP::NTRIP(SerialInterface& ser, const std::string& host, int port,
             const std::string& mountpoint,
             const std::string& username,
             const std::string& password,
             double lat, double lon, double alt)
    : NTRIPUtils(host, port, mountpoint, username, password, lat, lon, alt), ser_(ser) 
    {
    ser_.add_listener(this);
    }

NTRIP::~NTRIP()
{
    // Cleanup if necessary
}

void NTRIP::config_rtcm()
{
    rtcm_.registerListener(this);
}

void NTRIP::read_cb(const uint8_t *buf, size_t size)
{
            rtcm_.read_cb(buf, size);
}

void NTRIP::got_rtcm(const uint8_t *buf, const size_t size)
{
    // Base class no-op or log
    printf("[UBLOX] RTCM message received (%zu bytes).\n", size);
}

}  // namespace ntrip

