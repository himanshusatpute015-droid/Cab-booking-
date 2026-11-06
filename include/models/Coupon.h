#ifndef CAB_MODELS_COUPON_H
#define CAB_MODELS_COUPON_H

#include <string>

#include "models/Customer.h"

namespace cab {

// Abstract promo-code class. Each subclass computes its discount differently (POLYMORPHISM).
class Coupon {
protected:
    std::string code;
    std::string description;

public:
    Coupon(const std::string& code, const std::string& description);
    virtual ~Coupon() {}

    const std::string& getCode() const { return code; }
    const std::string& getDescription() const { return description; }

    // Throws ValidationException when the coupon cannot be used.
    virtual double discountFor(double fareTotal, const Customer& customer) const = 0;
};

class FlatCoupon : public Coupon {
private:
    double amount;
    double minimumFare;

public:
    FlatCoupon(const std::string& code, double amount, double minimumFare);
    double discountFor(double fareTotal, const Customer& customer) const override;
};

class PercentCoupon : public Coupon {
private:
    double percent;
    double maxDiscount;
    bool firstRideOnly;

public:
    PercentCoupon(const std::string& code, double percent, double maxDiscount, bool firstRideOnly = false);
    double discountFor(double fareTotal, const Customer& customer) const override;
};

class LoyaltyCoupon : public Coupon {
private:
    int minimumRides;
    double percent;

public:
    LoyaltyCoupon(const std::string& code, int minimumRides, double percent);
    double discountFor(double fareTotal, const Customer& customer) const override;
};

}  // namespace cab

#endif
