#pragma once
#include <vector>
#include "../model/Order.h"
#include "../model/Sample.h"

class MonitorView {
public:
    virtual ~MonitorView() = default;
    virtual void showMonitorMenu();
    virtual void showOrderStatus(const std::vector<Order>& orders);
    virtual void showStockStatus(const std::vector<Sample>& samples,
                                 const std::vector<Order>& orders);
};
