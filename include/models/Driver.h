#ifndef CAB_MODELS_DRIVER_H
#define CAB_MODELS_DRIVER_H

#include <memory>
#include <string>

#include "models/Location.h"
#include "models/Person.h"
#include "models/Rateable.h"
#include "models/Vehicle.h"

namespace cab {

// MULTIPLE INHERITANCE: a Driver is a Person AND is Rateable.
// COMPOSITION: a Driver owns a Vehicle (held polymorphically via a base-class pointer)
// and has a current Location.
class Driver : public Person, public Rateable {
private:
    std::string licenseNo;
    std::shared_ptr<Vehicle> vehicle;
    Location location;
    bool online;
    bool onRide;
    double earnings;
    int completedRides;

    static int nextId;
    friend class FileStorage;

public:
    static const double DRIVER_SHARE;  // fraction of each fare the driver keeps

    Driver(const std::string& name, const std::string& phone, const std::string& licenseNo,
           const std::shared_ptr<Vehicle>& vehicle, const Location& location);
    Driver(int id, const std::string& name, const std::string& phone, const std::string& licenseNo,
           const std::shared_ptr<Vehicle>& vehicle, const Location& location);

    std::string getRole() const override { return "Driver"; }
    std::string profile() const override;
    std::string ratingSummary() const override;  // overrides Rateable's version

    const std::string& getLicenseNo() const { return licenseNo; }
    std::shared_ptr<Vehicle> getVehicle() const { return vehicle; }
    const Location& getLocation() const { return location; }
    double getEarnings() const { return earnings; }
    int getCompletedRides() const { return completedRides; }
    bool isOnline() const { return online; }
    bool isOnRide() const { return onRide; }
    bool isAvailable() const { return online && !onRide; }
    std::string statusLabel() const;

    double distanceTo(const Location& point) const { return location - point; }

    void toggleOnline();
    void startRide();
    void finishRide(const Location& dropPoint, double fareAmount);
    void releaseFromRide();
    void resumeRide() { onRide = true; }  // used when restoring an open ride from disk

    static void ensureNextIdAbove(int usedId);
    static std::string validateLicense(const std::string& licenseNo);
};

}  // namespace cab

#endif
