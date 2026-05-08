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
            runReleaseMenu();
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

void OrderController::runReleaseMenu() {
    while (true) {
        orderView_.showReleaseMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)      listConfirmedOrders();
        else if (input == 2) executeRelease();
        else                 mainView_.showInvalidInput();
    }
}

void OrderController::listConfirmedOrders() {
    auto all = orderRepo_.findAll();
    std::vector<Order> confirmed;
    for (const auto& o : all)
        if (o.status == OrderStatus::CONFIRMED)
            confirmed.push_back(o);
    if (confirmed.empty()) { orderView_.showNoConfirmedOrders(); return; }
    orderView_.showOrderList(confirmed, sampleRepo_.findAll());
}

void OrderController::executeRelease() {
    int id = orderView_.inputOrderId();
    auto found = orderRepo_.findById(id);
    if (!found) {
        orderView_.showInvalidInput("존재하지 않는 주문입니다.");
        return;
    }
    if (found->status != OrderStatus::CONFIRMED) {
        orderView_.showInvalidInput("CONFIRMED 상태의 주문만 출고할 수 있습니다.");
        return;
    }
    orderRepo_.updateStatus(id, OrderStatus::RELEASED);
    auto sample = sampleRepo_.findById(found->sampleId);
    if (sample) {
        Sample updated = *sample;
        updated.stock -= found->quantity;
        sampleRepo_.update(updated);
    }
    orderView_.showReleased(*found);
}
