#pragma once
#include <string>
#include "OrderStatus.h"
#include "../lib/json.hpp"

struct Order {
    int         id           = 0;
    int         sampleId     = 0;
    std::string customerName;
    int         quantity     = 0;
    OrderStatus status       = OrderStatus::RESERVED;
    std::string createdAt;
    std::string updatedAt;

    bool operator==(const Order& o) const { return id == o.id; }
};

inline void to_json(nlohmann::json& j, const Order& o) {
    j = { {"id", o.id}, {"sampleId", o.sampleId},
          {"customerName", o.customerName}, {"quantity", o.quantity},
          {"status", toString(o.status)},
          {"createdAt", o.createdAt}, {"updatedAt", o.updatedAt} };
}

inline void from_json(const nlohmann::json& j, Order& o) {
    j.at("id").get_to(o.id);
    j.at("sampleId").get_to(o.sampleId);
    j.at("customerName").get_to(o.customerName);
    j.at("quantity").get_to(o.quantity);
    o.status = statusFromString(j.at("status").get<std::string>());
    j.at("createdAt").get_to(o.createdAt);
    j.at("updatedAt").get_to(o.updatedAt);
}
