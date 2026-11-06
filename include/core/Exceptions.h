#ifndef CAB_CORE_EXCEPTIONS_H
#define CAB_CORE_EXCEPTIONS_H

#include <exception>
#include <string>

namespace cab {

// EXCEPTION HANDLING + INHERITANCE: a custom exception hierarchy rooted at std::exception.
class CabException : public std::exception {
protected:
    std::string message;

public:
    explicit CabException(const std::string& msg) : message(msg) {}
    virtual ~CabException() noexcept {}

    const char* what() const noexcept override { return message.c_str(); }

    // Runtime polymorphism: each subclass reports its own category and HTTP status.
    virtual std::string category() const { return "Something went wrong"; }
    virtual int httpStatus() const { return 400; }
};

class ValidationException : public CabException {
public:
    explicit ValidationException(const std::string& msg) : CabException(msg) {}
    std::string category() const override { return "Invalid input"; }
};

class NotFoundException : public CabException {
public:
    explicit NotFoundException(const std::string& msg) : CabException(msg) {}
    std::string category() const override { return "Not found"; }
    int httpStatus() const override { return 404; }
};

class NoDriverAvailableException : public CabException {
public:
    explicit NoDriverAvailableException(const std::string& vehicleType)
        : CabException("No " + vehicleType + " driver is available right now. Please try another cab type.") {}
    std::string category() const override { return "No drivers nearby"; }
    int httpStatus() const override { return 409; }
};

class InvalidStateException : public CabException {
public:
    explicit InvalidStateException(const std::string& msg) : CabException(msg) {}
    std::string category() const override { return "Action not allowed"; }
    int httpStatus() const override { return 409; }
};

class PaymentException : public CabException {
public:
    explicit PaymentException(const std::string& msg) : CabException(msg) {}
    std::string category() const override { return "Payment failed"; }
    int httpStatus() const override { return 402; }
};

}  // namespace cab

#endif
