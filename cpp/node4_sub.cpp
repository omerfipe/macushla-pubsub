#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
#endif

static constexpr int PORT = 1111;

enum class SafetyState {
    SAFE,
    STOP
};

static long long ms_since(const std::chrono::steady_clock::time_point& start) {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

static bool parse_topic_data(const std::string& s, std::string& topic, std::string& data) {
    auto pos = s.find(',');
    if (pos == std::string::npos) return false;

    topic = s.substr(0, pos);
    data  = s.substr(pos + 1);

    // trim simple spaces
    while (!topic.empty() && (topic.back() == '\n' || topic.back() == '\r' || topic.back() == ' ')) topic.pop_back();
    while (!data.empty()  && (data.back()  == '\n' || data.back()  == '\r' || data.back()  == ' ')) data.pop_back();

    while (!topic.empty() && topic.front() == ' ') topic.erase(topic.begin());
    while (!data.empty()  && data.front()  == ' ') data.erase(data.begin());

    return !topic.empty() && !data.empty();
}

int main() {
#ifdef _WIN32
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[node4_sub] WSAStartup failed\n";
        return 1;
    }
#endif

    int sock =
#ifdef _WIN32
        static_cast<int>(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
#else
        socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (sock < 0) {
        std::cerr << "[node4_sub] failed to create UDP socket\n";
        return 1;
    }

    // Allow multiple listeners on same port (best effort)
    int enable = 1;
#ifdef _WIN32
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
#endif

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (
#ifdef _WIN32
        bind(sock, reinterpret_cast<sockaddr*>(&local), sizeof(local)) != 0
#else
        bind(sock, reinterpret_cast<sockaddr*>(&local), sizeof(local)) != 0
#endif
    ) {
        std::cerr << "[node4_sub] bind failed (is something else using the port?)\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    std::ofstream log("safety_log.txt", std::ios::app);
    if (!log.is_open()) {
        std::cerr << "[node4_sub] failed to open safety_log.txt\n";
        return 1;
    }

    auto start = std::chrono::steady_clock::now();
    SafetyState state = SafetyState::SAFE;

    std::cout << "[node4_sub] listening on UDP port 1111 (topic=estop)\n";

    while (true) {
        char buf[2048];
        sockaddr_in from{};
#ifdef _WIN32
        int from_len = sizeof(from);
        int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, reinterpret_cast<sockaddr*>(&from), &from_len);
#else
        socklen_t from_len = sizeof(from);
        int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, reinterpret_cast<sockaddr*>(&from), &from_len);
#endif
        if (n <= 0) {
            continue;
        }
        buf[n] = '\0';
        std::string raw(buf);

        std::string topic, data;
        if (!parse_topic_data(raw, topic, data)) {
            continue;
        }

        if (topic != "estop") {
            continue;  // node4 listens only to estop
        }

        if (data != "0" && data != "1") {
            continue;  // invalid
        }

        SafetyState new_state = (data == "1") ? SafetyState::STOP : SafetyState::SAFE;

        if (new_state != state) {
            state = new_state;

            long long t = ms_since(start);
            const char* state_str = (state == SafetyState::STOP) ? "STOP" : "SAFE";

            log << t << " ms: " << state_str << "\n";
            log.flush();

            std::cout << "[node4_sub] transition -> " << state_str << " (logged)\n";
        }
    }

    // unreachable
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}
