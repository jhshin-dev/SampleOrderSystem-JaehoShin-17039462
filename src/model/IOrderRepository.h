#pragma once
#include "IRepository.h"
#include "Order.h"

class IOrderRepository : public IRepository<Order> {
public:
    virtual bool updateStatus(int id, OrderStatus newStatus) = 0;
};
