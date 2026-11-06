#include "models/Rateable.h"

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

Rateable::Rateable() : ratingTotal(0.0), ratingCount(0) {}

Rateable::~Rateable() {}

void Rateable::addRating(int stars) {
    if (stars < 1 || stars > 5) throw ValidationException("Rating must be between 1 and 5 stars.");
    ratingTotal += stars;
    ++ratingCount;
}

double Rateable::averageRating() const { return ratingCount == 0 ? 0.0 : ratingTotal / ratingCount; }

std::string Rateable::ratingSummary() const {
    if (ratingCount == 0) return "New - no ratings yet";
    return utils::fixed(averageRating(), 1) + " \xE2\x98\x85 (" + utils::toString(ratingCount) + " ratings)";
}

}  // namespace cab
