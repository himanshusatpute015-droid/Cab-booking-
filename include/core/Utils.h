#ifndef CAB_CORE_UTILS_H
#define CAB_CORE_UTILS_H

#include <sstream>
#include <string>
#include <vector>

namespace cab {
namespace utils {

// FUNCTION TEMPLATE (generic programming): converts any streamable type to string.
template <typename T>
std::string toString(const T& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

std::string fixed(double value, int precision = 2);          // default argument
int toInt(const std::string& text, int fallback = 0);
double toDouble(const std::string& text, double fallback = 0.0);

std::string trim(const std::string& text);
std::string toLower(std::string text);
std::string toUpper(std::string text);
std::vector<std::string> split(const std::string& text, char delimiter);
bool isDigits(const std::string& text);

std::string htmlEscape(const std::string& text);
std::string urlDecode(const std::string& text);
std::string sanitizeField(const std::string& text);  // strips characters used by file storage

std::string currentTimestamp();
std::string stars(double rating);

}  // namespace utils
}  // namespace cab

#endif
