#include "kv/key_value_store.h"
#include "kv/persistence.h"
#include "kv/protocol.h"
#include "kv/thread_pool.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
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
        int n = send(s, data.data() + sent, static_cast<int>(data.size() - sent), 0);
#else
        ssize_t n = send(s, data.data() + sent, data.size() - sent, 0);
#endif
        if (n <= 0) return false;
        sent += static_cast<std::size_t>(n);
    }
    return true;
}

void handle_client(socket_t client, kv::KeyValueStore& store, kv::Persistence& persistence) {
    std::string buffer;
    char temp[4096];
    while (true) {
#ifdef _WIN32
        int n = recv(client, temp, sizeof(temp), 0);
#else
        ssize_t n = recv(client, temp, sizeof(temp), 0);
#endif
        if (n <= 0) break;
        buffer.append(temp, static_cast<std::size_t>(n));

        std::size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const std::string response = kv::protocol::process_command(line, store, persistence) + "\n";
            if (!send_all(client, response)) {
                close_socket(client);
                return;
            }
            if (line == "QUIT") {
                close_socket(client);
                return;
            }
        }
    }
    close_socket(client);
}

} // namespace

int main(int argc, char* argv[]) {
    int port = 6379;
    std::size_t threads = std::max(1u, std::thread::hardware_concurrency());
    if (argc >= 2) port = std::stoi(argv[1]);
    if (argc >= 3) threads = static_cast<std::size_t>(std::stoul(argv[2]));

#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    kv::KeyValueStore store(16);
    kv::Persistence persistence("data/store.snapshot");
    persistence.load(store); // First run is fine if no snapshot exists.
    kv::ThreadPool pool(threads);

    socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET_VALUE) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0 || listen(server, 128) < 0) {
        std::cerr << "Failed to bind/listen on port " << port << "\n";
        close_socket(server);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "============================================\n"
              << "   HIGH-PERFORMANCE KEY-VALUE STORE\n"
              << "============================================\n"
              << "Port: " << port << "\n"
              << "Worker threads: " << threads << "\n"
              << "Shards: 16\n"
              << "Commands: SET GET DELETE EXISTS SIZE SAVE LOAD CLEAR PING QUIT\n"
              << "Waiting for clients...\n";

    while (true) {
        sockaddr_in client_addr{};
#ifdef _WIN32
        int len = sizeof(client_addr);
#else
        socklen_t len = sizeof(client_addr);
#endif
        socket_t client = accept(server, reinterpret_cast<sockaddr*>(&client_addr), &len);
        if (client == INVALID_SOCKET_VALUE) continue;
        pool.submit(handle_client, client, std::ref(store), std::ref(persistence));
    }

    close_socket(server);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
