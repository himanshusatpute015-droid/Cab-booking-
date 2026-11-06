#ifndef CAB_MODELS_PERSON_H
#define CAB_MODELS_PERSON_H

#include <string>

namespace cab {

// ABSTRACTION: Person is an abstract base class (it has a pure virtual function),
// so it can never be instantiated directly - only Customer and Driver can.
class Person {
protected:  // accessible to derived classes, hidden from everyone else
    int id;
    std::string name;
    std::string phone;

public:
    Person(int id, const std::string& name, const std::string& phone);
    virtual ~Person();  // virtual destructor -> safe deletion through base pointers

    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getPhone() const { return phone; }

    // Setters validate their input (encapsulation protects the invariants).
    void setName(const std::string& newName);
    void setPhone(const std::string& newPhone);

    virtual std::string getRole() const = 0;  // PURE VIRTUAL FUNCTION
    virtual std::string profile() const;      // virtual, may be overridden

    std::string initials() const;

    static std::string validateName(const std::string& name);    // static member functions
    static std::string validatePhone(const std::string& phone);
};

}  // namespace cab

#endif
