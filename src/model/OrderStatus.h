#pragma once
#include <string>

enum class OrderStatus {
    RESERVED, CONFIRMED, PRODUCING, REJECTED, RELEASED
};

inline std::string toString(OrderStatus s) {
    switch (s) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::REJECTED:  return "REJECTED";
        case OrderStatus::RELEASED:  return "RELEASED";
    }
    return "UNKNOWN";
}

inline OrderStatus statusFromString(const std::string& s) {
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}

inline bool isValidTransition(OrderStatus from, OrderStatus to) {
    switch (from) {
        case OrderStatus::RESERVED:
            return to == OrderStatus::CONFIRMED
                || to == OrderStatus::PRODUCING
                || to == OrderStatus::REJECTED;
        case OrderStatus::PRODUCING:
            return to == OrderStatus::CONFIRMED;
        case OrderStatus::CONFIRMED:
            return to == OrderStatus::RELEASED;
        default:
            return false;
    }
}
