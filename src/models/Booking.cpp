#include "models/Booking.h"

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

int Booking::nextId = 1001;

Booking::Booking()
    : id(0),
      customerId(0),
      driverId(0),
      distanceKm(0),
      paymentFee(0),
      status(BookingStatus::Confirmed),
      rating(0),
      etaMinutes(0) {}

Booking::Booking(int customerId, int driverId, const Location& pickup, const Location& drop, double distanceKm,
                 const std::string& vehicleType, const Fare& fare, const std::string& paymentMethod,
                 const std::string& promoCode, int etaMinutes)
    : id(nextId++),
      customerId(customerId),
      driverId(driverId),
      pickup(pickup),
      drop(drop),
      distanceKm(distanceKm),
      vehicleType(vehicleType),
      fare(fare),
      paymentMethod(paymentMethod),
      promoCode(promoCode),
      paymentFee(0),
      status(BookingStatus::Confirmed),
      createdAt(utils::currentTimestamp()),
      rating(0),
      etaMinutes(etaMinutes) {}

void Booking::complete(int stars, const std::string& txnId, double fee) {
    if (status != BookingStatus::Confirmed) {
        throw InvalidStateException("Booking #" + utils::toString(id) + " is already " + statusToString(status) + ".");
    }
    if (stars < 1 || stars > 5) throw ValidationException("Rating must be between 1 and 5 stars.");
    rating = stars;
    transactionId = txnId;
    paymentFee = fee;
    status = BookingStatus::Completed;
}

void Booking::cancel() {
    if (status != BookingStatus::Confirmed) {
        throw InvalidStateException("Booking #" + utils::toString(id) + " is already " + statusToString(status) +
                                    " and cannot be cancelled.");
    }
    status = BookingStatus::Cancelled;
}

std::string Booking::statusToString(BookingStatus value) {
    switch (value) {
        case BookingStatus::Confirmed: return "Confirmed";
        case BookingStatus::Completed: return "Completed";
        case BookingStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

BookingStatus Booking::statusFromString(const std::string& text) {
    if (text == "Completed") return BookingStatus::Completed;
    if (text == "Cancelled") return BookingStatus::Cancelled;
    return BookingStatus::Confirmed;
}

std::ostream& operator<<(std::ostream& out, const Booking& booking) {
    out << "Booking #" << booking.id << " [" << Booking::statusToString(booking.status) << "] "
        << booking.pickup.getName() << " -> " << booking.drop.getName() << " by " << booking.vehicleType << ", Rs "
        << utils::fixed(booking.fare.total());
    return out;
}

}  // namespace cab
