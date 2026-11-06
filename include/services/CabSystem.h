#ifndef CAB_SERVICES_CAB_SYSTEM_H
#define CAB_SERVICES_CAB_SYSTEM_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core/Repository.h"
#include "models/Booking.h"
#include "models/Coupon.h"
#include "models/Customer.h"
#include "models/Driver.h"
#include "models/Location.h"
#include "services/FileStorage.h"

namespace cab {

struct BookingRequest {
    std::string riderName;
    std::string riderPhone;
    std::string pickup;
    std::string drop;
    std::string vehicleType;
    std::string paymentMethod;
    std::string promoCode;
};

struct FareEstimate {
    std::shared_ptr<Vehicle> vehicle;  // sample vehicle of this type
    Fare fare;
    int availableDrivers;
    int etaMinutes;
};

struct TypeReport {
    int rides;
    double revenue;
    TypeReport() : rides(0), revenue(0) {}
};

struct SystemStats {
    int totalBookings, activeRides, completedRides, cancelledRides;
    int totalDrivers, availableDrivers, totalCustomers;
    double revenue, driverPayouts, averageRating, surge;
};

// SINGLETON: exactly one CabSystem exists. It is the facade the web layer talks to,
// and it AGGREGATES the repositories, locations, coupons and storage.
class CabSystem {
private:
    Repository<Customer> customers;
    Repository<Driver> drivers;
    Repository<Booking> bookings;
    std::vector<Location> locations;
    std::vector<std::unique_ptr<Coupon>> coupons;
    std::unique_ptr<FileStorage> storage;

    static const double AVERAGE_SPEED_KMPH;

    CabSystem();  // private constructor: use instance()

    void seedLocations();
    void seedCoupons();
    void seedDrivers();
    void persist() const;

    std::shared_ptr<Customer> findOrCreateCustomer(const std::string& name, const std::string& phone);
    std::shared_ptr<Driver> nearestDriver(const std::string& vehicleType, const Location& point) const;
    const Coupon& findCoupon(const std::string& code) const;
    static int etaFor(double km);

public:
    CabSystem(const CabSystem&) = delete;             // non-copyable
    CabSystem& operator=(const CabSystem&) = delete;

    static CabSystem& instance();

    void init(const std::string& dataDirectory);

    // Queries
    const std::vector<Location>& getLocations() const { return locations; }
    const Location& findLocation(const std::string& name) const;
    const std::vector<std::unique_ptr<Coupon>>& getCoupons() const { return coupons; }
    double currentSurge() const;
    double distanceBetween(const std::string& pickup, const std::string& drop) const;
    std::vector<FareEstimate> estimateFares(const std::string& pickup, const std::string& drop) const;

    std::shared_ptr<Booking> getBooking(int id) const;
    std::shared_ptr<Driver> getDriver(int id) const;
    std::shared_ptr<Customer> getCustomer(int id) const;
    std::vector<std::shared_ptr<Booking>> getBookings() const;  // newest first
    std::vector<std::shared_ptr<Booking>> getBookingsByPhone(const std::string& phone) const;
    std::vector<std::shared_ptr<Driver>> getDrivers() const { return drivers.all(); }
    std::vector<std::shared_ptr<Customer>> getCustomers() const { return customers.all(); }
    SystemStats stats() const;
    std::map<std::string, TypeReport> reportByVehicleType() const;

    // Commands
    std::shared_ptr<Booking> bookRide(const BookingRequest& request);
    void cancelBooking(int bookingId);
    void completeBooking(int bookingId, int rating);
    std::shared_ptr<Driver> addDriver(const std::string& name, const std::string& phone, const std::string& license,
                                      const std::string& vehicleType, const std::string& model,
                                      const std::string& registrationNo, const std::string& locationName);
    void toggleDriver(int driverId);
};

}  // namespace cab

#endif
