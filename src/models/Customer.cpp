#include "models/Customer.h"

#include "core/Utils.h"

namespace cab {

int Customer::nextId = 1;

Customer::Customer(const std::string& name, const std::string& phone)
    : Customer(nextId, name, phone, 0, 0.0) {}  // DELEGATING CONSTRUCTOR

Customer::Customer(int id, const std::string& name, const std::string& phone, int totalRides, double totalSpent)
    : Person(id, name, phone), totalRides(totalRides), totalSpent(totalSpent) {
    ensureNextIdAbove(id);
}

std::string Customer::profile() const {
    return Person::profile() + " - " + tier() + " member, " + utils::toString(totalRides) + " rides";
}

std::string Customer::tier() const {
    if (totalRides >= 10) return "Gold";
    if (totalRides >= 3) return "Silver";
    return "Bronze";
}

void Customer::recordRide(double amount) {
    ++totalRides;
    totalSpent += amount;
}

void Customer::ensureNextIdAbove(int usedId) {
    if (usedId >= nextId) nextId = usedId + 1;
}

}  // namespace cab
