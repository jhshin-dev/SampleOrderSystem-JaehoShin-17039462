#include "OrderView.h"
#include <iostream>
#include <iomanip>
#include <limits>

int OrderView::inputSampleId() {
    std::cout << "  시료 ID: ";
    int v;
    if (std::cin >> v) return v;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

std::string OrderView::inputCustomerName() {
    std::cout << "  고객명: ";
    std::string name;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, name);
    return name;
}

int OrderView::inputQuantity() {
    std::cout << "  주문 수량: ";
    int v;
    if (std::cin >> v) return v;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

void OrderView::showOrderRegistered(const Order& o) {
    std::cout << "\n  [접수 완료]\n";
    std::cout << "  주문 ID: " << o.id
              << "  고객명: " << o.customerName
              << "  수량: "   << o.quantity
              << "  상태: RESERVED\n";
}

void OrderView::showOrderList(const std::vector<Order>& orders,
                              const std::vector<Sample>& samples) {
    if (orders.empty()) { showNoOrders(); return; }

    auto sampleName = [&](int id) -> std::string {
        for (const auto& s : samples)
            if (s.id == id) return s.name;
        return "(알 수 없음)";
    };

    std::cout << "\n";
    std::cout << "  ┌──────┬──────────────────────┬──────────────┬────┬──────────────────────┐\n";
    std::cout << "  │ 주문 │ 시료명               │ 고객명       │수량│ 접수일시             │\n";
    std::cout << "  ├──────┼──────────────────────┼──────────────┼────┼──────────────────────┤\n";
    for (const auto& o : orders) {
        std::cout << "  │"
                  << std::setw(5)  << o.id            << " │"
                  << std::setw(21) << std::left  << sampleName(o.sampleId) << std::right << " │"
                  << std::setw(13) << std::left  << o.customerName          << std::right << " │"
                  << std::setw(3)  << o.quantity       << " │"
                  << std::setw(21) << std::left  << o.createdAt             << std::right << " │\n";
    }
    std::cout << "  └──────┴──────────────────────┴──────────────┴────┴──────────────────────┘\n";
}

void OrderView::showNoOrders() {
    std::cout << "\n  접수된 주문이 없습니다.\n";
}

void OrderView::showInvalidInput(const std::string& msg) {
    std::cout << "\n  [오류] " << msg << "\n";
}

void OrderView::showComingSoon() {
    std::cout << "\n  [준비 중입니다. 다음 버전에서 제공됩니다.]\n";
}

void OrderView::showApprovalMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  [주문 승인 · 거절]\n";
    std::cout << "========================================\n";
    std::cout << " 1. 접수 주문 목록\n";
    std::cout << " 2. 주문 승인\n";
    std::cout << " 3. 주문 거절\n";
    std::cout << " 0. 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

int OrderView::inputOrderId() {
    std::cout << "  주문 ID: ";
    int v;
    if (std::cin >> v) return v;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

void OrderView::showConfirmed(const Order& o) {
    std::cout << "\n  [승인 완료] 주문 " << o.id
              << "이(가) CONFIRMED 상태로 전환되었습니다. (즉시 출고 대기)\n";
}

void OrderView::showSentToProduction(const Order& o) {
    std::cout << "\n  [생산 등록] 재고 부족으로 주문 " << o.id
              << "이(가) 생산 라인에 등록되었습니다. (PRODUCING)\n";
}

void OrderView::showRejected(const Order& o) {
    std::cout << "\n  [거절 완료] 주문 " << o.id
              << "이(가) REJECTED 상태로 전환되었습니다.\n";
}

void OrderView::showReleaseMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  [출고 처리]\n";
    std::cout << "========================================\n";
    std::cout << " 1. CONFIRMED 주문 목록\n";
    std::cout << " 2. 출고 실행\n";
    std::cout << " 0. 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void OrderView::showReleased(const Order& o) {
    std::cout << "\n  [출고 완료] 주문 " << o.id
              << "이(가) RELEASED 상태로 전환되었습니다.\n";
}

void OrderView::showNoConfirmedOrders() {
    std::cout << "\n  출고 가능한 주문이 없습니다.\n";
}
