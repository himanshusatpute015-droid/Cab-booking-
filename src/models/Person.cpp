#include "models/Person.h"

#include <cctype>

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

Person::Person(int id, const std::string& name, const std::string& phone) : id(id) {
    setName(name);
    setPhone(phone);
}

Person::~Person() {}

void Person::setName(const std::string& newName) { name = validateName(newName); }

void Person::setPhone(const std::string& newPhone) { phone = validatePhone(newPhone); }

std::string Person::profile() const { return getRole() + ": " + name + " (" + phone + ")"; }

std::string Person::initials() const {
    std::string result;
    bool takeNext = true;
    for (char c : name) {
        if (c == ' ') {
            takeNext = true;
        } else if (takeNext && result.size() < 2) {
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            takeNext = false;
        }
    }
    return result.empty() ? "?" : result;
}

std::string Person::validateName(const std::string& rawName) {
    std::string clean = utils::sanitizeField(rawName);
    if (clean.size() < 2) throw ValidationException("Name must have at least 2 characters.");
    if (clean.size() > 40) throw ValidationException("Name must be at most 40 characters.");
    for (char c : clean) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '.') {
            throw ValidationException("Name may only contain letters, spaces and dots.");
        }
    }
    return clean;
}

std::string Person::validatePhone(const std::string& rawPhone) {
    std::string clean = utils::trim(rawPhone);
    if (clean.size() != 10 || !utils::isDigits(clean)) {
        throw ValidationException("Phone number must be exactly 10 digits.");
    }
    if (clean[0] < '6') throw ValidationException("Phone number must start with 6, 7, 8 or 9.");
    return clean;
}

}  // namespace cab
