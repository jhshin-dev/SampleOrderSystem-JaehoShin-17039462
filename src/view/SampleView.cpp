#include "SampleView.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>

void SampleView::showSampleMenu() {
    std::cout << "\n========================================\n";
    std::cout << "  [시료 관리]\n";
    std::cout << "========================================\n";
    std::cout << " 1. 시료 등록\n";
    std::cout << " 2. 시료 조회\n";
    std::cout << " 3. 시료 검색\n";
    std::cout << " 0. 돌아가기\n";
    std::cout << "========================================\n";
    std::cout << "선택: ";
}

std::string SampleView::inputName() {
    std::cout << "  시료 이름: ";
    std::string name;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, name);
    return name;
}

int SampleView::inputAvgProductionTime() {
    std::cout << "  평균 생산시간 (분): ";
    int v;
    if (std::cin >> v) return v;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

double SampleView::inputYield() {
    std::cout << "  수율 (0.0 초과 1.0 이하): ";
    double v;
    if (std::cin >> v) return v;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1.0;
}

void SampleView::showRegistered(const Sample& s) {
    std::cout << "\n  [등록 완료]\n";
    std::cout << "  ID: " << s.id
              << "  이름: " << s.name
              << "  생산시간: " << s.avgProductionTime << "분"
              << "  수율: " << s.yield << "\n";
}

void SampleView::showInvalidInput(const std::string& msg) {
    std::cout << "\n  [오류] " << msg << "\n";
}

void SampleView::showComingSoon() {
    std::cout << "\n  [준비 중입니다. 다음 버전에서 제공됩니다.]\n";
}

void SampleView::showSampleList(const std::vector<Sample>& samples) {
    if (samples.empty()) {
        std::cout << "\n  등록된 시료가 없습니다.\n";
        return;
    }
    std::cout << "\n";
    std::cout << "  ┌────┬──────────────────────┬──────────┬──────┬──────┐\n";
    std::cout << "  │ ID │ 이름                 │ 생산시간 │ 수율 │ 재고 │\n";
    std::cout << "  ├────┼──────────────────────┼──────────┼──────┼──────┤\n";
    for (const auto& s : samples) {
        std::cout << "  │"
                  << std::setw(3) << s.id       << " │"
                  << std::setw(21) << std::left << s.name << std::right << " │"
                  << std::setw(7)  << s.avgProductionTime << "분  │"
                  << std::setw(5)  << std::fixed << std::setprecision(2) << s.yield << " │"
                  << std::setw(5)  << s.stock    << " │\n";
    }
    std::cout << "  └────┴──────────────────────┴──────────┴──────┴──────┘\n";
}

std::string SampleView::inputSearchKeyword() {
    std::cout << "  검색어: ";
    std::string keyword;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, keyword);
    return keyword;
}

void SampleView::showNoResult() {
    std::cout << "\n  검색 결과가 없습니다.\n";
}
