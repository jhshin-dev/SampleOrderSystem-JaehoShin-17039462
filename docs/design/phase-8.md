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

## 3. 생산량 계산 공식 및 데이터 구조

```
부족분       = quantity - stock
실 생산량    = ⌈부족분 / (yield × 0.9)⌉    (cmath::ceil 사용)
총 생산시간  = avgProductionTime × 실 생산량  (분)
```

**계산은 Controller에서 수행한다.** View는 전달받은 데이터를 출력만 담당한다.

```cpp
// model/ProductionEntry.h  (헤더 온리, .cpp 불필요)
#pragma once
#include <string>

struct ProductionEntry {
    int         orderId;
    std::string sampleName;
    std::string customerName;
    int         quantity;
    int         shortage;
    int         actualQty;
    int         totalTime;
    std::string updatedAt;   // FIFO 정렬 기준
};
```

Controller가 PRODUCING 주문마다 `ProductionEntry`를 계산하여 vector로 전달한다.

---

## 4. ProductionView

```cpp
// view/ProductionView.h
#pragma once
#include <vector>
#include "../model/ProductionEntry.h"

class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showProductionMenu();
    virtual void showProductionStatus(const std::vector<ProductionEntry>& entries);
    virtual void showProductionQueue(const std::vector<ProductionEntry>& entries);
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

**showProductionQueue():** Controller가 updatedAt 오름차순 정렬 후 전달. View는 순번과 함께 출력.

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

### buildEntries() — 공통 헬퍼

```cpp
std::vector<ProductionEntry> buildProductionEntries(bool sortByUpdatedAt) {
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
            std::ceil(shortage / (s->yield * 0.9)));
        entries.push_back({o.id, s->name, o.customerName,
                           o.quantity, shortage, actualQty,
                           s->avgProductionTime * actualQty, o.updatedAt});
    }
    return entries;
}
```

### showProductionStatus()

```cpp
auto entries = buildProductionEntries(false);
if (entries.empty()) { productionView_.showNoProductionOrders(); return; }
productionView_.showProductionStatus(entries);
```

### showProductionQueue()

```cpp
auto entries = buildProductionEntries(true);  // updatedAt 오름차순
if (entries.empty()) { productionView_.showNoProductionOrders(); return; }
productionView_.showProductionQueue(entries);
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

> `model/ProductionEntry.h`는 헤더 온리 — `<ClCompile>` 등록 불필요.

---

## 8. 테스트 계획 (gtest)

### PL01PL02Test — MockProductionView 사용

```cpp
class MockProductionView : public ProductionView {
    MOCK_METHOD(void, showProductionMenu,     (), (override));
    MOCK_METHOD(void, showProductionStatus,   (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showProductionQueue,    (const std::vector<ProductionEntry>&), (override));
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

## 10. 확정 사항

- [확정] **Controller 계산, View 출력**: Controller가 `ProductionEntry` 계산·정렬, View는 순수 출력만 담당
- [확정] **FIFO 정렬 기준**: `updatedAt` (생산 라인 등록 시각) 오름차순
