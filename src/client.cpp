#include "kv/protocol.h"

#include <iostream>
#include <string>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
constexpr socket_t INVALID_SOCKET_VALUE = -1;
#endif

namespace {

void close_socket(socket_t s) {
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

bool send_all(socket_t s, const std::string& data) {
    std::size_t sent = 0;

    while (sent < data.size()) {

#ifdef _WIN32
        int n = send(
            s,
            data.data() + sent,
            static_cast<int>(data.size() - sent),
            0
        );
#else
        ssize_t n = send(
            s,
            data.data() + sent,
            data.size() - sent,
            0
        );
#endif

        if (n <= 0) {
            return false;
        }

        sent += static_cast<std::size_t>(n);
    }

    return true;
}

std::string receive_line(socket_t s) {
    std::string result;
    char c;

    while (true) {

#ifdef _WIN32
        int n = recv(s, &c, 1, 0);
#else
        ssize_t n = recv(s, &c, 1, 0);
#endif

        if (n <= 0) {
            return {};
        }

        if (c == '\n') {
            return result;
        }

        result += c;

        if (result.size() > 1024 * 1024) {
            return {};
        }
    }
}

} // namespace

int main(int argc, char* argv[]) {

    const char* host = argc >= 2 ? argv[1] : "127.0.0.1";
    int port = argc >= 3 ? std::stoi(argv[2]) : 6379;

#ifdef _WIN32

    WSADATA wsa{};

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

#endif

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == INVALID_SOCKET_VALUE) {
        std::cerr << "Socket creation failed\n";

#ifdef _WIN32
        WSACleanup();
#endif

        return 1;
    }

    sockaddr_in addr{};

    addr.sin_family = AF_INET;

    addr.sin_port = htons(
        static_cast<std::uint16_t>(port)
    );

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {

        std::cerr << "Invalid IPv4 address\n";

        close_socket(sock);

#ifdef _WIN32
        WSACleanup();
#endif

        return 1;
    }

    if (connect(
            sock,
            reinterpret_cast<sockaddr*>(&addr),
            sizeof(addr)
        ) < 0) {

        std::cerr << "Connection failed\n";

        close_socket(sock);

#ifdef _WIN32
        WSACleanup();
#endif

        return 1;
    }

    std::cout
        << "Connected to "
        << host
        << ':'
        << port
        << "\n"
        << "Examples: "
        << "SET name Girisha | "
        << "GET name | "
        << "SET session abc 5 | "
        << "QUIT\n";

    std::string line;

    while (std::cout << "> " &&
           std::getline(std::cin, line)) {

        if (line.empty()) {
            continue;
        }

        if (!send_all(sock, line + "\n")) {
            std::cerr << "Failed to send request\n";
            break;
        }

        std::string response = receive_line(sock);

        if (response.empty()) {
            std::cerr << "Server disconnected\n";
            break;
        }

        std::cout << response << '\n';

        if (line == "QUIT") {
            break;
        }
    }

    close_socket(sock);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}