#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>

namespace ntrip
{

class NTRIPUtils
{
public:
    NTRIPUtils(const std::string& host, int port,
                const std::string& mountpoint,
                const std::string& username,
                const std::string& password,
                double lat, double lon, double alt)
        : host_(host), port_(port), mountpoint_(mountpoint),
          username_(username), password_(password),
          latitude_(lat), longitude_(lon), altitude_(alt) {}

    // bool connect_and_stream();
    // void close();

    std::string base64_encode(const std::string &in);

    std::string base64_encode(const uint8_t* buf, size_t size);

    std::string base64_encode(const char* buf, size_t size);

    std::string build_request_header();

    std::string generate_gga_sentence();

    std::string decimal_to_nmea(double deg, bool is_lat);

private:
    // int sock_fd_ = -1;
    std::string host_;
    int port_;
    std::string mountpoint_;
    std::string username_;
    std::string password_;
    double latitude_, longitude_, altitude_;

    // void parse_rtcm_messages(const char* data, size_t len);
};

} // namespace ntrip