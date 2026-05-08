# Phase 6 설계 — 모니터링 (MO-01·02)

## 1. 목표

주문 담당자·생산 담당자 모두 시스템 전체 상태를 한눈에 파악할 수 있다.
주문 상태별 현황(MO-01)과 시료별 재고 상태(MO-02)를 확인한다.

---

## 2. 생성할 파일

신규 파일:
```
view/
    MonitorView.h
    MonitorView.cpp
test/
    MO01MO02Test.cpp
```

수정 파일:
```
controller/OrderController.h      MonitorView& 주입 추가, showMonitorMenu(), showOrderStatus(), showStockStatus() 추가
controller/OrderController.cpp    위 메서드 구현
controller/ProductionController.h MonitorView& 주입 추가, 동일 메서드 추가
controller/ProductionController.cpp 위 메서드 구현
controller/AppController.h        MonitorView& 주입 추가
controller/AppController.cpp      MonitorView 생성 후 양쪽 컨트롤러에 전달
main.cpp                          MonitorView 인스턴스 생성
test/AppControllerTest.cpp        새 생성자에 따른 Mock 수정
test/OR01OR02Test.cpp             OrderController 생성자 수정
test/OR03OR04Test.cpp             ProductionController 생성자 수정 (이미 5개)
SampleOrderSystem.vcxproj         새 .cpp 파일 등록
```

---

## 3. MonitorView

```cpp
// view/MonitorView.h
#pragma once
#include <vector>
#include "../model/Order.h"
#include "../model/Sample.h"

class MonitorView {
public:
    virtual ~MonitorView() = default;
    virtual void showMonitorMenu();
    virtual void showOrderStatus(const std::vector<Order>& orders);
    virtual void showStockStatus(const std::vector<Sample>& samples,
                                 const std::vector<Order>& orders);
};
```

**showOrderStatus() 출력:**
- REJECTED 제외 4개 상태(RESERVED·CONFIRMED·PRODUCING·RELEASED)별 건수 요약
- 각 상태별 주문 목록 (주문 ID·시료명·고객명·수량)
- 해당 상태 주문이 없으면 "없음" 표시

**showStockStatus() 출력:**

| 시료 ID | 시료명 | 재고 | 유효 주문량 | 상태 |
|---------|--------|------|------------|------|
| 1 | Silicon-A | 0 | 10 | ❌ 고갈 |

재고 상태 기준 (판정 우선순위):
1. `stock == 0` → ❌ 고갈
2. `stock < 유효 주문량` → ⚠️ 부족
3. `stock >= 유효 주문량` → ✅ 여유

> 유효 주문 = RESERVED + CONFIRMED + PRODUCING 상태 주문의 sampleId별 수량 합산

---

## 4. Controller 변경

### OrderController

```cpp
OrderController(MainView& mainView,
                OrderView& orderView,
                MonitorView& monitorView,
                IRepository<Sample>& sampleRepo,
                IOrderRepository& orderRepo);
```

run() 메뉴 4번 → `runMonitor()`

**runMonitor() 흐름:**
```
루프
├─ monitorView_.showMonitorMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → showOrderStatus()   (MO-01)
├─ 2 → showStockStatus()   (MO-02)
└─ else → mainView_.showInvalidInput()
```

**showOrderStatus():**
```cpp
void OrderController::showOrderStatus() {
    auto orders = orderRepo_.findAll();
    monitorView_.showOrderStatus(orders);  // REJECTED 필터는 View 내부에서 처리
}
```

**showStockStatus():**
```cpp
void OrderController::showStockStatus() {
    auto samples = sampleRepo_.findAll();
    auto orders  = orderRepo_.findAll();
    monitorView_.showStockStatus(samples, orders);
}
```

### ProductionController

동일 구조. MonitorView& 주입 추가, run() 메뉴 4번 → `runMonitor()`.

---

## 5. AppController 변경

```cpp
AppController(MainView& mainView,
              OrderView& orderView,
              MonitorView& monitorView,
              SampleView& sampleView,
              IRepository<Sample>& sampleRepo,
              IOrderRepository& orderRepo);
```

---

## 6. vcxproj 신규 등록

```xml
<ClCompile Include="view\MonitorView.cpp" />
<ClCompile Include="test\MO01MO02Test.cpp" />
```

---

## 7. 테스트 계획 (gtest)

### MO01MO02Test — MockMonitorView 사용

```cpp
class MockMonitorView : public MonitorView {
    MOCK_METHOD(void, showMonitorMenu, (), (override));
    MOCK_METHOD(void, showOrderStatus, (const std::vector<Order>&), (override));
    MOCK_METHOD(void, showStockStatus, (const std::vector<Sample>&,
                                        const std::vector<Order>&), (override));
};
```

| 테스트 ID | 설명 |
|-----------|------|
| `MO01Test.ShowsOrderStatusExcludingRejected` | REJECTED 포함 목록 전달 → showOrderStatus 호출됨 (필터링은 View 내부) |
| `MO01Test.ShowsEmptyOrderStatus` | 주문 없음 → showOrderStatus([]) 호출 |
| `MO02Test.ShowsStockStatus` | 시료·주문 목록 → showStockStatus 호출 |
| `MO02Test.ShowsStockStatusWithNoOrders` | 주문 없음 → showStockStatus(samples, []) 호출 |

### 기존 테스트 수정

- `AppControllerTest.*` — AppController 6인수 생성자, MockMonitorView 추가
- `OR01OR02Test.*` — OrderController 5인수 생성자, MockMonitorView 추가
- `OR03OR04Test.*` — ProductionController 6인수 생성자, MockMonitorView 추가
- `SM01Test.*` / `SM02SM03Test.*` — ProductionController 생성자 수정

---

## 8. 구현 순서 (TDD 사이클)

```
1. MonitorView.h / MonitorView.cpp 작성
2. MO01MO02Test 작성 → RED
3. OrderController / ProductionController showOrderStatus() / showStockStatus() 구현 → GREEN
4. AppController 생성자 업데이트 + main.cpp 수정
5. 기존 테스트 생성자 일괄 수정
6. vcxproj 등록
7. 전체 빌드 + 테스트 확인
```

---

## 9. 검토 포인트

- REJECTED 필터링을 Controller가 할지, View가 할지?
  → **View가 처리**: Controller는 전체 주문 목록을 전달하고 View 내부에서 REJECTED를 제외해 출력
  (단위 테스트에서 Controller는 단순히 findAll()→showOrderStatus() 호출만 검증)
- 재고 상태 판정(여유·부족·고갈)을 View가 할지 Controller가 할지?
  → **View가 처리**: Controller는 samples와 orders를 전달하고 View가 유효 주문량 계산 및 판정
