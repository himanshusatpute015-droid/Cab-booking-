#ifndef CAB_SERVICES_VEHICLE_FACTORY_H
#define CAB_SERVICES_VEHICLE_FACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "models/Vehicle.h"

namespace cab {

// FACTORY pattern: creates the right Vehicle subclass and returns it as a base-class
// pointer, so callers never depend on concrete vehicle types.
class VehicleFactory {
public:
    VehicleFactory() = delete;

    static std::shared_ptr<Vehicle> create(const std::string& type, const std::string& registrationNo,
                                           const std::string& model);
    static const std::vector<std::string>& types();
    static bool isValidType(const std::string& type);

    // One sample object of every vehicle type - used to show pricing and estimates.
    static std::vector<std::shared_ptr<Vehicle>> catalogue();
};

}  // namespace cab

#endif
