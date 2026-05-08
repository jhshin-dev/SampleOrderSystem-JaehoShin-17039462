# Phase 4 설계 — 주문 접수 (OR-01·02)

## 1. 목표

고객 주문을 접수하여 RESERVED 상태로 등록한다. 접수된 주문 목록을 확인할 수 있다.
데이터는 `data/orders.json`에 영속 저장된다.

---

## 2. 생성할 파일

```
model/
    OrderStatus.h             OrderStatus enum + isValidTransition()
    Order.h                   Order 구조체 + to_json / from_json
    IOrderRepository.h        IRepository<Order> + updateStatus() 확장 인터페이스
    OrderRepository.h
    OrderRepository.cpp       create() 시 RESERVED 강제, ISO 8601 타임스탬프 자동
view/
    OrderView.h
    OrderView.cpp
test/
    OrderRepositoryTest.cpp   OrderRepository 단위 테스트
    OR01OR02Test.cpp          OR-01·02 흐름 테스트
```

수정 파일:
```
controller/OrderController.h    SampleRepository·OrderRepository·OrderView 주입 추가
controller/OrderController.cpp  OR-01·02 구현
controller/AppController.h      OrderRepository 주입 추가
controller/AppController.cpp    OrderRepository 생성 후 OrderController 에 전달
main.cpp                        OrderRepository 인스턴스 생성
test/AppControllerTest.cpp      OrderController 생성자 변경에 따른 Mock 수정
SampleOrderSystem.vcxproj       새 .cpp 파일 등록
```

---

## 3. 도메인 모델

### OrderStatus.h

```cpp
#pragma once
#include <string>

enum class OrderStatus {
    RESERVED, CONFIRMED, PRODUCING, REJECTED, RELEASED
};

inline std::string toString(OrderStatus s) {
    switch (s) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::REJECTED:  return "REJECTED";
        case OrderStatus::RELEASED:  return "RELEASED";
    }
    return "UNKNOWN";
}

inline OrderStatus statusFromString(const std::string& s) {
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}

inline bool isValidTransition(OrderStatus from, OrderStatus to) {
    switch (from) {
        case OrderStatus::RESERVED:
            return to == OrderStatus::CONFIRMED
                || to == OrderStatus::PRODUCING
                || to == OrderStatus::REJECTED;
        case OrderStatus::PRODUCING:
            return to == OrderStatus::CONFIRMED;
        case OrderStatus::CONFIRMED:
            return to == OrderStatus::RELEASED;
        default:
            return false;
    }
}
```

### Order.h

```cpp
#pragma once
#include <string>
#include "OrderStatus.h"
#include "../lib/json.hpp"

struct Order {
    int         id           = 0;
    int         sampleId     = 0;
    std::string customerName;
    int         quantity     = 0;
    OrderStatus status       = OrderStatus::RESERVED;
    std::string createdAt;
    std::string updatedAt;

    bool operator==(const Order& o) const {
        return id == o.id;
    }
};

inline void to_json(nlohmann::json& j, const Order& o) {
    j = { {"id", o.id}, {"sampleId", o.sampleId},
          {"customerName", o.customerName}, {"quantity", o.quantity},
          {"status", toString(o.status)},
          {"createdAt", o.createdAt}, {"updatedAt", o.updatedAt} };
}

inline void from_json(const nlohmann::json& j, Order& o) {
    j.at("id").get_to(o.id);
    j.at("sampleId").get_to(o.sampleId);
    j.at("customerName").get_to(o.customerName);
    j.at("quantity").get_to(o.quantity);
    o.status = statusFromString(j.at("status").get<std::string>());
    j.at("createdAt").get_to(o.createdAt);
    j.at("updatedAt").get_to(o.updatedAt);
}
```

---

## 4. Repository 계층

### IOrderRepository.h — 확장 인터페이스

```cpp
#pragma once
#include "IRepository.h"
#include "Order.h"

class IOrderRepository : public IRepository<Order> {
public:
    virtual bool updateStatus(int id, OrderStatus newStatus) = 0;
};
```

> `IRepository<Order>` CRUD + `updateStatus()` 를 포함한 인터페이스.
> OrderController는 이 인터페이스를 주입받아 Mock 테스트가 가능하다.

### OrderRepository.h

```cpp
#pragma once
#include "IOrderRepository.h"
#include "JsonRepository.h"

class OrderRepository : public JsonRepository<Order>, public IOrderRepository {
public:
    explicit OrderRepository(const std::string& filePath = "data/orders.json");
    Order create(Order entity) override;        // RESERVED 강제, 타임스탬프 자동
    bool  remove(int id)       override;        // RELEASED 삭제 불가
    bool  updateStatus(int id, OrderStatus newStatus) override;

    // 다중 상속(JsonRepository + IOrderRepository) 모호성 해결용 포워딩
    std::vector<Order>   findAll()              override;
    std::optional<Order> findById(int id)       override;
    bool                 update(const Order& o) override;

private:
    static std::string nowIso8601();
};
```

**핵심 동작:**
- `create()` → status = RESERVED 강제, createdAt = updatedAt = 현재 ISO 8601
- `remove()` → RELEASED 상태면 거부(false 반환)
- `updateStatus()` → `isValidTransition()` 검증 후 status·updatedAt 갱신

---

## 5. View — OrderView

```cpp
// view/OrderView.h
#pragma once
#include <string>
#include <vector>
#include "../model/Order.h"
#include "../model/Sample.h"

class OrderView {
public:
    virtual ~OrderView() = default;
    virtual void        showOrderMenu();
    virtual int         inputSampleId();
    virtual std::string inputCustomerName();
    virtual int         inputQuantity();
    virtual void        showOrderRegistered(const Order& o);
    virtual void        showOrderList(const std::vector<Order>& orders,
                                     const std::vector<Sample>& samples);
    virtual void        showNoOrders();
    virtual void        showInvalidInput(const std::string& msg);
    virtual void        showComingSoon();
};
```

