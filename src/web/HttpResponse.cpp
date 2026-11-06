#include "web/HttpResponse.h"

#include "core/Utils.h"

namespace web {

HttpResponse::HttpResponse(int status, const std::string& body, const std::string& contentType)
    : status(status), contentType(contentType), body(body) {}

HttpResponse HttpResponse::html(const std::string& body, int status) { return HttpResponse(status, body); }

HttpResponse HttpResponse::text(const std::string& body, int status) {
    return HttpResponse(status, body, "text/plain; charset=utf-8");
}

HttpResponse HttpResponse::redirect(const std::string& location) {
    HttpResponse response(303, "");
    response.setHeader("Location", location);
    return response;
}

std::string HttpResponse::reasonPhrase(int status) {
    switch (status) {
        case 200: return "OK";
        case 303: return "See Other";
        case 400: return "Bad Request";
        case 402: return "Payment Required";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        default: return "Internal Server Error";
    }
}

std::string HttpResponse::serialize() const {
    std::string out = "HTTP/1.1 " + cab::utils::toString(status) + " " + reasonPhrase(status) + "\r\n";
    out += "Content-Type: " + contentType + "\r\n";
    out += "Content-Length: " + cab::utils::toString(body.size()) + "\r\n";
    out += "Connection: close\r\n";
    out += "Cache-Control: no-store\r\n";
    for (const auto& h : headers) out += h.first + ": " + h.second + "\r\n";
    out += "\r\n";
    out += body;
    return out;
}

}  // namespace web
