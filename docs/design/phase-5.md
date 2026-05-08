# Phase 5 설계 — 주문 승인 · 거절 (OR-03·04)

## 1. 목표

생산 담당자가 RESERVED 주문을 승인 또는 거절한다.
승인 시 재고 상태에 따라 CONFIRMED(충분) 또는 PRODUCING(부족)으로 자동 분기된다.

---

## 2. 생성할 파일

신규 파일 없음. 기존 파일을 수정한다.

수정 파일:
```
view/OrderView.h            inputOrderId(), showApprovalMenu(),
                            showConfirmed(), showSentToProduction(), showRejected() 추가
view/OrderView.cpp          위 메서드 구현
controller/ProductionController.h   OrderView&, IOrderRepository& 주입 추가
                                    runOrderApprovalMenu(), approveOrder(), rejectOrder() 추가
controller/ProductionController.cpp 위 메서드 구현
controller/AppController.cpp        ProductionController 생성자 인수 변경 반영
test/AppControllerTest.cpp  ProductionController 생성자 변경에 따른 Mock 수정
test/OR03OR04Test.cpp       OR-03·04 흐름 테스트 (신규 파일)
SampleOrderSystem.vcxproj  test\OR03OR04Test.cpp 등록
```

---

## 3. OrderView 추가 메서드

```cpp
// view/OrderView.h 추가
virtual void showApprovalMenu();
virtual int  inputOrderId();
virtual void showConfirmed(const Order& o);
virtual void showSentToProduction(const Order& o);
virtual void showRejected(const Order& o);
```

**출력 메시지:**
- `showConfirmed()` → "[승인 완료] 주문 N이 CONFIRMED 상태로 전환되었습니다. (즉시 출고 대기)"
- `showSentToProduction()` → "[생산 등록] 재고 부족으로 주문 N이 생산 라인에 등록되었습니다. (PRODUCING)"
- `showRejected()` → "[거절 완료] 주문 N이 REJECTED 상태로 전환되었습니다."

**showApprovalMenu() 출력:**
```
========================================
  [주문 승인 · 거절]
========================================
 1. 접수 주문 목록
 2. 주문 승인
 3. 주문 거절
 0. 돌아가기
========================================
선택: _
```

---

## 4. ProductionController 변경

### 생성자

```cpp
ProductionController(MainView& mainView,
                     SampleView& sampleView,
                     OrderView& orderView,
                     IRepository<Sample>& sampleRepo,
                     IOrderRepository& orderRepo);
```

### runOrderApprovalMenu() 흐름

```
루프
├─ orderView_.showApprovalMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → listReservedOrders()     접수 주문 목록 (OR-02 재사용)
├─ 2 → approveOrder()           (OR-03)
├─ 3 → rejectOrder()            (OR-04)
└─ else → mainView_.showInvalidInput()
```

> ProductionController::run()의 메뉴 2번이 기존 `showComingSoon()` 대신 `runOrderApprovalMenu()` 를 호출한다.

### approveOrder() — OR-03 핵심 로직

```
주문 ID 입력 (orderView_.inputOrderId())
→ orderRepo_.findById(id) 조회
    ├─ 미존재 → orderView_.showInvalidInput("존재하지 않는 주문입니다.")
    └─ status != RESERVED → orderView_.showInvalidInput("RESERVED 상태의 주문만 승인할 수 있습니다.")

→ sampleRepo_.findById(order.sampleId) 조회
→ 재고 분기
    ├─ stock >= quantity  → orderRepo_.updateStatus(id, CONFIRMED)
    │                       orderView_.showConfirmed(order)
    └─ stock < quantity   → orderRepo_.updateStatus(id, PRODUCING)
                            orderView_.showSentToProduction(order)
```

### rejectOrder() — OR-04

```
주문 ID 입력
→ findById → RESERVED 상태 확인
    ├─ 미존재 → showInvalidInput
    └─ status != RESERVED → showInvalidInput("RESERVED 상태의 주문만 거절할 수 있습니다.")
→ orderRepo_.updateStatus(id, REJECTED)
→ orderView_.showRejected(order)
```

### listReservedOrders() — OR-02 재사용

```
orderRepo_.findAll() → RESERVED 필터
→ orderView_.showOrderList(reserved, sampleRepo_.findAll())
→ 빈 경우 orderView_.showNoOrders()
```

---

## 5. AppController 변경

```cpp
AppController(MainView& mainView,
              OrderView& orderView,
              SampleView& sampleView,
              IRepository<Sample>& sampleRepo,
              IOrderRepository& orderRepo);
```

AppController가 `orderView_`와 `orderRepo_`를 ProductionController에도 전달한다.

```cpp
prodCtrl_(mainView, sampleView, orderView, sampleRepo, orderRepo)
```

---

## 6. vcxproj 신규 등록

```xml
<ClCompile Include="test\OR03OR04Test.cpp" />
```

---

## 7. 테스트 계획 (gtest)

### OR03OR04Test — MockOrderView + MockOrderRepository + MockSampleRepository

| 테스트 ID | 설명 |
|-----------|------|
| `OR03Test.ApprovesOrderConfirmedWhenStockSufficient` | stock >= quantity → CONFIRMED, showConfirmed 호출 |
| `OR03Test.ApprovesOrderProducingWhenStockInsufficient` | stock < quantity → PRODUCING, showSentToProduction 호출 |
| `OR03Test.RejectsApprovalForNonExistentOrder` | 미존재 주문 ID → showInvalidInput, updateStatus 미호출 |
| `OR03Test.RejectsApprovalForNonReservedOrder` | CONFIRMED 주문 승인 시도 → showInvalidInput |
| `OR04Test.RejectsOrder` | RESERVED → REJECTED, showRejected 호출 |
| `OR04Test.RejectsRejectionForNonReservedOrder` | CONFIRMED 주문 거절 시도 → showInvalidInput |

### 기존 테스트 수정

- `ProductionControllerTest.*` — 생성자에 `MockOrderView`, `MockOrderRepository` 추가 주입
- `AppControllerTest.*` — ProductionController 생성자 변경 반영

---

## 8. 구현 순서 (TDD 사이클)

```
1. OR03OR04Test 작성 → RED
2. OrderView 메서드 추가 구현
3. ProductionController::approveOrder() / rejectOrder() 구현 → GREEN
4. ProductionController::runOrderApprovalMenu() 연결
5. AppController 생성자 업데이트
6. 기존 ProductionControllerTest·AppControllerTest 수정
7. vcxproj 등록
8. 전체 빌드 + 테스트 실행 확인
```

---

## 9. 검토 포인트

- `listReservedOrders()`를 ProductionController에 중복 구현하는 대신
  OrderController의 메서드를 재사용하는 방안 — 단, 컨트롤러 간 직접 호출은
  MVC 원칙에 위배되므로 중복 구현이 적절하다
- 재고 부족 승인 시 `Sample.stock`을 즉시 차감하지 않음
  (실제 차감은 Phase 9 생산 완료 후 stock += 실 생산량 시점)
