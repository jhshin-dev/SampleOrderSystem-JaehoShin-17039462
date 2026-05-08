#pragma once
#include <string>
#include <vector>
#include "../model/ProductionEntry.h"
#include "../model/Order.h"

class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showProductionMenu();
    virtual void showProductionStatus(const std::vector<ProductionEntry>& entries);
    virtual void showProductionQueue(const std::vector<ProductionEntry>& entries);
    virtual void showNoProductionOrders();
    virtual int  inputOrderId();
    virtual void showProductionCompleted(const Order& o, int actualQty);
    virtual void showInvalidInput(const std::string& msg);
};
