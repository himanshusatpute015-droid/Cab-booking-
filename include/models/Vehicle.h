#ifndef CAB_MODELS_VEHICLE_H
#define CAB_MODELS_VEHICLE_H

#include <string>

#include "models/Fare.h"

namespace cab {

/*
 * Vehicle hierarchy (HIERARCHICAL + MULTILEVEL INHERITANCE):
 *
 *                    Vehicle (abstract)
 *          ____________|_____________
 *         |            |             |
 *       Bike     AutoRickshaw    Car (abstract)
 *                          _________|_________
 *                         |         |         |
 *                       Mini      Sedan      SUV
 *                                   |
 *                                Premier (final)
 */
class Vehicle {
protected:
    std::string registrationNo;
    std::string model;
    int capacity;

private:
    static int liveCount;  // how many Vehicle objects currently exist

public:
    Vehicle(const std::string& registrationNo, const std::string& model, int capacity);
    virtual ~Vehicle();

    const std::string& getRegistrationNo() const { return registrationNo; }
    const std::string& getModel() const { return model; }
    int getCapacity() const { return capacity; }

    // PURE VIRTUAL FUNCTIONS - each concrete vehicle must provide its own pricing.
    virtual std::string getType() const = 0;
    virtual std::string icon() const = 0;
    virtual double baseFare() const = 0;
    virtual double perKmRate() const = 0;

    // VIRTUAL FUNCTIONS with default behaviour that subclasses may override.
    virtual double minimumFare() const;
    virtual double extraCharges(double km) const;
    virtual std::string description() const;

    // TEMPLATE METHOD pattern: a fixed algorithm that calls the virtual hooks above,
    // so the same call produces different results for each vehicle (RUNTIME POLYMORPHISM).
    Fare calculateFare(double km, double surgeMultiplier = 1.0) const;

    static int getLiveCount() { return liveCount; }
};

class Bike : public Vehicle {
public:
    Bike(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "Bike"; }
    std::string icon() const override { return "\xF0\x9F\x8F\x8D"; }  // motorbike emoji
    double baseFare() const override { return 20.0; }
    double perKmRate() const override { return 6.0; }
    std::string description() const override;
};

class AutoRickshaw : public Vehicle {
public:
    AutoRickshaw(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "Auto"; }
    std::string icon() const override { return "\xF0\x9F\x9B\xBA"; }  // auto rickshaw emoji
    double baseFare() const override { return 30.0; }
    double perKmRate() const override { return 10.0; }
    std::string description() const override;
};

// Car is still abstract: it adds AC behaviour but does not define prices.
class Car : public Vehicle {
protected:
    bool airConditioned;

public:
    Car(const std::string& registrationNo, const std::string& model, int capacity, bool airConditioned = true);
    bool isAirConditioned() const { return airConditioned; }
    double extraCharges(double km) const override;  // AC surcharge
    std::string description() const override;
};

class Mini : public Car {
public:
    Mini(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "Mini"; }
    std::string icon() const override { return "\xF0\x9F\x9A\x97"; }  // car emoji
    double baseFare() const override { return 50.0; }
    double perKmRate() const override { return 12.0; }
};

class Sedan : public Car {
public:
    Sedan(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "Sedan"; }
    std::string icon() const override { return "\xF0\x9F\x9A\x98"; }  // oncoming car emoji
    double baseFare() const override { return 70.0; }
    double perKmRate() const override { return 15.0; }
};

class SUV : public Car {
public:
    SUV(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "SUV"; }
    std::string icon() const override { return "\xF0\x9F\x9A\x99"; }  // SUV emoji
    double baseFare() const override { return 100.0; }
    double perKmRate() const override { return 20.0; }
    double extraCharges(double km) const override;  // AC + large-vehicle fee
    std::string description() const override;
};

// Third level of inheritance. `final` prevents any further derivation.
class Premier final : public Sedan {
public:
    Premier(const std::string& registrationNo, const std::string& model);
    std::string getType() const override { return "Premier"; }
    std::string icon() const override { return "\xF0\x9F\x9A\x96"; }  // oncoming taxi emoji
    double baseFare() const override { return 120.0; }
    double perKmRate() const override { return 22.0; }
    double extraCharges(double km) const override;  // reuses Sedan's logic, adds service fee
    std::string description() const override;
};

}  // namespace cab

#endif
