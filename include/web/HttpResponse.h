#ifndef CAB_WEB_HTTP_RESPONSE_H
#define CAB_WEB_HTTP_RESPONSE_H

#include <map>
#include <string>

namespace web {

class HttpResponse {
private:
    int status;
    std::string contentType;
    std::string body;
    std::map<std::string, std::string> headers;

public:
    explicit HttpResponse(int status = 200, const std::string& body = "",
                          const std::string& contentType = "text/html; charset=utf-8");

    // Named constructors (static factory functions)
    static HttpResponse html(const std::string& body, int status = 200);
    static HttpResponse text(const std::string& body, int status = 200);
    static HttpResponse redirect(const std::string& location);

    int getStatus() const { return status; }
    void setHeader(const std::string& name, const std::string& value) { headers[name] = value; }
    std::string serialize() const;

    static std::string reasonPhrase(int status);
};

}  // namespace web

#endif
