#include "core/Utils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>

namespace cab {
namespace utils {

std::string fixed(double value, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

int toInt(const std::string& text, int fallback) {
    std::string t = trim(text);
    if (t.empty()) return fallback;
    char* end = nullptr;
    long value = std::strtol(t.c_str(), &end, 10);
    if (end == nullptr || *end != '\0') return fallback;
    return static_cast<int>(value);
}

double toDouble(const std::string& text, double fallback) {
    std::string t = trim(text);
    if (t.empty()) return fallback;
    char* end = nullptr;
    double value = std::strtod(t.c_str(), &end);
    if (end == nullptr || *end != '\0') return fallback;
    return value;
}

std::string trim(const std::string& text) {
    const char* ws = " \t\r\n";
    std::string::size_type start = text.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::string::size_type end = text.find_last_not_of(ws);
    return text.substr(start, end - start + 1);
}

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string toUpper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

bool isDigits(const std::string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

std::string htmlEscape(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c;
        }
    }
    return out;
}

static int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::string urlDecode(const std::string& text) {
    std::string out;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '+') {
            out += ' ';
        } else if (c == '%' && i + 2 < text.size()) {
            int hi = hexValue(text[i + 1]);
            int lo = hexValue(text[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += static_cast<char>(hi * 16 + lo);
                i += 2;
            } else {
                out += c;
            }
        } else {
            out += c;
        }
    }
    return out;
}

std::string sanitizeField(const std::string& text) {
    std::string out;
    for (char c : text) {
        if (c == '|' || c == '\n' || c == '\r') continue;
        out += c;
    }
    return trim(out);
}

std::string currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%d %b %Y, %I:%M %p", local);
    return buffer;
}

std::string stars(double rating) {
    int full = static_cast<int>(rating + 0.5);
    if (full < 0) full = 0;
    if (full > 5) full = 5;
    std::string out;
    for (int i = 0; i < 5; ++i) out += (i < full) ? "\xE2\x98\x85" : "\xE2\x98\x86";  // ★ / ☆
    return out;
}

}  // namespace utils
}  // namespace cab
