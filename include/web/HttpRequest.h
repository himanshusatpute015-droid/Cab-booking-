#ifndef CAB_WEB_HTTP_REQUEST_H
#define CAB_WEB_HTTP_REQUEST_H

#include <map>
#include <string>

namespace web {

class HttpRequest {
private:
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query;
    std::map<std::string, std::string> form;
    std::string body;

    static std::map<std::string, std::string> parseUrlEncoded(const std::string& text);

public:
    static HttpRequest parse(const std::string& raw);  // static factory method

    const std::string& getMethod() const { return method; }
    const std::string& getPath() const { return path; }
    const std::string& getBody() const { return body; }
    std::string header(const std::string& name) const;

    // Looks in the POST form first, then the query string.
    std::string param(const std::string& name, const std::string& fallback = "") const;
};

}  // namespace web

#endif
