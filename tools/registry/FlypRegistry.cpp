// flyp-registry — minimal REST package registry for Fly
//
// API:
//   GET  /v1/{name}                    → JSON array of available versions
//   GET  /v1/{name}/{version}          → fly.toml content (package metadata)
//   GET  /v1/{name}/{version}/download → source tarball (.tar.gz)
//   POST /v1/{name}/{version}          → upload a source tarball
//   GET  /v1/search?q={query}          → JSON array of matching package names
//
// Usage:
//   flyp-registry [--storage DIR] [--port PORT] [--host HOST]

#include "include/RegistryHandler.h"

#include <CLI/CLI.hpp>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
   using sock_t = SOCKET;
#  define INVALID_SOCK INVALID_SOCKET
#  define CLOSE_SOCK   closesocket
#else
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <unistd.h>
   using sock_t = int;
#  define INVALID_SOCK (-1)
#  define CLOSE_SOCK   close
#endif

using flyp::registry::Request;
using flyp::registry::Response;
using flyp::registry::RegistryHandler;

// ── HTTP parser ───────────────────────────────────────────────────────────────

static Request parse_request(const std::string& raw) {
    Request req;
    if (raw.empty()) return req;

    std::istringstream ss(raw);
    std::string request_line;
    std::getline(ss, request_line);
    if (!request_line.empty() && request_line.back() == '\r')
        request_line.pop_back();

    std::istringstream rl(request_line);
    std::string url;
    rl >> req.method >> url;

    auto q = url.find('?');
    if (q != std::string::npos) { req.path = url.substr(0, q); req.query = url.substr(q+1); }
    else req.path = url;

    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        auto colon = line.find(':');
        if (colon != std::string::npos)
            req.headers[line.substr(0, colon)] = line.substr(colon + 2);
    }
    auto he = raw.find("\r\n\r\n");
    if (he != std::string::npos) req.body = raw.substr(he + 4);
    return req;
}

// ── Connection handler ────────────────────────────────────────────────────────

static void handle_connection(sock_t client, const RegistryHandler& handler) {
    std::string raw;
    std::array<char, 4096> buf{};
    ssize_t n;
    while ((n = recv(client, buf.data(), buf.size(), 0)) > 0) {
        raw.append(buf.data(), static_cast<size_t>(n));
        auto he = raw.find("\r\n\r\n");
        if (he != std::string::npos) {
            size_t cl = 0;
            auto cl_pos = raw.find("Content-Length:");
            if (cl_pos != std::string::npos) {
                auto nl = raw.find("\r\n", cl_pos);
                cl = std::stoull(raw.substr(cl_pos + 15,
                    nl != std::string::npos ? nl - cl_pos - 15 : std::string::npos));
            }
            size_t body_start = he + 4;
            while (raw.size() - body_start < cl) {
                n = recv(client, buf.data(),
                         std::min(buf.size(), cl - (raw.size() - body_start)), 0);
                if (n <= 0) break;
                raw.append(buf.data(), static_cast<size_t>(n));
            }
            break;
        }
    }

    if (!raw.empty()) {
        Request  req  = parse_request(raw);
        Response resp = handler.handle(req);
        std::string r = resp.to_http();
        send(client, r.data(), static_cast<int>(r.size()), 0);
    }
    CLOSE_SOCK(client);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    CLI::App app{"flyp-registry — Fly package registry server"};

    std::string storage_str;
    int         port  = 5000;
    std::string host  = "0.0.0.0";
    std::string token;

    app.add_option("--storage", storage_str,
        "Directory to store packages (default: ~/.flyp/registry)");
    app.add_option("--port,-p", port, "Port to listen on (default: 5000)");
    app.add_option("--host",    host, "Host to bind to (default: 0.0.0.0)");
    app.add_option("--token",   token,
        "API key required for publish (POST) operations.\n"
        "Can also be set via FLYP_REGISTRY_TOKEN environment variable.\n"
        "Read operations (GET) are always public.");
    CLI11_PARSE(app, argc, argv);

    // Environment variable fallback.
    if (token.empty()) {
        const char* env = std::getenv("FLYP_REGISTRY_TOKEN");
        if (env && *env) token = env;
    }

    std::filesystem::path storage;
    if (storage_str.empty()) {
        const char* home = std::getenv("FLYP_HOME");
        storage = home && *home
            ? std::filesystem::path(home) / "registry"
            : std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : ".")
              / ".flyp" / "registry";
    } else {
        storage = storage_str;
    }

    RegistryHandler handler(storage, token);

    std::cout << "flyp-registry\n"
              << "  storage : " << storage.string() << "\n"
              << "  listen  : " << host << ":" << port << "\n"
              << "  auth    : " << (token.empty() ? "disabled" : "Bearer token") << "\n\n";

#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    sock_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCK) { std::cerr << "error: socket() failed\n"; return 1; }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = inet_addr(host.c_str());

    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "error: bind() failed\n"; return 1;
    }
    if (listen(server, 16) < 0) {
        std::cerr << "error: listen() failed\n"; return 1;
    }

    while (true) {
        sock_t client = accept(server, nullptr, nullptr);
        if (client != INVALID_SOCK) handle_connection(client, handler);
    }

    CLOSE_SOCK(server);
    return 0;
}
