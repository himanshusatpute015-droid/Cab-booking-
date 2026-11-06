#include "models/Fare.h"

#include <iomanip>

namespace cab {

const double Fare::GST_RATE = 0.05;

Fare::Fare() : baseFare(0), distanceFare(0), surgeCharge(0), extraCharge(0), tax(0), discount(0) {}

Fare::Fare(double base, double distance, double surge, double extra, double tax, double discount)
    : baseFare(base), distanceFare(distance), surgeCharge(surge), extraCharge(extra), tax(tax), discount(0) {
    applyDiscount(discount);
}

Fare::Fare(const Fare& other)
    : baseFare(other.baseFare),
      distanceFare(other.distanceFare),
      surgeCharge(other.surgeCharge),
      extraCharge(other.extraCharge),
      tax(other.tax),
      discount(other.discount) {}

Fare& Fare::operator=(const Fare& other) {
    if (this != &other) {
        baseFare = other.baseFare;
        distanceFare = other.distanceFare;
        surgeCharge = other.surgeCharge;
        extraCharge = other.extraCharge;
        tax = other.tax;
        discount = other.discount;
    }
    return *this;
}

double Fare::subtotal() const { return baseFare + distanceFare + surgeCharge + extraCharge; }

double Fare::total() const {
    double value = subtotal() + tax - discount;
    return value < 0 ? 0 : value;
}

void Fare::applyDiscount(double amount) {
    double maxAllowed = subtotal() + tax;
    if (amount < 0) amount = 0;
    if (amount > maxAllowed) amount = maxAllowed;
    discount = amount;
}

Fare Fare::operator+(const Fare& other) const {
    return Fare(baseFare + other.baseFare, distanceFare + other.distanceFare, surgeCharge + other.surgeCharge,
                extraCharge + other.extraCharge, tax + other.tax, discount + other.discount);
}

Fare& Fare::operator+=(const Fare& other) {
    *this = *this + other;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Fare& fare) {
    out << std::fixed << std::setprecision(2) << "Base " << fare.baseFare << " + Distance " << fare.distanceFare
        << " + Surge " << fare.surgeCharge << " + Extra " << fare.extraCharge << " + GST " << fare.tax
        << " - Discount " << fare.discount << " = " << fare.total();
    return out;
}

}  // namespace cab
