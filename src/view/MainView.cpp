#include "MainView.h"
#include <iostream>
#include <limits>

void MainView::showRoleMenu(int sampleCount, int totalStock) {
    std::cout << "\n========================================\n";
    std::cout << "  반도체 시료 생산주문관리 시스템\n";
    std::cout << "  [요약] 시료: " << sampleCount << "종  총 재고: " << totalStock << "개\n";
    std::cout << "========================================\n";
    std::cout << " 1. 주문 담당자\n";
    std::cout << " 2. 생산 담당자\n";
    std::cout << " 0. 종료\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void MainView::showOrderManagerMenu(int sampleCount, int totalStock) {
    std::cout << "\n========================================\n";
    std::cout << "  [주문 담당자]  시료: " << sampleCount << "종  총 재고: " << totalStock << "개\n";
    std::cout << "========================================\n";
    std::cout << " 1. 주문 접수\n";
    std::cout << " 2. 접수 주문 목록\n";
    std::cout << " 3. 출고 처리\n";
    std::cout << " 4. 모니터링\n";
    std::cout << " 0. 역할 선택으로 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void MainView::showProductionManagerMenu(int sampleCount, int totalStock) {
    std::cout << "\n========================================\n";
    std::cout << "  [생산 담당자]  시료: " << sampleCount << "종  총 재고: " << totalStock << "개\n";
    std::cout << "========================================\n";
    std::cout << " 1. 시료 관리\n";
    std::cout << " 2. 주문 승인 · 거절\n";
    std::cout << " 3. 생산 라인\n";
    std::cout << " 4. 모니터링\n";
    std::cout << " 0. 역할 선택으로 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

void MainView::showComingSoon() {
    std::cout << "\n  [준비 중입니다. 다음 버전에서 제공됩니다.]\n";
}

void MainView::showInvalidInput() {
    std::cout << "\n  [올바른 메뉴를 선택해주세요.]\n";
}

int MainView::getMenuInput() {
    int value;
    if (std::cin >> value) {
        return value;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}
