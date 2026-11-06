#include "web/HttpServer.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "core/Utils.h"

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
static const socket_t BAD_SOCKET = INVALID_SOCKET;
static void closeSocket(socket_t s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
typedef int socket_t;
static const socket_t BAD_SOCKET = -1;
static void closeSocket(socket_t s) { close(s); }
#endif

namespace web {

namespace {

// RAII helper: initialises WinSock once for the lifetime of the program.
class SocketRuntime {
public:
    SocketRuntime() {
#ifdef _WIN32
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) throw std::runtime_error("WSAStartup failed");
#endif
    }
    ~SocketRuntime() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

void setReceiveTimeout(socket_t s, int milliseconds) {
#ifdef _WIN32
    DWORD timeout = static_cast<DWORD>(milliseconds);
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

std::size_t contentLength(const std::string& head) {
    std::string lower = cab::utils::toLower(head);
    std::string::size_type pos = lower.find("content-length:");
    if (pos == std::string::npos) return 0;
    std::string::size_type end = lower.find("\r\n", pos);
    int value = cab::utils::toInt(lower.substr(pos + 15, end - pos - 15), 0);
    return value > 0 ? static_cast<std::size_t>(value) : 0;
}

bool readRequest(socket_t client, std::string& raw) {
    const std::size_t MAX_REQUEST = 1024 * 1024;
    char buffer[4096];
    std::string::size_type headerEnd = std::string::npos;
    std::size_t expectedBody = 0;
    while (raw.size() < MAX_REQUEST) {
        int received = recv(client, buffer, sizeof(buffer), 0);
        if (received <= 0) break;
        raw.append(buffer, static_cast<std::size_t>(received));
        if (headerEnd == std::string::npos) {
            headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos) expectedBody = contentLength(raw.substr(0, headerEnd));
        }
        if (headerEnd != std::string::npos && raw.size() >= headerEnd + 4 + expectedBody) return true;
    }
    return headerEnd != std::string::npos;
}

void sendAll(socket_t client, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        int n = send(client, data.c_str() + sent, static_cast<int>(data.size() - sent), 0);
        if (n <= 0) return;
        sent += static_cast<std::size_t>(n);
    }
}

SocketRuntime& runtime() {
    static SocketRuntime instance;
    return instance;
}

}  // namespace

HttpServer::HttpServer(int port, const std::string& staticDirectory)
    : port(port), staticDirectory(staticDirectory), listenSocket(static_cast<long long>(BAD_SOCKET)) {
    runtime();
    socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == BAD_SOCKET) throw std::runtime_error("Could not create socket");

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        closeSocket(s);
        throw std::runtime_error("Port " + cab::utils::toString(port) + " is already in use");
    }
    if (listen(s, 16) != 0) {
        closeSocket(s);
        throw std::runtime_error("listen() failed");
    }
    listenSocket = static_cast<long long>(s);
}

HttpServer::~HttpServer() {
    if (static_cast<socket_t>(listenSocket) != BAD_SOCKET) closeSocket(static_cast<socket_t>(listenSocket));
}

void HttpServer::get(const std::string& path, Handler handler) { getRoutes[path] = handler; }

void HttpServer::post(const std::string& path, Handler handler) { postRoutes[path] = handler; }

void HttpServer::run() {
    socket_t server = static_cast<socket_t>(listenSocket);
    for (;;) {
        socket_t client = accept(server, nullptr, nullptr);
        if (client == BAD_SOCKET) continue;
        setReceiveTimeout(client, 3000);  // idle browser pre-connections must not block us

        std::string raw;
        if (readRequest(client, raw)) {
            HttpRequest request = HttpRequest::parse(raw);
            HttpResponse response = dispatch(request);
            if (request.getPath().compare(0, 8, "/static/") != 0) {
                std::cout << "[http] " << request.getMethod() << " " << request.getPath() << " -> "
                          << response.getStatus() << std::endl;
            }
            sendAll(client, response.serialize());
        }
        closeSocket(client);
    }
}

HttpResponse HttpServer::dispatch(const HttpRequest& request) const {
    try {
        const std::string& path = request.getPath();
        if (request.getMethod() == "GET") {
            if (path.compare(0, 8, "/static/") == 0) return serveStatic(path.substr(8));
            auto it = getRoutes.find(path);
            if (it != getRoutes.end()) return it->second(request);
        } else if (request.getMethod() == "POST") {
            auto it = postRoutes.find(path);
            if (it != postRoutes.end()) return it->second(request);
            if (getRoutes.count(path)) return HttpResponse::text("Method not allowed", 405);
        }
        auto notFound = getRoutes.find("/404");
        if (notFound != getRoutes.end()) return notFound->second(request);
        return HttpResponse::text("Not found", 404);
    } catch (const std::exception& e) {
        std::cerr << "[http] unhandled error: " << e.what() << std::endl;
        return HttpResponse::text(std::string("Internal server error: ") + e.what(), 500);
    }
}

HttpResponse HttpServer::serveStatic(const std::string& relativePath) const {
    if (relativePath.find("..") != std::string::npos) return HttpResponse::text("Forbidden", 404);
    std::ifstream file((staticDirectory + "/" + relativePath).c_str(), std::ios::binary);
    if (!file) return HttpResponse::text("Not found", 404);
    std::ostringstream content;
    content << file.rdbuf();
    return HttpResponse(200, content.str(), mimeType(relativePath));
}

std::string HttpServer::mimeType(const std::string& path) {
    std::string::size_type dot = path.rfind('.');
    std::string ext = dot == std::string::npos ? "" : cab::utils::toLower(path.substr(dot + 1));
    if (ext == "css") return "text/css; charset=utf-8";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";
    return "application/octet-stream";
}

}  // namespace web
