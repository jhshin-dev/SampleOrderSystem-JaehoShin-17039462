#include "ProductionController.h"
#include <vector>
#include <algorithm>
#include <cmath>

ProductionController::ProductionController(MainView& mainView,
                                           SampleView& sampleView,
                                           OrderView& orderView,
                                           MonitorView& monitorView,
                                           ProductionView& productionView,
                                           IRepository<Sample>& sampleRepo,
                                           IOrderRepository& orderRepo)
    : mainView_(mainView), sampleView_(sampleView), orderView_(orderView),
      monitorView_(monitorView), productionView_(productionView),
      sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

void ProductionController::run() {
    while (true) {
        mainView_.showProductionManagerMenu(0, 0);
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            runSampleMenu();
        else if (input == 2)
            runOrderApprovalMenu();
        else if (input == 3)
            runProductionMenu();
        else if (input == 4)
            runMonitor();
        else
            mainView_.showInvalidInput();
    }
}

// ── 시료 관리 ──────────────────────────────────────────────

void ProductionController::runSampleMenu() {
    while (true) {
        sampleView_.showSampleMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            registerSample();
        else if (input == 2)
            listSamples();
        else if (input == 3)
            searchSamples();
        else
            mainView_.showInvalidInput();
    }
}

void ProductionController::registerSample() {
    std::string name = sampleView_.inputName();
    if (name.empty()) {
        sampleView_.showInvalidInput("이름을 입력해주세요.");
        return;
    }
    int avgTime = sampleView_.inputAvgProductionTime();
    if (avgTime <= 0) {
        sampleView_.showInvalidInput("평균 생산시간은 1분 이상이어야 합니다.");
        return;
    }
    double yield = sampleView_.inputYield();
    if (yield <= 0.0 || yield > 1.0) {
        sampleView_.showInvalidInput("수율은 0.0 초과 1.0 이하로 입력해주세요.");
        return;
    }
    Sample s;
    s.name              = name;
    s.avgProductionTime = avgTime;
    s.yield             = yield;
    Sample created = sampleRepo_.create(s);
    sampleView_.showRegistered(created);
}

void ProductionController::listSamples() {
    auto samples = sampleRepo_.findAll();
    sampleView_.showSampleList(samples);
}

void ProductionController::searchSamples() {
    std::string keyword = sampleView_.inputSearchKeyword();
    auto all = sampleRepo_.findAll();
    std::vector<Sample> result;
    for (const auto& s : all)
        if (s.name.find(keyword) != std::string::npos)
            result.push_back(s);
    if (result.empty())
        sampleView_.showNoResult();
    else
        sampleView_.showSampleList(result);
}

// ── 주문 승인 · 거절 ───────────────────────────────────────

void ProductionController::runOrderApprovalMenu() {
    while (true) {
        orderView_.showApprovalMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            listReservedOrders();
        else if (input == 2)
            approveOrder();
        else if (input == 3)
            rejectOrder();
        else
            mainView_.showInvalidInput();
    }
}

void ProductionController::listReservedOrders() {
    auto all = orderRepo_.findAll();
    std::vector<Order> reserved;
    for (const auto& o : all)
        if (o.status == OrderStatus::RESERVED)
            reserved.push_back(o);
    if (reserved.empty()) {
        orderView_.showNoOrders();
        return;
    }
    auto samples = sampleRepo_.findAll();
    orderView_.showOrderList(reserved, samples);
}

void ProductionController::approveOrder() {
    int id = orderView_.inputOrderId();
    auto found = orderRepo_.findById(id);
    if (!found) {
        orderView_.showInvalidInput("존재하지 않는 주문입니다.");
        return;
    }
    if (found->status != OrderStatus::RESERVED) {
        orderView_.showInvalidInput("RESERVED 상태의 주문만 승인할 수 있습니다.");
        return;
    }
    auto sample = sampleRepo_.findById(found->sampleId);
    if (sample && sample->stock >= found->quantity) {
        orderRepo_.updateStatus(id, OrderStatus::CONFIRMED);
        orderView_.showConfirmed(*found);
    } else {
        orderRepo_.updateStatus(id, OrderStatus::PRODUCING);
        orderView_.showSentToProduction(*found);
    }
}

void ProductionController::rejectOrder() {
    int id = orderView_.inputOrderId();
    auto found = orderRepo_.findById(id);
    if (!found) {
        orderView_.showInvalidInput("존재하지 않는 주문입니다.");
        return;
    }
    if (found->status != OrderStatus::RESERVED) {
        orderView_.showInvalidInput("RESERVED 상태의 주문만 거절할 수 있습니다.");
        return;
    }
    orderRepo_.updateStatus(id, OrderStatus::REJECTED);
    orderView_.showRejected(*found);
}

void ProductionController::runMonitor() {
    while (true) {
        monitorView_.showMonitorMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)      showOrderStatus();
        else if (input == 2) showStockStatus();
        else                 mainView_.showInvalidInput();
    }
}

void ProductionController::showOrderStatus() {
    monitorView_.showOrderStatus(orderRepo_.findAll());
}

void ProductionController::showStockStatus() {
    monitorView_.showStockStatus(sampleRepo_.findAll(), orderRepo_.findAll());
}

void ProductionController::runProductionMenu() {
    while (true) {
        productionView_.showProductionMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)      showProductionStatus();
        else if (input == 2) showProductionQueue();
        else                 mainView_.showInvalidInput();
    }
}

std::vector<ProductionEntry> ProductionController::buildProductionEntries(bool sortByUpdatedAt) {
    auto all = orderRepo_.findAll();
    std::vector<Order> producing;
    for (const auto& o : all)
        if (o.status == OrderStatus::PRODUCING)
            producing.push_back(o);

    if (sortByUpdatedAt)
        std::sort(producing.begin(), producing.end(),
            [](const Order& a, const Order& b) { return a.updatedAt < b.updatedAt; });

    std::vector<ProductionEntry> entries;
    for (const auto& o : producing) {
        auto s = sampleRepo_.findById(o.sampleId);
        if (!s) continue;
        int shortage  = o.quantity - s->stock;
        int actualQty = static_cast<int>(
            std::ceil(static_cast<double>(shortage) / (s->yield * 0.9)));
        entries.push_back({o.id, s->name, o.customerName,
                           o.quantity, shortage, actualQty,
                           s->avgProductionTime * actualQty, o.updatedAt});
    }
    return entries;
}

void ProductionController::showProductionStatus() {
    auto entries = buildProductionEntries(false);
    if (entries.empty()) { productionView_.showNoProductionOrders(); return; }
    productionView_.showProductionStatus(entries);
}

void ProductionController::showProductionQueue() {
    auto entries = buildProductionEntries(true);
    if (entries.empty()) { productionView_.showNoProductionOrders(); return; }
    productionView_.showProductionQueue(entries);
}
