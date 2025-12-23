#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

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
static const char* BROADCAST_IP = "255.255.255.255";

int main() {
#ifdef _WIN32
    // 1) Windows-specific init for sockets
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[node2_pub] WSAStartup failed\n";
        return 1;
    }
#endif

    // 2) Create UDP socket
    int sock =
#ifdef _WIN32
        static_cast<int>(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
#else
        socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (sock < 0) {
        std::cerr << "[node2_pub] failed to create UDP socket\n";
        return 1;
    }

    // 3) Enable broadcast on this socket
    int enable = 1;
#ifdef _WIN32
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<const char*>(&enable), sizeof(enable)) != 0) {
        std::cerr << "[node2_pub] setsockopt(SO_BROADCAST) failed\n";
    }
#else
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
#endif

    // 4) Destination address: broadcast on port 1111
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    // 5) Random generators:
    //    estop: 0 or 1
    //    sleep: random interval between messages
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> estop_dist(0, 1);
    std::uniform_int_distribution<int> sleep_ms_dist(1500, 4000);  // 1.5s..4s

    std::cout << "[node2_pub] broadcasting estop on UDP port 1111...\n";

    while (true) {
        int estop = estop_dist(rng);
        std::string msg = "estop," + std::to_string(estop);

        int sent =
#ifdef _WIN32
            sendto(sock, msg.c_str(), static_cast<int>(msg.size()), 0,
                   reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#else
            sendto(sock, msg.c_str(), msg.size(), 0,
                   reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#endif

        if (sent < 0) {
            std::cerr << "[node2_pub] sendto failed\n";
        } else {
            std::cout << "[node2_pub] sent: " << msg << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms_dist(rng)));
    }

    // (unreachable in this loop, but good practice)
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}