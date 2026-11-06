#include "models/Driver.h"

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

int Driver::nextId = 1;
const double Driver::DRIVER_SHARE = 0.80;

Driver::Driver(const std::string& name, const std::string& phone, const std::string& licenseNo,
               const std::shared_ptr<Vehicle>& vehicle, const Location& location)
    : Driver(nextId, name, phone, licenseNo, vehicle, location) {}  // DELEGATING CONSTRUCTOR

Driver::Driver(int id, const std::string& name, const std::string& phone, const std::string& licenseNo,
               const std::shared_ptr<Vehicle>& vehicle, const Location& location)
    : Person(id, name, phone),
      Rateable(),
      licenseNo(validateLicense(licenseNo)),
      vehicle(vehicle),
      location(location),
      online(true),
      onRide(false),
      earnings(0.0),
      completedRides(0) {
    if (!vehicle) throw ValidationException("A driver must have a vehicle.");
    ensureNextIdAbove(id);
}

std::string Driver::profile() const {
    return Person::profile() + " - drives " + vehicle->getType() + " " + vehicle->getModel();
}

std::string Driver::ratingSummary() const {
    std::string base = Rateable::ratingSummary();
    if (ratingCount >= 3 && averageRating() >= 4.5) base += " - Top rated";
    return base;
}

std::string Driver::statusLabel() const {
    if (!online) return "Offline";
    return onRide ? "On a ride" : "Available";
}

void Driver::toggleOnline() {
    if (onRide) throw InvalidStateException(name + " is on a ride and cannot go offline right now.");
    online = !online;
}

void Driver::startRide() {
    if (!isAvailable()) throw InvalidStateException(name + " is not available for a new ride.");
    onRide = true;
}

void Driver::finishRide(const Location& dropPoint, double fareAmount) {
    onRide = false;
    location = dropPoint;  // the driver ends up where the rider was dropped
    earnings += fareAmount * DRIVER_SHARE;
    ++completedRides;
}

void Driver::releaseFromRide() { onRide = false; }

void Driver::ensureNextIdAbove(int usedId) {
    if (usedId >= nextId) nextId = usedId + 1;
}

std::string Driver::validateLicense(const std::string& raw) {
    std::string clean = utils::toUpper(utils::sanitizeField(raw));
    if (clean.size() < 6) throw ValidationException("Driving licence number must have at least 6 characters.");
    return clean;
}

}  // namespace cab
