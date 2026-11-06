#include "models/Coupon.h"

#include <algorithm>

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

Coupon::Coupon(const std::string& code, const std::string& description) : code(code), description(description) {}

// ---------------------------------------------------------------- FlatCoupon
FlatCoupon::FlatCoupon(const std::string& code, double amount, double minimumFare)
    : Coupon(code, "Flat Rs " + utils::fixed(amount, 0) + " off on rides above Rs " + utils::fixed(minimumFare, 0)),
      amount(amount),
      minimumFare(minimumFare) {}

double FlatCoupon::discountFor(double fareTotal, const Customer&) const {
    if (fareTotal < minimumFare) {
        throw ValidationException(code + " needs a minimum fare of Rs " + utils::fixed(minimumFare, 0) + ".");
    }
    return amount;
}

// ---------------------------------------------------------------- PercentCoupon
PercentCoupon::PercentCoupon(const std::string& code, double percent, double maxDiscount, bool firstRideOnly)
    : Coupon(code, utils::fixed(percent, 0) + "% off up to Rs " + utils::fixed(maxDiscount, 0) +
                       (firstRideOnly ? " on your first ride" : " on any ride")),
      percent(percent),
      maxDiscount(maxDiscount),
      firstRideOnly(firstRideOnly) {}

double PercentCoupon::discountFor(double fareTotal, const Customer& customer) const {
    if (firstRideOnly && !customer.isFirstRide()) {
        throw ValidationException(code + " is valid only on your first ride.");
    }
    return std::min(fareTotal * percent / 100.0, maxDiscount);
}

// ---------------------------------------------------------------- LoyaltyCoupon
LoyaltyCoupon::LoyaltyCoupon(const std::string& code, int minimumRides, double percent)
    : Coupon(code, utils::fixed(percent, 0) + "% off for riders with " + utils::toString(minimumRides) +
                       "+ completed rides"),
      minimumRides(minimumRides),
      percent(percent) {}

double LoyaltyCoupon::discountFor(double fareTotal, const Customer& customer) const {
    if (customer.getTotalRides() < minimumRides) {
        throw ValidationException(code + " unlocks after " + utils::toString(minimumRides) + " completed rides.");
    }
    return fareTotal * percent / 100.0;
}

}  // namespace cab
