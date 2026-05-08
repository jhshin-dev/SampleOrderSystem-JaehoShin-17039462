#pragma once
#include <vector>
#include "../model/ProductionEntry.h"

class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showProductionMenu();
    virtual void showProductionStatus(const std::vector<ProductionEntry>& entries);
    virtual void showProductionQueue(const std::vector<ProductionEntry>& entries);
    virtual void showNoProductionOrders();
};
