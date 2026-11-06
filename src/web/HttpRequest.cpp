#include "web/HttpRequest.h"

#include <sstream>

#include "core/Utils.h"

namespace web {

using cab::utils::toLower;
using cab::utils::trim;
using cab::utils::urlDecode;

std::map<std::string, std::string> HttpRequest::parseUrlEncoded(const std::string& text) {
    std::map<std::string, std::string> values;
    for (const std::string& pair : cab::utils::split(text, '&')) {
        if (pair.empty()) continue;
        std::string::size_type eq = pair.find('=');
        if (eq == std::string::npos) {
            values[urlDecode(pair)] = "";
        } else {
            values[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
        }
    }
    return values;
}

HttpRequest HttpRequest::parse(const std::string& raw) {
    HttpRequest request;
    std::string::size_type headerEnd = raw.find("\r\n\r\n");
    std::string head = raw.substr(0, headerEnd);
    request.body = (headerEnd == std::string::npos) ? "" : raw.substr(headerEnd + 4);

    std::istringstream lines(head);
    std::string requestLine;
    std::getline(lines, requestLine);
    std::istringstream first(trim(requestLine));
    std::string target, version;
    first >> request.method >> target >> version;

    std::string::size_type q = target.find('?');
    request.path = urlDecode(target.substr(0, q));
    if (q != std::string::npos) request.query = parseUrlEncoded(target.substr(q + 1));
    if (request.path.empty()) request.path = "/";

    std::string line;
    while (std::getline(lines, line)) {
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) continue;
        request.headers[toLower(trim(line.substr(0, colon)))] = trim(line.substr(colon + 1));
    }

    if (request.method == "POST") request.form = parseUrlEncoded(request.body);
    return request;
}

std::string HttpRequest::header(const std::string& name) const {
    auto it = headers.find(toLower(name));
    return it == headers.end() ? "" : it->second;
}

std::string HttpRequest::param(const std::string& name, const std::string& fallback) const {
    auto it = form.find(name);
    if (it != form.end()) return it->second;
    it = query.find(name);
    if (it != query.end()) return it->second;
    return fallback;
}

}  // namespace web
