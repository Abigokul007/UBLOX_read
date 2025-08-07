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

void NTRIP::read_cb(const uint8_t* buf, size_t size)
{
    // printf("[NTRIP] Received %zu bytes\n", size);
    rtcm_.read_cb(buf, size);
}

}  // namespace ntrip

