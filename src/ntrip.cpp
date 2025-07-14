#include <UBLOX/ntrip.h>

std::string base64_encode(const std::string &in) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

bool NTRIPClient::connect_and_stream() {
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_ < 0) {
        perror("Socket creation failed");
        return false;
    }

    struct hostent* server = gethostbyname(host_.c_str());
    if (server == nullptr) {
        std::cerr << "No such host: " << host_ << std::endl;
        return false;
    }

    struct sockaddr_in server_addr{};
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to caster" << std::endl;
        ::close(sock_fd_);
        sock_fd_ = -1;
        return false;
    }

    std::cout << "Connected to caster.\n";

    std::string request_header = build_request_header();
    send(sock_fd_, request_header.c_str(), request_header.size(), 0);

    std::string gga_sentence = generate_gga_sentence();
    std::cout << "Sending GGA: " << gga_sentence;
    send(sock_fd_, gga_sentence.c_str(), gga_sentence.size(), 0);

    // Read caster response headers
    char buffer[4096];
    int n = recv(sock_fd_, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << "Caster response:\n" << buffer << "\n";
    }

    std::cout << "Starting RTCM stream...\n";

    while ((n = recv(sock_fd_, buffer, sizeof(buffer), 0)) > 0) {
        parse_rtcm_messages(buffer, n);
    }

    ::close(sock_fd_);
    return true;
}

void NTRIPClient::close() {
    if (sock_fd_ != -1) {
        ::close(sock_fd_);
        std::cout << "NTRIP connection closed." << std::endl;
        sock_fd_ = -1;
    }
}

std::string NTRIPClient::build_request_header() {
    std::string auth = base64_encode(username_ + ":" + password_);
    std::ostringstream oss;
    oss << "GET /" << mountpoint_ << " HTTP/1.1\r\n"
        << "Host: " << host_ << ":" << port_ << "\r\n"
        << "Ntrip-Version: Ntrip/2.0\r\n"
        << "User-Agent: NTRIP Killis Swarmer/1.0\r\n"
        << "Authorization: Basic " << auth << "\r\n"
        << "Connection: close\r\n\r\n";
    return oss.str();
}

std::string NTRIPClient::generate_gga_sentence() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    struct tm* ptm = std::gmtime(&tt);
    char time_buf[10];
    std::strftime(time_buf, sizeof(time_buf), "%H%M%S", ptm);

    std::ostringstream gga;
    gga << "$GPGGA," << time_buf << ".00,"
        << decimal_to_nmea(latitude_, true) << ","
        << decimal_to_nmea(longitude_, false) << ",1,12,1.0,"
        << std::fixed << std::setprecision(1) << altitude_
        << ",M,0.0,M,,*";

    unsigned char checksum = 0;
    std::string gga_str = gga.str();
    for (size_t i = 1; i < gga_str.size(); ++i) {
        checksum ^= gga_str[i];
    }

    std::ostringstream final_gga;
    final_gga << gga_str << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << (int)checksum << "\r\n";

    return final_gga.str();
}

std::string NTRIPClient::decimal_to_nmea(double deg, bool is_lat) {
    int degrees = (int)std::abs(deg);
    double minutes = (std::abs(deg) - degrees) * 60.0;

    std::ostringstream oss;
    if (is_lat)
        oss << std::setw(2) << std::setfill('0') << degrees
            << std::fixed << std::setprecision(4) << minutes
            << (deg >= 0 ? ",N" : ",S");
    else
        oss << std::setw(3) << std::setfill('0') << degrees
            << std::fixed << std::setprecision(4) << minutes
            << (deg >= 0 ? ",E" : ",W");

    return oss.str();
}

void NTRIPClient::parse_rtcm_messages(const char* data, size_t len) {
    size_t i = 0;
    while (i < len) {
        // Check RTCM message sync byte
        if ((unsigned char)data[i] != 0xD3) {
            ++i;
            continue;
        }

        if (i + 3 > len) break; // wait for full header

        uint16_t length = ((data[i + 1] & 0x03) << 8) | (unsigned char)data[i + 2];
        if (i + 3 + length + 3 > len) break; // wait for full message (payload + 3-byte CRC)

        // Extract message number (10 bits from payload)
        uint16_t msg_num = ((unsigned char)data[i + 3] << 4) | ((data[i + 4] & 0xF0) >> 4);

        std::cout << "RTCM Message: " << msg_num << " | Length: " << (length + 6) << " bytes\n";
        i += 3 + length + 3;
    }
}

