#ifndef CAB_SERVICES_FILE_STORAGE_H
#define CAB_SERVICES_FILE_STORAGE_H

#include <memory>
#include <string>
#include <vector>

#include "models/Booking.h"
#include "models/Customer.h"
#include "models/Driver.h"

namespace cab {

// FILE HANDLING: persists every object to pipe-separated text files using fstream.
// It is declared a friend of Customer, Driver and Booking so it can restore their
// private state exactly as it was saved.
class FileStorage {
private:
    std::string directory;

    std::string pathFor(const std::string& fileName) const;
    static std::vector<std::string> readLines(const std::string& path);
    static void writeLines(const std::string& path, const std::vector<std::string>& lines);

public:
    explicit FileStorage(const std::string& directory);

    bool hasSavedData() const;

    void saveCustomers(const std::vector<std::shared_ptr<Customer>>& customers) const;
    void saveDrivers(const std::vector<std::shared_ptr<Driver>>& drivers) const;
    void saveBookings(const std::vector<std::shared_ptr<Booking>>& bookings) const;

    std::vector<std::shared_ptr<Customer>> loadCustomers() const;
    std::vector<std::shared_ptr<Driver>> loadDrivers() const;
    std::vector<std::shared_ptr<Booking>> loadBookings() const;
};

}  // namespace cab

#endif
