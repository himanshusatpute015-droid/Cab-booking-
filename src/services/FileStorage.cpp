#include "services/FileStorage.h"

#include <fstream>
#include <iostream>

#include "core/Utils.h"
#include "services/VehicleFactory.h"

#ifdef _WIN32
#include <direct.h>
#define CAB_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define CAB_MKDIR(path) mkdir(path, 0755)
#endif

namespace cab {

using utils::fixed;
using utils::split;
using utils::toDouble;
using utils::toInt;
using utils::toString;

FileStorage::FileStorage(const std::string& directory) : directory(directory) { CAB_MKDIR(directory.c_str()); }

std::string FileStorage::pathFor(const std::string& fileName) const { return directory + "/" + fileName; }

bool FileStorage::hasSavedData() const {
    std::ifstream in(pathFor("drivers.txt").c_str());
    return in.good();
}

std::vector<std::string> FileStorage::readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream in(path.c_str());
    std::string line;
    while (std::getline(in, line)) {
        line = utils::trim(line);
        if (!line.empty() && line[0] != '#') lines.push_back(line);
    }
    return lines;
}

void FileStorage::writeLines(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream out(path.c_str(), std::ios::trunc);
    if (!out) {
        std::cerr << "[storage] could not write " << path << std::endl;
        return;
    }
    for (const std::string& line : lines) out << line << '\n';
}

// ------------------------------------------------------------------ save
void FileStorage::saveCustomers(const std::vector<std::shared_ptr<Customer>>& customers) const {
    std::vector<std::string> lines = {"# id|name|phone|rides|spent"};
    for (const auto& c : customers) {
        lines.push_back(toString(c->id) + "|" + c->name + "|" + c->phone + "|" + toString(c->totalRides) + "|" +
                        fixed(c->totalSpent));
    }
    writeLines(pathFor("customers.txt"), lines);
}

void FileStorage::saveDrivers(const std::vector<std::shared_ptr<Driver>>& drivers) const {
    std::vector<std::string> lines = {
        "# id|name|phone|license|type|reg|model|loc|x|y|online|earnings|ratingTotal|ratingCount|rides"};
    for (const auto& d : drivers) {
        lines.push_back(toString(d->id) + "|" + d->name + "|" + d->phone + "|" + d->licenseNo + "|" +
                        d->vehicle->getType() + "|" + d->vehicle->getRegistrationNo() + "|" + d->vehicle->getModel() +
                        "|" + d->location.getName() + "|" + fixed(d->location.getX()) + "|" +
                        fixed(d->location.getY()) + "|" + (d->online ? "1" : "0") + "|" + fixed(d->earnings) + "|" +
                        fixed(d->ratingTotal) + "|" + toString(d->ratingCount) + "|" + toString(d->completedRides));
    }
    writeLines(pathFor("drivers.txt"), lines);
}

void FileStorage::saveBookings(const std::vector<std::shared_ptr<Booking>>& bookings) const {
    std::vector<std::string> lines = {
        "# id|customer|driver|pickup|px|py|drop|dx|dy|km|type|base|distance|surge|extra|tax|discount|"
        "payment|promo|txn|fee|status|created|rating|eta"};
    for (const auto& b : bookings) {
        const Fare& f = b->fare;
        lines.push_back(toString(b->id) + "|" + toString(b->customerId) + "|" + toString(b->driverId) + "|" +
                        b->pickup.getName() + "|" + fixed(b->pickup.getX()) + "|" + fixed(b->pickup.getY()) + "|" +
                        b->drop.getName() + "|" + fixed(b->drop.getX()) + "|" + fixed(b->drop.getY()) + "|" +
                        fixed(b->distanceKm) + "|" + b->vehicleType + "|" + fixed(f.getBaseFare()) + "|" +
                        fixed(f.getDistanceFare()) + "|" + fixed(f.getSurgeCharge()) + "|" +
                        fixed(f.getExtraCharge()) + "|" + fixed(f.getTax()) + "|" + fixed(f.getDiscount()) + "|" +
                        b->paymentMethod + "|" + b->promoCode + "|" + b->transactionId + "|" +
                        fixed(b->paymentFee) + "|" + Booking::statusToString(b->status) + "|" + b->createdAt + "|" +
                        toString(b->rating) + "|" + toString(b->etaMinutes));
    }
    writeLines(pathFor("bookings.txt"), lines);
}

// ------------------------------------------------------------------ load
std::vector<std::shared_ptr<Customer>> FileStorage::loadCustomers() const {
    std::vector<std::shared_ptr<Customer>> result;
    for (const std::string& line : readLines(pathFor("customers.txt"))) {
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 5) continue;
        try {
            result.push_back(std::make_shared<Customer>(toInt(p[0]), p[1], p[2], toInt(p[3]), toDouble(p[4])));
        } catch (const std::exception& e) {
            std::cerr << "[storage] skipped customer: " << e.what() << std::endl;
        }
    }
    return result;
}

std::vector<std::shared_ptr<Driver>> FileStorage::loadDrivers() const {
    std::vector<std::shared_ptr<Driver>> result;
    for (const std::string& line : readLines(pathFor("drivers.txt"))) {
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 15) continue;
        try {
            auto vehicle = VehicleFactory::create(p[4], p[5], p[6]);
            Location loc(p[7], toDouble(p[8]), toDouble(p[9]));
            auto driver = std::make_shared<Driver>(toInt(p[0]), p[1], p[2], p[3], vehicle, loc);
            driver->online = (p[10] == "1");
            driver->earnings = toDouble(p[11]);
            driver->ratingTotal = toDouble(p[12]);
            driver->ratingCount = toInt(p[13]);
            driver->completedRides = toInt(p[14]);
            result.push_back(driver);
        } catch (const std::exception& e) {
            std::cerr << "[storage] skipped driver: " << e.what() << std::endl;
        }
    }
    return result;
}

std::vector<std::shared_ptr<Booking>> FileStorage::loadBookings() const {
    std::vector<std::shared_ptr<Booking>> result;
    for (const std::string& line : readLines(pathFor("bookings.txt"))) {
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 25) continue;
        std::shared_ptr<Booking> b(new Booking());  // private constructor - allowed for a friend
        b->id = toInt(p[0]);
        b->customerId = toInt(p[1]);
        b->driverId = toInt(p[2]);
        b->pickup = Location(p[3], toDouble(p[4]), toDouble(p[5]));
        b->drop = Location(p[6], toDouble(p[7]), toDouble(p[8]));
        b->distanceKm = toDouble(p[9]);
        b->vehicleType = p[10];
        b->fare = Fare(toDouble(p[11]), toDouble(p[12]), toDouble(p[13]), toDouble(p[14]), toDouble(p[15]),
                       toDouble(p[16]));
        b->paymentMethod = p[17];
        b->promoCode = p[18];
        b->transactionId = p[19];
        b->paymentFee = toDouble(p[20]);
        b->status = Booking::statusFromString(p[21]);
        b->createdAt = p[22];
        b->rating = toInt(p[23]);
        b->etaMinutes = toInt(p[24]);
        if (b->id >= Booking::nextId) Booking::nextId = b->id + 1;
        result.push_back(b);
    }
    return result;
}

}  // namespace cab
