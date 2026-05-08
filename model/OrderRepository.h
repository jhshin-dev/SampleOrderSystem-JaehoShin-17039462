#pragma once
#include "IOrderRepository.h"
#include "JsonRepository.h"

class OrderRepository : public JsonRepository<Order>, public IOrderRepository {
public:
    explicit OrderRepository(const std::string& filePath = "data/orders.json");
    Order create(Order entity) override;
    bool  remove(int id)       override;
    bool  updateStatus(int id, OrderStatus newStatus) override;

    // IRepository<Order> 메서드는 JsonRepository에서 상속
    std::vector<Order>   findAll()               override { return JsonRepository<Order>::findAll(); }
    std::optional<Order> findById(int id)        override { return JsonRepository<Order>::findById(id); }
    bool                 update(const Order& o)  override { return JsonRepository<Order>::update(o); }

private:
    static std::string nowIso8601();
};
