#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

// Winsock2 must be included before any Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

#include "ThreadPool.h"
#include "HttpParser.h"
#include "HttpHandler.h"

#pragma comment(lib, "Ws2_32.lib")

namespace fs = std::filesystem;

static const int    PORT        = 8080;
static const int    BACKLOG     = 64;
static const size_t THREAD_COUNT = 4;
static const size_t RECV_BUF   = 8192;

// Derive the static directory relative to the executable's location.
// Falls back to a hardcoded path when running inside the debugger.
static std::string getStaticDir() {
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    fs::path exe_dir = fs::path(buf).parent_path();
    fs::path candidate = exe_dir.parent_path().parent_path() / "static";
    if (fs::exists(candidate))
        return candidate.string();
    // Fallback: cwd/static
    return (fs::current_path() / "static").string();
}

static void handleClient(SOCKET client_sock, const std::string& static_dir) {
    char buf[RECV_BUF] = {};
    int bytes = recv(client_sock, buf, sizeof(buf) - 1, 0);
    if (bytes > 0) {
        std::string raw_request(buf, static_cast<size_t>(bytes));
        HttpRequest req     = parseRequest(raw_request);
        std::string response = buildResponse(req, static_dir);
        send(client_sock, response.c_str(), static_cast<int>(response.size()), 0);
    }
    closesocket(client_sock);
}

int main() {
    // Initialise Winsock
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "[ERROR] WSAStartup failed\n";
        return 1;
    }

    const std::string static_dir = getStaticDir();
    std::cout << "[INFO]  Serving files from: " << static_dir << "\n";

    // Create listening socket
    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "[ERROR] socket() failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Allow address reuse so we can restart quickly
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<u_short>(PORT));

    if (bind(server_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] bind() failed: " << WSAGetLastError() << "\n";
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    if (listen(server_sock, BACKLOG) == SOCKET_ERROR) {
        std::cerr << "[ERROR] listen() failed: " << WSAGetLastError() << "\n";
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    std::cout << "[INFO]  HTTP server listening on http://localhost:" << PORT << "\n";
    std::cout << "[INFO]  Thread pool size: " << THREAD_COUNT << "\n";
    std::cout << "[INFO]  Press Ctrl+C to stop.\n\n";

    ThreadPool pool(THREAD_COUNT);

    while (true) {
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);

        SOCKET client_sock = accept(server_sock,
                                    reinterpret_cast<sockaddr*>(&client_addr),
                                    &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            std::cerr << "[WARN]  accept() failed: " << WSAGetLastError() << "\n";
            continue;
        }

        // Log the incoming connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "[CONN]  " << client_ip << ":"
                  << ntohs(client_addr.sin_port) << "\n";

        // Dispatch to thread pool
        pool.enqueue([client_sock, static_dir]() {
            handleClient(client_sock, static_dir);
        });
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
