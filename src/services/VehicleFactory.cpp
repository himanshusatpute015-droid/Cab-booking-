#include "services/VehicleFactory.h"

#include "core/Exceptions.h"

namespace cab {

std::shared_ptr<Vehicle> VehicleFactory::create(const std::string& type, const std::string& registrationNo,
                                                const std::string& model) {
    if (type == "Bike") return std::make_shared<Bike>(registrationNo, model);
    if (type == "Auto") return std::make_shared<AutoRickshaw>(registrationNo, model);
    if (type == "Mini") return std::make_shared<Mini>(registrationNo, model);
    if (type == "Sedan") return std::make_shared<Sedan>(registrationNo, model);
    if (type == "SUV") return std::make_shared<SUV>(registrationNo, model);
    if (type == "Premier") return std::make_shared<Premier>(registrationNo, model);
    throw ValidationException("Unknown vehicle type: " + type);
}

const std::vector<std::string>& VehicleFactory::types() {
    static const std::vector<std::string> list = {"Bike", "Auto", "Mini", "Sedan", "SUV", "Premier"};
    return list;
}

bool VehicleFactory::isValidType(const std::string& type) {
    for (const std::string& t : types()) {
        if (t == type) return true;
    }
    return false;
}

std::vector<std::shared_ptr<Vehicle>> VehicleFactory::catalogue() {
    std::vector<std::shared_ptr<Vehicle>> list;
    for (const std::string& type : types()) list.push_back(create(type, "SAMPLE", "Sample " + type));
    return list;
}

}  // namespace cab
