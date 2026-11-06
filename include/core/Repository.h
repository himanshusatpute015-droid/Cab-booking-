#ifndef CAB_CORE_REPOSITORY_H
#define CAB_CORE_REPOSITORY_H

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

#include "core/Exceptions.h"
#include "core/Utils.h"

namespace cab {

// CLASS TEMPLATE: a generic, type-safe in-memory store reused for Customers,
// Drivers and Bookings. Objects are shared through smart pointers (RAII).
template <typename T, typename Key = int>
class Repository {
private:
    std::map<Key, std::shared_ptr<T>> items;

public:
    void add(const Key& key, const std::shared_ptr<T>& item) {
        if (items.count(key)) {
            throw CabException("Duplicate record id: " + utils::toString(key));
        }
        items[key] = item;
    }

    std::shared_ptr<T> find(const Key& key) const {
        typename std::map<Key, std::shared_ptr<T>>::const_iterator it = items.find(key);
        return it == items.end() ? std::shared_ptr<T>() : it->second;
    }

    bool exists(const Key& key) const { return items.count(key) > 0; }

    bool remove(const Key& key) { return items.erase(key) > 0; }

    std::vector<std::shared_ptr<T>> all() const {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& entry : items) result.push_back(entry.second);
        return result;
    }

    // MEMBER FUNCTION TEMPLATE: accepts any predicate (lambda / functor).
    template <typename Predicate>
    std::vector<std::shared_ptr<T>> filter(Predicate predicate) const {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& entry : items) {
            if (predicate(*entry.second)) result.push_back(entry.second);
        }
        return result;
    }

    template <typename Predicate>
    std::shared_ptr<T> findFirst(Predicate predicate) const {
        for (const auto& entry : items) {
            if (predicate(*entry.second)) return entry.second;
        }
        return std::shared_ptr<T>();
    }

    std::size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear() { items.clear(); }
};

}  // namespace cab

#endif
