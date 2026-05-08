#include "ProductionController.h"
#include <vector>

ProductionController::ProductionController(MainView& mainView,
                                           SampleView& sampleView,
                                           OrderView& orderView,
                                           IRepository<Sample>& sampleRepo,
                                           IOrderRepository& orderRepo)
    : mainView_(mainView), sampleView_(sampleView), orderView_(orderView),
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
        else if (input >= 3 && input <= 4)
            mainView_.showComingSoon();
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
