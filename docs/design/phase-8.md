# Phase 8 설계 — 생산 현황 · 대기 큐 (PL-01·02)

## 1. 목표

재고 부족으로 생산 중인 주문의 현황(PL-01)과 FIFO 대기 큐(PL-02)를 확인한다.
수율 기반 실 생산량·총 생산시간을 화면에 표시한다.

---

## 2. 생성할 파일

신규 파일:
```
view/
    ProductionView.h
    ProductionView.cpp
test/
    PL01PL02Test.cpp
```

수정 파일:
```
controller/ProductionController.h     ProductionView& 주입 추가,
                                      runProductionMenu(), showProductionStatus(), showProductionQueue() 추가
controller/ProductionController.cpp   메뉴 3번 showComingSoon → runProductionMenu(), 위 메서드 구현
controller/AppController.h            ProductionView& 주입 추가
controller/AppController.cpp          ProductionView 생성 후 ProductionController에 전달
main.cpp                              ProductionView 인스턴스 생성
test/AppControllerTest.cpp            새 생성자에 따른 Mock 수정
test/OR03OR04Test.cpp                 ProductionController 생성자 수정
test/SM01Test.cpp                     ProductionController 생성자 수정
test/SM02SM03Test.cpp                 ProductionController 생성자 수정
SampleOrderSystem.vcxproj            새 .cpp 파일 등록
```

---

## 3. 생산량 계산 공식

```
부족분       = quantity - stock
실 생산량    = ⌈부족분 / (yield × 0.9)⌉    (cmath::ceil 사용)
총 생산시간  = avgProductionTime × 실 생산량  (분)
```

> 계산은 **ProductionView** 내부에서 처리한다.
> Controller는 PRODUCING 주문 목록과 시료 정보를 전달하고,
> View가 각 주문별 계산 및 표시를 담당한다.
> (MonitorView의 재고 상태 판정과 동일한 패턴)

---

## 4. ProductionView

```cpp
// view/ProductionView.h
#pragma once
#include <vector>
#include "../model/Order.h"
#include "../model/Sample.h"

class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showProductionMenu();
    virtual void showProductionStatus(const std::vector<Order>& producingOrders,
                                      const std::vector<Sample>& samples);
    virtual void showProductionQueue(const std::vector<Order>& producingOrders,
                                     const std::vector<Sample>& samples);
    virtual void showNoProductionOrders();
};
```

**showProductionStatus() 출력 형식:**

```
  ┌──────┬──────────────┬──────────┬──────┬──────┬──────────┬──────────┐
  │ 주문 │ 시료명       │ 고객명   │ 수량 │ 부족 │ 실생산량 │ 생산시간 │
  ├──────┼──────────────┼──────────┼──────┼──────┼──────────┼──────────┤
  │    2 │ Silicon-A    │ LeeCo    │    2 │    2 │        3 │    30분  │
  └──────┴──────────────┴──────────┴──────┴──────┴──────────┴──────────┘
```

**showProductionQueue():** updatedAt 오름차순 정렬 후 순번과 함께 출력.
(정렬은 Controller에서 전달 전에 수행, View는 받은 순서대로 출력)

---

## 5. ProductionController 변경

### 생성자

```cpp
ProductionController(MainView& mainView,
                     SampleView& sampleView,
                     OrderView& orderView,
                     MonitorView& monitorView,
                     ProductionView& productionView,
                     IRepository<Sample>& sampleRepo,
                     IOrderRepository& orderRepo);
```

### 메뉴 3번 변경

```
run() 메뉴 3번:
  기존: mainView_.showComingSoon()
  변경: runProductionMenu()
```

### runProductionMenu() 흐름

```
루프
├─ productionView_.showProductionMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → showProductionStatus()   (PL-01)
├─ 2 → showProductionQueue()    (PL-02)
└─ else → mainView_.showInvalidInput()
```

### showProductionStatus()

```cpp
auto all = orderRepo_.findAll();
std::vector<Order> producing;
for (const auto& o : all)
    if (o.status == OrderStatus::PRODUCING)
        producing.push_back(o);

if (producing.empty()) { productionView_.showNoProductionOrders(); return; }
productionView_.showProductionStatus(producing, sampleRepo_.findAll());
```

### showProductionQueue()

```cpp
auto all = orderRepo_.findAll();
std::vector<Order> producing;
for (const auto& o : all)
    if (o.status == OrderStatus::PRODUCING)
        producing.push_back(o);

// FIFO: updatedAt 오름차순 정렬
std::sort(producing.begin(), producing.end(),
    [](const Order& a, const Order& b) { return a.updatedAt < b.updatedAt; });

if (producing.empty()) { productionView_.showNoProductionOrders(); return; }
productionView_.showProductionQueue(producing, sampleRepo_.findAll());
```

---

## 6. AppController 변경

```cpp
AppController(MainView& mainView,
              OrderView& orderView,
              MonitorView& monitorView,
              SampleView& sampleView,
              ProductionView& productionView,
              IRepository<Sample>& sampleRepo,
              IOrderRepository& orderRepo);
```

---

## 7. vcxproj 신규 등록

```xml
<ClCompile Include="view\ProductionView.cpp" />
<ClCompile Include="test\PL01PL02Test.cpp" />
```

---

## 8. 테스트 계획 (gtest)

### PL01PL02Test — MockProductionView 사용

```cpp
class MockProductionView : public ProductionView {
    MOCK_METHOD(void, showProductionMenu,   (), (override));
    MOCK_METHOD(void, showProductionStatus, (const std::vector<Order>&,
                                             const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showProductionQueue,  (const std::vector<Order>&,
                                             const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoProductionOrders, (), (override));
};
```

| 테스트 ID | 설명 |
|-----------|------|
| `PL01Test.ShowsProductionStatus` | PRODUCING 주문 → showProductionStatus 호출 |
| `PL01Test.ShowsNoOrdersWhenEmpty` | PRODUCING 없음 → showNoProductionOrders 호출 |
| `PL02Test.ShowsProductionQueueSortedByUpdatedAt` | updatedAt 오름차순 정렬 후 showProductionQueue 호출 |
| `PL02Test.ShowsNoQueueWhenEmpty` | PRODUCING 없음 → showNoProductionOrders 호출 |

### 기존 테스트 수정

- `AppControllerTest.*` — AppController 7인수 생성자, MockProductionView 추가
- `OR03OR04Test.*` — ProductionController 7인수 생성자, MockProductionView 추가
- `SM01Test.*` / `SM02SM03Test.*` — 동일

---

## 9. 구현 순서 (TDD 사이클)

```
1. ProductionView.h / ProductionView.cpp 작성
2. PL01PL02Test 작성 → RED
3. ProductionController::runProductionMenu() / showProductionStatus() / showProductionQueue() → GREEN
4. AppController 생성자 업데이트 + main.cpp 수정
5. 기존 테스트 생성자 일괄 수정
6. vcxproj 등록
7. 전체 빌드 + 테스트 확인
```

---

## 10. 검토 포인트

- 생산량 계산을 View에서 처리하는 방식에 동의하는가?
  (Controller는 PRODUCING 주문 + samples 전달, View가 계산·출력)
- PL-02 FIFO 정렬을 Controller에서 수행 후 View에 전달하는 방식에 동의하는가?
