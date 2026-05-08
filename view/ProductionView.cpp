#include "ProductionView.h"
#include <iostream>
#include <iomanip>

void ProductionView::showProductionMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  [생산 라인]\n";
    std::cout << "========================================\n";
    std::cout << " 1. 생산 현황\n";
    std::cout << " 2. 생산 대기 큐\n";
    std::cout << " 0. 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void ProductionView::showProductionStatus(const std::vector<ProductionEntry>& entries) {
    std::cout << "\n  [생산 현황]\n";
    std::cout << "  ┌──────┬──────────────┬──────────┬──────┬──────┬──────────┬──────────┐\n";
    std::cout << "  │ 주문 │ 시료명       │ 고객명   │ 수량 │ 부족 │ 실생산량 │ 생산시간 │\n";
    std::cout << "  ├──────┼──────────────┼──────────┼──────┼──────┼──────────┼──────────┤\n";
    for (const auto& e : entries) {
        std::cout << "  │"
                  << std::setw(5)  << e.orderId      << " │"
                  << std::setw(13) << std::left << e.sampleName   << std::right << " │"
                  << std::setw(9)  << std::left << e.customerName << std::right << " │"
                  << std::setw(5)  << e.quantity     << " │"
                  << std::setw(5)  << e.shortage     << " │"
                  << std::setw(9)  << e.actualQty    << " │"
                  << std::setw(7)  << e.totalTime    << "분  │\n";
    }
    std::cout << "  └──────┴──────────────┴──────────┴──────┴──────┴──────────┴──────────┘\n";
}

void ProductionView::showProductionQueue(const std::vector<ProductionEntry>& entries) {
    std::cout << "\n  [생산 대기 큐] (FIFO 순서)\n";
    std::cout << "  ┌──────┬──────┬──────────────┬──────────┬──────┬──────────┬──────────┐\n";
    std::cout << "  │ 순번 │ 주문 │ 시료명       │ 고객명   │ 수량 │ 실생산량 │ 생산시간 │\n";
    std::cout << "  ├──────┼──────┼──────────────┼──────────┼──────┼──────────┼──────────┤\n";
    int seq = 1;
    for (const auto& e : entries) {
        std::cout << "  │"
                  << std::setw(5)  << seq++          << " │"
                  << std::setw(5)  << e.orderId      << " │"
                  << std::setw(13) << std::left << e.sampleName   << std::right << " │"
                  << std::setw(9)  << std::left << e.customerName << std::right << " │"
                  << std::setw(5)  << e.quantity     << " │"
                  << std::setw(9)  << e.actualQty    << " │"
                  << std::setw(7)  << e.totalTime    << "분  │\n";
    }
    std::cout << "  └──────┴──────┴──────────────┴──────────┴──────┴──────────┴──────────┘\n";
}

void ProductionView::showNoProductionOrders() {
    std::cout << "\n  현재 생산 중인 주문이 없습니다.\n";
}
