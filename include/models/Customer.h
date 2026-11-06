#ifndef CAB_MODELS_CUSTOMER_H
#define CAB_MODELS_CUSTOMER_H

#include "models/Person.h"

namespace cab {

// SINGLE INHERITANCE: a Customer "is a" Person.
class Customer : public Person {
private:
    int totalRides;
    double totalSpent;
    static int nextId;

    friend class FileStorage;  // FRIEND CLASS: storage may restore private state

public:
    // CONSTRUCTOR OVERLOADING: new customer vs. customer restored from disk.
    Customer(const std::string& name, const std::string& phone);
    Customer(int id, const std::string& name, const std::string& phone, int totalRides, double totalSpent);

    std::string getRole() const override { return "Customer"; }
    std::string profile() const override;

    int getTotalRides() const { return totalRides; }
    double getTotalSpent() const { return totalSpent; }
    bool isFirstRide() const { return totalRides == 0; }
    std::string tier() const;

    void recordRide(double amount);

    static void ensureNextIdAbove(int usedId);
};

}  // namespace cab

#endif
