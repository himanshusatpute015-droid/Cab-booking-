#include "models/Vehicle.h"

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

int Vehicle::liveCount = 0;

// ---------------------------------------------------------------- Vehicle
Vehicle::Vehicle(const std::string& registrationNo, const std::string& model, int capacity)
    : registrationNo(utils::toUpper(utils::sanitizeField(registrationNo))),
      model(utils::sanitizeField(model)),
      capacity(capacity) {
    if (this->registrationNo.size() < 4) throw ValidationException("Registration number is too short.");
    if (this->model.empty()) throw ValidationException("Vehicle model is required.");
    ++liveCount;
}

Vehicle::~Vehicle() { --liveCount; }

double Vehicle::minimumFare() const { return baseFare() + 2 * perKmRate(); }  // first 2 km

double Vehicle::extraCharges(double) const { return 0.0; }

std::string Vehicle::description() const { return "Comfortable ride for up to " + utils::toString(capacity) + "."; }

Fare Vehicle::calculateFare(double km, double surgeMultiplier) const {
    if (km < 0) throw ValidationException("Distance cannot be negative.");
    if (surgeMultiplier < 1.0) surgeMultiplier = 1.0;

    double base = baseFare();                       // virtual call
    double distance = km * perKmRate();             // virtual call
    if (base + distance < minimumFare()) {          // virtual call
        distance = minimumFare() - base;
    }
    double surge = (base + distance) * (surgeMultiplier - 1.0);
    double extra = extraCharges(km);                // virtual call
    double tax = (base + distance + surge + extra) * Fare::GST_RATE;
    return Fare(base, distance, surge, extra, tax);
}

// ---------------------------------------------------------------- Bike
Bike::Bike(const std::string& registrationNo, const std::string& model) : Vehicle(registrationNo, model, 1) {}

std::string Bike::description() const { return "Beat the traffic - quick and cheap solo rides."; }

// ---------------------------------------------------------------- AutoRickshaw
AutoRickshaw::AutoRickshaw(const std::string& registrationNo, const std::string& model)
    : Vehicle(registrationNo, model, 3) {}

std::string AutoRickshaw::description() const { return "Classic three-wheeler for short city hops."; }

// ---------------------------------------------------------------- Car
Car::Car(const std::string& registrationNo, const std::string& model, int capacity, bool airConditioned)
    : Vehicle(registrationNo, model, capacity), airConditioned(airConditioned) {}

double Car::extraCharges(double km) const { return airConditioned ? km * 1.0 : 0.0; }

std::string Car::description() const {
    return std::string(airConditioned ? "AC" : "Non-AC") + " car for up to " + utils::toString(capacity) + " riders.";
}

// ---------------------------------------------------------------- Mini / Sedan / SUV
Mini::Mini(const std::string& registrationNo, const std::string& model) : Car(registrationNo, model, 4) {}

Sedan::Sedan(const std::string& registrationNo, const std::string& model) : Car(registrationNo, model, 4) {}

SUV::SUV(const std::string& registrationNo, const std::string& model) : Car(registrationNo, model, 6) {}

double SUV::extraCharges(double km) const { return Car::extraCharges(km) + 30.0; }

std::string SUV::description() const { return "Spacious AC ride for groups of up to 6 with extra luggage room."; }

// ---------------------------------------------------------------- Premier
Premier::Premier(const std::string& registrationNo, const std::string& model) : Sedan(registrationNo, model) {}

double Premier::extraCharges(double km) const { return Sedan::extraCharges(km) + 50.0; }

std::string Premier::description() const { return "Top-rated drivers, premium sedans and a complimentary water bottle."; }

}  // namespace cab
