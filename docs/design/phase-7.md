# Phase 7 설계 — 출고 처리 (RL-01·02)

## 1. 목표

CONFIRMED 상태의 주문을 최종 출고(RELEASED) 처리한다.
출고 시 해당 시료의 재고를 차감한다. 주문 생애주기의 마지막 단계가 완성된다.

---

## 2. 생성할 파일

신규 파일:
```
test/
    RL01RL02Test.cpp
```

수정 파일:
```
view/OrderView.h            showReleaseMenu(), showReleased(), showNoConfirmedOrders() 추가
view/OrderView.cpp          위 메서드 구현
controller/OrderController.h  runReleaseMenu(), listConfirmedOrders(), executeRelease() 추가
controller/OrderController.cpp 메뉴 3번 showComingSoon → runReleaseMenu(), 위 메서드 구현
SampleOrderSystem.vcxproj   test\RL01RL02Test.cpp 등록
```

> OrderController는 이미 `IRepository<Sample>&`와 `IOrderRepository&`를 보유하므로
> 생성자 변경 없이 출고 기능을 구현할 수 있다.

---

## 3. OrderView 추가 메서드

```cpp
// view/OrderView.h 추가
virtual void showReleaseMenu();
virtual void showReleased(const Order& o);
virtual void showNoConfirmedOrders();
```

**showReleaseMenu() 출력:**
```
========================================
  [출고 처리]
========================================
 1. CONFIRMED 주문 목록
 2. 출고 실행
 0. 돌아가기
========================================
선택: _
```

**showReleased():** "[출고 완료] 주문 N이(가) RELEASED 상태로 전환되었습니다."

---

## 4. OrderController 변경

### 메뉴 3번 변경

```
run() 메뉴 3번:
  기존: orderView_.showComingSoon()
  변경: runReleaseMenu()
```

### runReleaseMenu() 흐름

```
루프
├─ orderView_.showReleaseMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → listConfirmedOrders()   (RL-01)
├─ 2 → executeRelease()        (RL-02)
└─ else → mainView_.showInvalidInput()
```

### listConfirmedOrders() — RL-01

```cpp
auto all = orderRepo_.findAll();
std::vector<Order> confirmed;
for (const auto& o : all)
    if (o.status == OrderStatus::CONFIRMED)
        confirmed.push_back(o);
if (confirmed.empty()) { orderView_.showNoConfirmedOrders(); return; }
orderView_.showOrderList(confirmed, sampleRepo_.findAll());
```

### executeRelease() — RL-02

```
주문 ID 입력 (orderView_.inputOrderId())
→ orderRepo_.findById(id)
    ├─ 미존재 → showInvalidInput("존재하지 않는 주문입니다.")
    └─ status != CONFIRMED → showInvalidInput("CONFIRMED 상태의 주문만 출고할 수 있습니다.")

→ orderRepo_.updateStatus(id, RELEASED)
→ sample = sampleRepo_.findById(order.sampleId)
→ sample.stock -= order.quantity   (재고 차감)
→ sampleRepo_.update(sample)
→ orderView_.showReleased(order)
```

> `stock -= quantity` 적용 후 stock이 음수가 될 수 있는 경우는 현재 Phase에서
> 별도 검증하지 않는다 (승인 시 재고 충분 조건으로 이미 보장됨).

---

## 5. vcxproj 신규 등록

```xml
<ClCompile Include="test\RL01RL02Test.cpp" />
```

---

## 6. 테스트 계획 (gtest)

### RL01RL02Test — MockOrderView + MockOrderRepository + MockSampleRepository

| 테스트 ID | 설명 |
|-----------|------|
| `RL01Test.ShowsConfirmedOrders` | findAll → CONFIRMED 필터 → showOrderList 호출 |
| `RL01Test.ShowsNoOrdersWhenNoneConfirmed` | CONFIRMED 없음 → showNoConfirmedOrders 호출 |
| `RL02Test.ExecutesRelease` | CONFIRMED → updateStatus(RELEASED) + stock 차감 + showReleased |
| `RL02Test.RejectsReleaseForNonConfirmedOrder` | RESERVED 주문 출고 시도 → showInvalidInput |
| `RL02Test.RejectsReleaseForNonExistentOrder` | 미존재 주문 ID → showInvalidInput |

---

## 7. 구현 순서 (TDD 사이클)

```
1. RL01RL02Test 작성 → RED
2. OrderView 메서드 추가 구현
3. OrderController::runReleaseMenu() / listConfirmedOrders() / executeRelease() 구현 → GREEN
4. vcxproj 등록
5. 전체 빌드 + 테스트 확인
```

> **기존 테스트 변경**: `OrderControllerTest.ComingSoonOnValidMenu` 제거
> - Phase 7에서 메뉴 3번이 runReleaseMenu()로 구현되어 showComingSoon() 호출 없음
> - OrderController 전 메뉴(1~4)에 실제 기능이 구현되어 "준비 중" 테스트 불필요

---

## 8. 검토 포인트

- 출고 시 `stock -= quantity` 로 재고를 차감하는 위치:
  Controller(executeRelease)에서 직접 처리 — 동의하는가?
  (대안: OrderRepository.executeRelease()로 캡슐화)
- stock이 0 미만이 되는 엣지 케이스를 현재 Phase에서 검증할 필요가 있는가?
  (현재 설계: CONFIRMED는 stock >= quantity 조건으로 승인됐으므로 차감 후 음수 불가)
