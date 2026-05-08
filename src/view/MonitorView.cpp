#include "MonitorView.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <string>

void MonitorView::showMonitorMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  [모니터링]\n";
    std::cout << "========================================\n";
    std::cout << " 1. 주문량 확인\n";
    std::cout << " 2. 재고량 확인\n";
    std::cout << " 0. 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void MonitorView::showOrderStatus(const std::vector<Order>& orders) {
    // REJECTED 제외, 4개 상태별 분류
    std::map<OrderStatus, std::vector<const Order*>> grouped;
    for (auto status : {OrderStatus::RESERVED, OrderStatus::CONFIRMED,
                        OrderStatus::PRODUCING, OrderStatus::RELEASED})
        grouped[status] = {};

    for (const auto& o : orders)
        if (o.status != OrderStatus::REJECTED)
            grouped[o.status].push_back(&o);

    std::cout << "\n  [주문량 현황]\n";
    for (auto status : {OrderStatus::RESERVED, OrderStatus::CONFIRMED,
                        OrderStatus::PRODUCING, OrderStatus::RELEASED}) {
        const auto& list = grouped[status];
        std::cout << "  " << toString(status) << ": " << list.size() << "건\n";
        for (const auto* o : list)
            std::cout << "    주문 " << o->id
                      << " | 고객: " << o->customerName
                      << " | 수량: " << o->quantity << "\n";
    }
}

void MonitorView::showStockStatus(const std::vector<Sample>& samples,
                                  const std::vector<Order>& orders) {
    // 유효 주문량 계산 (RESERVED+CONFIRMED+PRODUCING)
    std::map<int, int> validQty;
    for (const auto& o : orders)
        if (o.status == OrderStatus::RESERVED ||
            o.status == OrderStatus::CONFIRMED ||
            o.status == OrderStatus::PRODUCING)
            validQty[o.sampleId] += o.quantity;

    std::cout << "\n  [재고 현황]\n";
    std::cout << "  ┌────┬──────────────────────┬──────┬──────────┬──────────┐\n";
    std::cout << "  │ ID │ 시료명               │ 재고 │ 유효주문 │ 상태     │\n";
    std::cout << "  ├────┼──────────────────────┼──────┼──────────┼──────────┤\n";
    for (const auto& s : samples) {
        int vq = validQty.count(s.id) ? validQty[s.id] : 0;
        std::string status;
        if (s.stock == 0)        status = "❌ 고갈";
        else if (s.stock < vq)   status = "⚠️ 부족";
        else                     status = "✅ 여유";
        std::cout << "  │"
                  << std::setw(3)  << s.id           << " │"
                  << std::setw(21) << std::left << s.name << std::right << " │"
                  << std::setw(5)  << s.stock         << " │"
                  << std::setw(8)  << vq              << "  │"
                  << std::setw(8)  << std::left << status << std::right << "  │\n";
    }
    std::cout << "  └────┴──────────────────────┴──────┴──────────┴──────────┘\n";
}
