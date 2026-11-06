#ifndef CAB_MODELS_LOCATION_H
#define CAB_MODELS_LOCATION_H

#include <ostream>
#include <string>

namespace cab {

// ENCAPSULATION: coordinates are private and exposed only through const getters.
class Location {
private:
    std::string name;
    double x;  // kilometres east of the city centre
    double y;  // kilometres north of the city centre

public:
    Location();                                                // default constructor
    Location(const std::string& name, double x, double y);    // parameterised constructor
    Location(const Location& other);                           // copy constructor
    Location& operator=(const Location& other);                // copy assignment
    ~Location();                                               // destructor

    const std::string& getName() const { return name; }       // inline member functions
    double getX() const { return x; }
    double getY() const { return y; }

    double distanceTo(const Location& other) const;

    // OPERATOR OVERLOADING: `a - b` gives the distance between two locations in km.
    double operator-(const Location& other) const { return distanceTo(other); }
    bool operator==(const Location& other) const;
    bool operator!=(const Location& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& out, const Location& location);
};

}  // namespace cab

#endif
