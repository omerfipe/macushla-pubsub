#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

static constexpr int PORT = 1111;
static const char* BROADCAST_IP = "255.255.255.255";

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
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> bit(0, 1);
    std::uniform_int_distribution<int> sleep_ms(1500, 4000);

    std::cout << "[node2_pub] sending estop on UDP broadcast:1111\n";

    while (true) {
        std::string msg = "estop," + std::to_string(bit(rng));
        int rc = sendto(sock, msg.c_str(), (int)msg.size(), 0, (sockaddr*)&addr, sizeof(addr));
        if (rc != SOCKET_ERROR) std::cout << "[node2_pub] " << msg << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms(rng)));
    }
}
