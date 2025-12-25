#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

static constexpr int PORT = 1111;

int main() {
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket failed\n";
        WSACleanup();
        return 1;
    }

    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        std::cerr << "bind failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::ofstream log("safety_log.txt", std::ios::app);
    auto t0 = std::chrono::steady_clock::now();

    int state = 0; // 0=SAFE, 1=STOP
    std::cout << "[node4_sub] listening on UDP :1111 (estop)\n";

    while (true) {
        char buf[2048];
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) continue;
        buf[n] = '\0';

        std::string s(buf);
        auto comma = s.find(',');
        if (comma == std::string::npos) continue;

        std::string topic = s.substr(0, comma);
        std::string data = s.substr(comma + 1);
        while (!data.empty() && (data.back() == '\n' || data.back() == '\r' || data.back() == ' ')) data.pop_back();

        if (topic != "estop") continue;
        if (data != "0" && data != "1") continue;

        int new_state = (data == "1") ? 1 : 0;
        if (new_state == state) continue;

        state = new_state;

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0)
                      .count();

        log << ms << " ms: " << (state ? "STOP" : "SAFE") << "\n";
        log.flush();

        std::cout << "[node4_sub] transition -> " << (state ? "STOP" : "SAFE") << "\n";
    }
}
