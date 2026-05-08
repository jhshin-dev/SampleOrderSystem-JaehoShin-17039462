#include "OrderRepository.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

OrderRepository::OrderRepository(const std::string& filePath)
    : JsonRepository<Order>(filePath) {}

std::string OrderRepository::nowIso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

Order OrderRepository::create(Order entity) {
    entity.status    = OrderStatus::RESERVED;
    entity.createdAt = nowIso8601();
    entity.updatedAt = entity.createdAt;
    return JsonRepository<Order>::create(entity);
}

bool OrderRepository::remove(int id) {
    auto found = findById(id);
    if (found && found->status == OrderStatus::RELEASED) return false;
    return JsonRepository<Order>::remove(id);
}

bool OrderRepository::updateStatus(int id, OrderStatus newStatus) {
    auto found = findById(id);
    if (!found) return false;
    if (!isValidTransition(found->status, newStatus)) return false;
    Order updated = *found;
    updated.status    = newStatus;
    updated.updatedAt = nowIso8601();
    return JsonRepository<Order>::update(updated);
}
