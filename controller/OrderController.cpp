#include "OrderController.h"
#include <vector>

OrderController::OrderController(MainView& mainView,
                                 OrderView& orderView,
                                 MonitorView& monitorView,
                                 IRepository<Sample>& sampleRepo,
                                 IOrderRepository& orderRepo)
    : mainView_(mainView), orderView_(orderView), monitorView_(monitorView),
      sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

void OrderController::run() {
    while (true) {
        mainView_.showOrderManagerMenu(0, 0);
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            receiveOrder();
        else if (input == 2)
            listReservedOrders();
        else if (input == 3)
            orderView_.showComingSoon();
        else if (input == 4)
            runMonitor();
        else
            mainView_.showInvalidInput();
    }
}

void OrderController::receiveOrder() {
    int sampleId = orderView_.inputSampleId();
    auto sample = sampleRepo_.findById(sampleId);
    if (!sample) {
        orderView_.showInvalidInput("존재하지 않는 시료입니다.");
        return;
    }
    std::string name = orderView_.inputCustomerName();
    if (name.empty()) {
        orderView_.showInvalidInput("고객명을 입력해주세요.");
        return;
    }
    int qty = orderView_.inputQuantity();
    if (qty <= 0) {
        orderView_.showInvalidInput("수량은 1 이상이어야 합니다.");
        return;
    }
    Order o;
    o.sampleId    = sampleId;
    o.customerName = name;
    o.quantity    = qty;
    orderView_.showOrderRegistered(orderRepo_.create(o));
}

void OrderController::listReservedOrders() {
    auto orders = orderRepo_.findAll();
    std::vector<Order> reserved;
    for (const auto& o : orders)
        if (o.status == OrderStatus::RESERVED)
            reserved.push_back(o);
    if (reserved.empty()) { orderView_.showNoOrders(); return; }
    orderView_.showOrderList(reserved, sampleRepo_.findAll());
}

void OrderController::runMonitor() {
    while (true) {
        monitorView_.showMonitorMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)      showOrderStatus();
        else if (input == 2) showStockStatus();
        else                 mainView_.showInvalidInput();
    }
}

void OrderController::showOrderStatus() {
    monitorView_.showOrderStatus(orderRepo_.findAll());
}

void OrderController::showStockStatus() {
    monitorView_.showStockStatus(sampleRepo_.findAll(), orderRepo_.findAll());
}
