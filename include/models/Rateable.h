#ifndef CAB_MODELS_RATEABLE_H
#define CAB_MODELS_RATEABLE_H

#include <string>

namespace cab {

// A small mixin "interface" that gives any class a star-rating capability.
// Driver inherits from both Person and Rateable (MULTIPLE INHERITANCE).
class Rateable {
protected:
    double ratingTotal;
    int ratingCount;

public:
    Rateable();
    virtual ~Rateable();

    void addRating(int stars);
    double averageRating() const;
    int getRatingCount() const { return ratingCount; }
    virtual std::string ratingSummary() const;
};

}  // namespace cab

#endif
