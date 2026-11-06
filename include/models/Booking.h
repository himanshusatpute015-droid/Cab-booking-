#ifndef CAB_MODELS_BOOKING_H
#define CAB_MODELS_BOOKING_H

#include <ostream>
#include <string>

#include "models/Fare.h"
#include "models/Location.h"

namespace cab {

enum class BookingStatus { Confirmed, Completed, Cancelled };  // strongly-typed enum

// A Booking ASSOCIATES a Customer with a Driver (by id) for a single trip.
class Booking {
private:
    int id;
    int customerId;
    int driverId;
    Location pickup;
    Location drop;
    double distanceKm;
    std::string vehicleType;
    Fare fare;
    std::string paymentMethod;
    std::string promoCode;
    std::string transactionId;
    double paymentFee;
    BookingStatus status;
    std::string createdAt;
    int rating;
    int etaMinutes;

    static int nextId;

    Booking();                 // private: only FileStorage (a friend) can build an empty booking
    friend class FileStorage;  // FRIEND CLASS

public:
    Booking(int customerId, int driverId, const Location& pickup, const Location& drop, double distanceKm,
            const std::string& vehicleType, const Fare& fare, const std::string& paymentMethod,
            const std::string& promoCode, int etaMinutes);

    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    int getDriverId() const { return driverId; }
    const Location& getPickup() const { return pickup; }
    const Location& getDrop() const { return drop; }
    double getDistanceKm() const { return distanceKm; }
    const std::string& getVehicleType() const { return vehicleType; }
    const Fare& getFare() const { return fare; }
    const std::string& getPaymentMethod() const { return paymentMethod; }
    const std::string& getPromoCode() const { return promoCode; }
    const std::string& getTransactionId() const { return transactionId; }
    double getPaymentFee() const { return paymentFee; }
    BookingStatus getStatus() const { return status; }
    const std::string& getCreatedAt() const { return createdAt; }
    int getRating() const { return rating; }
    int getEtaMinutes() const { return etaMinutes; }

    bool isActive() const { return status == BookingStatus::Confirmed; }
    double amountPaid() const { return fare.total() + paymentFee; }

    void complete(int stars, const std::string& txnId, double fee);
    void cancel();

    static std::string statusToString(BookingStatus status);
    static BookingStatus statusFromString(const std::string& text);

    friend std::ostream& operator<<(std::ostream& out, const Booking& booking);
};

}  // namespace cab

#endif
