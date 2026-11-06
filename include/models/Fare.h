#ifndef CAB_MODELS_FARE_H
#define CAB_MODELS_FARE_H

#include <ostream>

namespace cab {

// A value object holding a full fare breakdown.
class Fare {
private:
    double baseFare;
    double distanceFare;
    double surgeCharge;
    double extraCharge;
    double tax;
    double discount;

public:
    static const double GST_RATE;  // STATIC DATA MEMBER shared by every Fare

    Fare();
    Fare(double base, double distance, double surge, double extra, double tax, double discount = 0.0);
    Fare(const Fare& other);
    Fare& operator=(const Fare& other);

    double getBaseFare() const { return baseFare; }
    double getDistanceFare() const { return distanceFare; }
    double getSurgeCharge() const { return surgeCharge; }
    double getExtraCharge() const { return extraCharge; }
    double getTax() const { return tax; }
    double getDiscount() const { return discount; }

    double subtotal() const;
    double total() const;
    void applyDiscount(double amount);

    // OPERATOR OVERLOADING: fares can be added together (used for revenue reports)
    Fare operator+(const Fare& other) const;
    Fare& operator+=(const Fare& other);
    bool operator<(const Fare& other) const { return total() < other.total(); }

    // FRIEND FUNCTION: prints the private breakdown directly.
    friend std::ostream& operator<<(std::ostream& out, const Fare& fare);
};

}  // namespace cab

#endif
