#include "models/Location.h"

#include <cmath>

namespace cab {

Location::Location() : name("Unknown"), x(0.0), y(0.0) {}

Location::Location(const std::string& name, double x, double y) : name(name), x(x), y(y) {}

Location::Location(const Location& other) : name(other.name), x(other.x), y(other.y) {}

Location& Location::operator=(const Location& other) {
    if (this != &other) {  // `this` pointer guards against self-assignment
        name = other.name;
        x = other.x;
        y = other.y;
    }
    return *this;
}

Location::~Location() {}

double Location::distanceTo(const Location& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    // Road distance is roughly 1.3x the straight-line distance.
    return std::sqrt(dx * dx + dy * dy) * 1.3;
}

bool Location::operator==(const Location& other) const { return name == other.name; }

std::ostream& operator<<(std::ostream& out, const Location& location) {
    out << location.name << " (" << location.x << ", " << location.y << ")";
    return out;
}

}  // namespace cab
