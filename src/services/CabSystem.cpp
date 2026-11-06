#include "services/CabSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "core/Exceptions.h"
#include "core/Utils.h"
#include "models/Payment.h"
#include "services/VehicleFactory.h"

namespace cab {

const double CabSystem::AVERAGE_SPEED_KMPH = 24.0;

CabSystem::CabSystem() {
    seedLocations();
    seedCoupons();
}

CabSystem& CabSystem::instance() {
    static CabSystem system;  // created once, on first use (thread-safe since C++11)
    return system;
}

// ===================================================================== setup
void CabSystem::init(const std::string& dataDirectory) {
    storage.reset(new FileStorage(dataDirectory));
    customers.clear();
    drivers.clear();
    bookings.clear();

    if (storage->hasSavedData()) {
        for (const auto& c : storage->loadCustomers()) customers.add(c->getId(), c);
        for (const auto& d : storage->loadDrivers()) drivers.add(d->getId(), d);
        for (const auto& b : storage->loadBookings()) bookings.add(b->getId(), b);
        // Drivers whose ride is still open must stay busy after a restart.
        for (const auto& b : bookings.all()) {
            if (b->isActive()) {
                auto d = drivers.find(b->getDriverId());
                if (d) d->resumeRide();
            }
        }
        std::cout << "[system] loaded " << drivers.size() << " drivers, " << customers.size() << " customers, "
                  << bookings.size() << " bookings" << std::endl;
    }
    if (drivers.empty()) {
        seedDrivers();
        persist();
        std::cout << "[system] seeded " << drivers.size() << " demo drivers" << std::endl;
    }
}

void CabSystem::seedLocations() {
    locations = {
        Location("Railway Station", 0, 0),   Location("City Mall", 4, 3),       Location("Airport", 14, 9),
        Location("Tech Park", 9, -4),        Location("University", -5, 6),     Location("Old Town", -3, -2),
        Location("Central Hospital", 2, -5), Location("Lake View", -8, -6),     Location("Bus Stand", 1, 2),
        Location("Sports Stadium", 6, 8),    Location("Industrial Area", 12, -9), Location("Green Valley", -10, 3),
    };
}

void CabSystem::seedCoupons() {
    // Base-class pointers to different derived coupons (POLYMORPHIC CONTAINER).
    coupons.push_back(std::unique_ptr<Coupon>(new PercentCoupon("FIRST50", 50, 100, true)));
    coupons.push_back(std::unique_ptr<Coupon>(new FlatCoupon("SAVE30", 30, 150)));
    coupons.push_back(std::unique_ptr<Coupon>(new PercentCoupon("RIDE10", 10, 60)));
    coupons.push_back(std::unique_ptr<Coupon>(new LoyaltyCoupon("LOYAL20", 3, 20)));
}

void CabSystem::seedDrivers() {
    struct Seed {
        const char *name, *phone, *license, *type, *model, *reg, *location;
    };
    const Seed seeds[] = {
        {"Rahul Verma", "9876501234", "MH12DL2019001", "Bike", "Honda Activa", "MH12AB1001", "Railway Station"},
        {"Sneha Patil", "9876502345", "MH12DL2018002", "Bike", "TVS Jupiter", "MH12AB1002", "Tech Park"},
        {"Imran Shaikh", "9876503456", "MH12DL2015003", "Auto", "Bajaj RE", "MH12AR2001", "Old Town"},
        {"Kavita Joshi", "9876504567", "MH12DL2016004", "Auto", "Piaggio Ape", "MH12AR2002", "University"},
        {"Arjun Mehta", "9876505678", "MH12DL2017005", "Mini", "Maruti WagonR", "MH12CM3001", "City Mall"},
        {"Priya Nair", "9876506789", "MH12DL2014006", "Mini", "Hyundai i10", "MH12CM3002", "Bus Stand"},
        {"Vikram Singh", "9876507890", "MH12DL2013007", "Sedan", "Honda City", "MH12CS4001", "Airport"},
        {"Neha Kulkarni", "9876508901", "MH12DL2012008", "Sedan", "Maruti Dzire", "MH12CS4002", "Central Hospital"},
        {"Rohan Das", "9876509012", "MH12DL2011009", "SUV", "Toyota Innova", "MH12SU5001", "Sports Stadium"},
        {"Anjali Rao", "9876500123", "MH12DL2010010", "SUV", "Mahindra XUV700", "MH12SU5002", "Lake View"},
        {"Karan Malhotra", "9812345670", "MH12DL2009011", "Premier", "Skoda Superb", "MH12PR6001", "Tech Park"},
        {"Farah Khan", "9812345671", "MH12DL2008012", "Premier", "Toyota Camry", "MH12PR6002", "Green Valley"},
    };
    const int ratings[] = {5, 4, 5, 4, 5, 3, 5, 4, 4, 5, 5, 5};
    int index = 0;
    for (const Seed& s : seeds) {
        auto vehicle = VehicleFactory::create(s.type, s.reg, s.model);
        auto driver = std::make_shared<Driver>(s.name, s.phone, s.license, vehicle, findLocation(s.location));
        driver->addRating(ratings[index]);
        driver->addRating(ratings[(index + 3) % 12]);
        drivers.add(driver->getId(), driver);
        ++index;
    }
}

void CabSystem::persist() const {
    if (!storage) return;
    storage->saveCustomers(customers.all());
    storage->saveDrivers(drivers.all());
    storage->saveBookings(bookings.all());
}

// ===================================================================== queries
const Location& CabSystem::findLocation(const std::string& name) const {
    for (const Location& loc : locations) {
        if (loc.getName() == name) return loc;
    }
    throw ValidationException("Unknown location: '" + name + "'. Please choose one from the list.");
}

double CabSystem::currentSurge() const {
    std::size_t online = 0, busy = 0;
    for (const auto& d : drivers.all()) {
        if (!d->isOnline()) continue;
        ++online;
        if (d->isOnRide()) ++busy;
    }
    if (online == 0) return 2.0;
    double demand = static_cast<double>(busy) / online;
    if (demand >= 0.75) return 1.8;
    if (demand >= 0.5) return 1.4;
    if (demand >= 0.25) return 1.2;
    return 1.0;
}

double CabSystem::distanceBetween(const std::string& pickup, const std::string& drop) const {
    const Location& from = findLocation(pickup);
    const Location& to = findLocation(drop);
    if (from == to) throw ValidationException("Pickup and drop locations must be different.");
    return from - to;  // overloaded operator-
}

int CabSystem::etaFor(double km) {
    int minutes = static_cast<int>(std::ceil(km / AVERAGE_SPEED_KMPH * 60.0));
    return std::max(2, minutes);
}

std::vector<FareEstimate> CabSystem::estimateFares(const std::string& pickup, const std::string& drop) const {
    double km = distanceBetween(pickup, drop);
    const Location& from = findLocation(pickup);
    double surge = currentSurge();

    std::vector<FareEstimate> estimates;
    for (const auto& vehicle : VehicleFactory::catalogue()) {
        FareEstimate e;
        e.vehicle = vehicle;
        e.fare = vehicle->calculateFare(km, surge);  // dynamic dispatch on each subclass
        std::string type = vehicle->getType();
        e.availableDrivers = static_cast<int>(
            drivers.filter([&type](const Driver& d) { return d.isAvailable() && d.getVehicle()->getType() == type; })
                .size());
        auto nearest = nearestDriver(type, from);
        e.etaMinutes = nearest ? etaFor(nearest->distanceTo(from)) : -1;
        estimates.push_back(e);
    }
    return estimates;
}

std::shared_ptr<Driver> CabSystem::nearestDriver(const std::string& vehicleType, const Location& point) const {
    std::shared_ptr<Driver> best;
    double bestDistance = 1e18;
    for (const auto& d : drivers.all()) {
        if (!d->isAvailable() || d->getVehicle()->getType() != vehicleType) continue;
        double km = d->distanceTo(point);
        // Prefer the closest driver; break ties with the better rating.
        if (km < bestDistance - 1e-9 ||
            (std::fabs(km - bestDistance) < 1e-9 && best && d->averageRating() > best->averageRating())) {
            best = d;
            bestDistance = km;
        }
    }
    return best;
}

const Coupon& CabSystem::findCoupon(const std::string& code) const {
    std::string wanted = utils::toUpper(utils::trim(code));
    for (const auto& coupon : coupons) {
        if (coupon->getCode() == wanted) return *coupon;
    }
    throw ValidationException("Promo code '" + code + "' does not exist.");
}

std::shared_ptr<Booking> CabSystem::getBooking(int id) const {
    auto booking = bookings.find(id);
    if (!booking) throw NotFoundException("Booking #" + utils::toString(id) + " was not found.");
    return booking;
}

std::shared_ptr<Driver> CabSystem::getDriver(int id) const {
    auto driver = drivers.find(id);
    if (!driver) throw NotFoundException("Driver #" + utils::toString(id) + " was not found.");
    return driver;
}

std::shared_ptr<Customer> CabSystem::getCustomer(int id) const {
    auto customer = customers.find(id);
    if (!customer) throw NotFoundException("Customer #" + utils::toString(id) + " was not found.");
    return customer;
}

std::vector<std::shared_ptr<Booking>> CabSystem::getBookings() const {
    std::vector<std::shared_ptr<Booking>> list = bookings.all();
    std::sort(list.begin(), list.end(),
              [](const std::shared_ptr<Booking>& a, const std::shared_ptr<Booking>& b) { return a->getId() > b->getId(); });
    return list;
}

std::vector<std::shared_ptr<Booking>> CabSystem::getBookingsByPhone(const std::string& phone) const {
    std::string wanted = utils::trim(phone);
    auto customer = customers.findFirst([&wanted](const Customer& c) { return c.getPhone() == wanted; });
    std::vector<std::shared_ptr<Booking>> result;
    if (!customer) return result;
    int id = customer->getId();
    for (const auto& b : getBookings()) {
        if (b->getCustomerId() == id) result.push_back(b);
    }
    return result;
}

SystemStats CabSystem::stats() const {
    SystemStats s = {0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 1.0};
    for (const auto& b : bookings.all()) {
        ++s.totalBookings;
        switch (b->getStatus()) {
            case BookingStatus::Confirmed: ++s.activeRides; break;
            case BookingStatus::Completed:
                ++s.completedRides;
                s.revenue += b->getFare().total();
                break;
            case BookingStatus::Cancelled: ++s.cancelledRides; break;
        }
    }
    double ratingSum = 0;
    int rated = 0;
    for (const auto& d : drivers.all()) {
        ++s.totalDrivers;
        if (d->isAvailable()) ++s.availableDrivers;
        s.driverPayouts += d->getEarnings();
        if (d->getRatingCount() > 0) {
            ratingSum += d->averageRating();
            ++rated;
        }
    }
    s.totalCustomers = static_cast<int>(customers.size());
    s.averageRating = rated ? ratingSum / rated : 0.0;
    s.surge = currentSurge();
    return s;
}

std::map<std::string, TypeReport> CabSystem::reportByVehicleType() const {
    std::map<std::string, TypeReport> report;
    for (const std::string& type : VehicleFactory::types()) report[type] = TypeReport();
    for (const auto& b : bookings.all()) {
        if (b->getStatus() != BookingStatus::Completed) continue;
        TypeReport& r = report[b->getVehicleType()];
        ++r.rides;
        r.revenue += b->getFare().total();
    }
    return report;
}

// ===================================================================== commands
std::shared_ptr<Customer> CabSystem::findOrCreateCustomer(const std::string& name, const std::string& phone) {
    std::string cleanPhone = Person::validatePhone(phone);
    auto existing = customers.findFirst([&cleanPhone](const Customer& c) { return c.getPhone() == cleanPhone; });
    if (existing) {
        existing->setName(name);  // keep the latest name the rider used
        return existing;
    }
    auto created = std::make_shared<Customer>(name, cleanPhone);
    customers.add(created->getId(), created);
    return created;
}

std::shared_ptr<Booking> CabSystem::bookRide(const BookingRequest& request) {
    if (!VehicleFactory::isValidType(request.vehicleType)) {
        throw ValidationException("Please choose a cab type.");
    }
    PaymentFactory::create(request.paymentMethod);  // validates the payment method early

    double km = distanceBetween(request.pickup, request.drop);
    const Location& from = findLocation(request.pickup);
    const Location& to = findLocation(request.drop);

    // Validate the rider before changing any state.
    Person::validateName(request.riderName);
    Person::validatePhone(request.riderPhone);

    auto driver = nearestDriver(request.vehicleType, from);
    if (!driver) throw NoDriverAvailableException(request.vehicleType);

    Fare fare = driver->getVehicle()->calculateFare(km, currentSurge());

    // Check the promo code against the rider's history before any state is changed.
    std::string promo = utils::toUpper(utils::trim(request.promoCode));
    if (!promo.empty()) {
        std::string phone = Person::validatePhone(request.riderPhone);
        auto existing = customers.findFirst([&phone](const Customer& c) { return c.getPhone() == phone; });
        Customer rider = existing ? *existing : Customer(0, request.riderName, phone, 0, 0.0);  // copy / temp object
        const Coupon& coupon = findCoupon(promo);
        fare.applyDiscount(coupon.discountFor(fare.total(), rider));  // virtual call
    }

    std::shared_ptr<Customer> customer = findOrCreateCustomer(request.riderName, request.riderPhone);
    driver->startRide();
    auto booking = std::make_shared<Booking>(customer->getId(), driver->getId(), from, to, km,
                                             request.vehicleType, fare, PaymentFactory::create(request.paymentMethod)->method(),
                                             promo, etaFor(driver->distanceTo(from)));
    bookings.add(booking->getId(), booking);
    persist();
    std::cout << "[booking] " << *booking << std::endl;  // overloaded operator<<
    return booking;
}

void CabSystem::cancelBooking(int bookingId) {
    auto booking = getBooking(bookingId);
    booking->cancel();
    auto driver = drivers.find(booking->getDriverId());
    if (driver) driver->releaseFromRide();
    persist();
}

void CabSystem::completeBooking(int bookingId, int rating) {
    auto booking = getBooking(bookingId);
    if (!booking->isActive()) {
        throw InvalidStateException("Booking #" + utils::toString(bookingId) + " is already " +
                                    Booking::statusToString(booking->getStatus()) + ".");
    }
    if (rating < 1 || rating > 5) throw ValidationException("Please rate the driver from 1 to 5 stars.");

    std::unique_ptr<Payment> payment = PaymentFactory::create(booking->getPaymentMethod());
    double amount = booking->getFare().total();
    double fee = payment->processingFee(amount);              // strategy-specific fee
    std::string txn = payment->pay(amount + fee, bookingId);  // strategy-specific processing

    booking->complete(rating, txn, fee);

    auto driver = drivers.find(booking->getDriverId());
    if (driver) {
        driver->finishRide(booking->getDrop(), amount);
        driver->addRating(rating);
    }
    auto customer = customers.find(booking->getCustomerId());
    if (customer) customer->recordRide(booking->amountPaid());
    persist();
}

std::shared_ptr<Driver> CabSystem::addDriver(const std::string& name, const std::string& phone,
                                             const std::string& license, const std::string& vehicleType,
                                             const std::string& model, const std::string& registrationNo,
                                             const std::string& locationName) {
    std::string cleanPhone = Person::validatePhone(phone);
    if (drivers.findFirst([&cleanPhone](const Driver& d) { return d.getPhone() == cleanPhone; })) {
        throw ValidationException("A driver with phone " + cleanPhone + " is already registered.");
    }
    std::string reg = utils::toUpper(utils::trim(registrationNo));
    if (drivers.findFirst([&reg](const Driver& d) { return d.getVehicle()->getRegistrationNo() == reg; })) {
        throw ValidationException("Vehicle " + reg + " is already registered.");
    }
    auto vehicle = VehicleFactory::create(vehicleType, registrationNo, model);
    auto driver = std::make_shared<Driver>(name, cleanPhone, license, vehicle, findLocation(locationName));
    drivers.add(driver->getId(), driver);
    persist();
    return driver;
}

void CabSystem::toggleDriver(int driverId) {
    getDriver(driverId)->toggleOnline();
    persist();
}

}  // namespace cab