**showOrderList() 출력 형식:**
```
  ┌──────┬───────────────┬──────────────┬────┬──────────────────────┐
  │ 주문 │ 시료명        │ 고객명       │ 수량 │ 접수일시             │
  ├──────┼───────────────┼──────────────┼────┼──────────────────────┤
  │    1 │ Silicon-A     │ KimTest      │  5 │ 2026-05-08T10:00:00Z │
  └──────┴───────────────┴──────────────┴────┴──────────────────────┘
```

---

## 6. Controller 변경

### OrderController

```cpp
// controller/OrderController.h
#pragma once
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"

class OrderController {
public:
    OrderController(MainView& mainView,
                    OrderView& orderView,
                    IRepository<Sample>& sampleRepo,
                    IOrderRepository& orderRepo);
    void run();
private:
    MainView&            mainView_;
    OrderView&           orderView_;
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;

    void receiveOrder();           // OR-01
    void listReservedOrders();     // OR-02
};
```

**OR-01 receiveOrder() 검증 규칙:**

| 항목 | 조건 | 오류 메시지 |
|------|------|------------|
| 시료 ID | SampleRepository에 존재해야 함 | "존재하지 않는 시료입니다." |
| 고객명 | 비어있으면 거부 | "고객명을 입력해주세요." |
| 수량 | 1 이상 | "수량은 1 이상이어야 합니다." |

**OrderController::run() 흐름:**
```
루프
├─ mainView_.showOrderManagerMenu(0, 0)
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → receiveOrder()       (OR-01)
├─ 2 → listReservedOrders() (OR-02)
├─ 3~4 → orderView_.showComingSoon()
└─ else → mainView_.showInvalidInput()
```

### AppController 변경

```cpp
AppController(MainView& mainView,
              OrderView& orderView,
              SampleView& sampleView,
              IRepository<Sample>& sampleRepo,
              OrderRepository& orderRepo);
```

### main.cpp

```cpp
SampleRepository sampleRepo;
OrderRepository  orderRepo;
MainView   mainView;
OrderView  orderView;
SampleView sampleView;
AppController app(mainView, orderView, sampleView, sampleRepo, orderRepo);
app.run();
```

---

## 7. vcxproj 신규 등록

```xml
<ClCompile Include="model\OrderRepository.cpp" />
<!-- IOrderRepository.h는 헤더 온리 인터페이스 — ClCompile 불필요 -->
<ClCompile Include="view\OrderView.cpp" />
<ClCompile Include="test\OrderRepositoryTest.cpp" />
<ClCompile Include="test\OR01OR02Test.cpp" />
```

---

## 8. 테스트 계획 (gtest)

### OrderRepositoryTest

| 테스트 ID | 설명 |
|-----------|------|
| `OrderRepositoryTest.CreateSetsStatusToReserved` | create 후 status == RESERVED |
| `OrderRepositoryTest.CreateSetsTimestamps` | createdAt·updatedAt 비어있지 않음 |
| `OrderRepositoryTest.FindAllReturnsCreated` | create 후 findAll에 포함 |
| `OrderRepositoryTest.PersistsAcrossInstances` | 재시작 후 데이터 유지 |
| `OrderRepositoryTest.RemoveReleasedOrderFails` | RELEASED 주문 삭제 → false |
| `OrderRepositoryTest.UpdateStatusValid` | RESERVED→CONFIRMED 성공 |
| `OrderRepositoryTest.UpdateStatusInvalid` | CONFIRMED→RESERVED 실패 |

### OR01OR02Test — MockOrderView + MockOrderRepository

| 테스트 ID | 설명 |
|-----------|------|
| `OR01Test.ReceiveOrderSuccess` | 유효 입력 → create 호출, showOrderRegistered 호출 |
| `OR01Test.RejectsInvalidSampleId` | 미존재 시료 ID → showInvalidInput, create 미호출 |
| `OR01Test.RejectsEmptyCustomerName` | 빈 고객명 → showInvalidInput |
| `OR01Test.RejectsZeroQuantity` | 수량 0 → showInvalidInput |
| `OR02Test.ShowsReservedOrders` | RESERVED 주문 → showOrderList 호출 |
| `OR02Test.ShowsEmptyWhenNoOrders` | 빈 목록 → showNoOrders 호출 |

---

## 9. 구현 순서 (TDD 사이클)

```
1. OrderStatus.h, Order.h 작성
2. OrderRepositoryTest 작성 → RED
3. OrderRepository 구현 → GREEN
4. OrderView 작성 (pure I/O)
5. OR01OR02Test 작성 → RED
6. OrderController::receiveOrder() / listReservedOrders() 구현 → GREEN
7. AppController·main.cpp 업데이트
8. AppControllerTest 기존 테스트 수정
9. vcxproj 등록
10. 전체 빌드 + 테스트 확인
```

---

## 10. 검토 포인트

- [확정] `IOrderRepository` 인터페이스 신설(updateStatus 포함) — Phase 2 패턴과 일치, Mock 가능
- `AppController` 생성자 인수가 늘어남 (mainView, orderView, sampleView, sampleRepo, orderRepo)
  — 추후 구조체로 묶는 리팩터링을 고려하는가, 아니면 현재 방식 유지하는가?
- `OrderView::showOrderList()`에서 시료명을 표시하기 위해 `SampleRepository`를 직접 받는 대신
  `std::vector<Sample>`을 함께 전달하는 방식 — 동의하는가?
