#ifndef CAB_WEB_HTTP_SERVER_H
#define CAB_WEB_HTTP_SERVER_H

#include <functional>
#include <map>
#include <string>

#include "web/HttpRequest.h"
#include "web/HttpResponse.h"

namespace web {

// A tiny HTTP/1.1 server written directly on top of OS sockets (WinSock / BSD sockets).
// RAII: the constructor opens the listening socket and the destructor closes it.
class HttpServer {
public:
    typedef std::function<HttpResponse(const HttpRequest&)> Handler;

    HttpServer(int port, const std::string& staticDirectory);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void run();

private:
    int port;
    std::string staticDirectory;
    long long listenSocket;  // stored as an integer so this header needs no OS includes
    std::map<std::string, Handler> getRoutes;
    std::map<std::string, Handler> postRoutes;

    HttpResponse dispatch(const HttpRequest& request) const;
    HttpResponse serveStatic(const std::string& path) const;
    static std::string mimeType(const std::string& path);
};

}  // namespace web

#endif
