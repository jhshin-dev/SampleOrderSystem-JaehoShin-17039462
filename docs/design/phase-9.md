# Phase 9 설계 — 생산 완료 처리 (PL-03)

## 1. 목표

PRODUCING 상태의 주문을 CONFIRMED로 전환하고, 실 생산량만큼 시료 재고를 증가시킨다.
전체 기능이 완성되어 End-to-End 시나리오가 동작한다.

---

## 2. 생성할 파일

신규 파일:
```
test/
    PL03Test.cpp
```

수정 파일:
```
view/ProductionView.h            inputOrderId(), showProductionCompleted() 추가
view/ProductionView.cpp          위 메서드 구현
controller/ProductionController.h  completeProduction() 추가
controller/ProductionController.cpp runProductionMenu()에 메뉴 3번 연결,
                                    completeProduction() 구현
SampleOrderSystem.vcxproj        test\PL03Test.cpp 등록
```

> ProductionController는 이미 `IRepository<Sample>&`와 `IOrderRepository&`를 보유하므로
> 생성자 변경 없이 생산 완료 기능을 구현할 수 있다.

---

## 3. ProductionView 추가 메서드

```cpp
// view/ProductionView.h 추가
virtual int  inputOrderId();
virtual void showProductionCompleted(const Order& o, int actualQty);
virtual void showInvalidInput(const std::string& msg);
```

**showProductionCompleted() 출력:**
```
[생산 완료] 주문 N이(가) CONFIRMED 상태로 전환되었습니다.
           실 생산량 M개가 재고에 반영되었습니다.
```

---

## 4. ProductionController — completeProduction() 구현

### runProductionMenu() 메뉴 3번 연결

```
루프
├─ productionView_.showProductionMenu()
├─ 0 → 종료
├─ 1 → showProductionStatus()
├─ 2 → showProductionQueue()
├─ 3 → completeProduction()   (PL-03, 신규)
└─ else → showInvalidInput
```

### completeProduction() 흐름

```
주문 ID 입력 (productionView_.inputOrderId())
→ orderRepo_.findById(id)
    ├─ 미존재 → productionView_.showInvalidInput("존재하지 않는 주문입니다.")
    └─ status != PRODUCING → productionView_.showInvalidInput("PRODUCING 상태의 주문만 완료 처리할 수 있습니다.")

→ sampleRepo_.findById(order.sampleId) 조회
→ 실 생산량 계산: actualQty = ⌈(order.quantity - sample.stock) / (sample.yield × 0.9)⌉
→ orderRepo_.updateStatus(id, CONFIRMED)
→ sample.stock += actualQty
→ sampleRepo_.update(sample)
→ productionView_.showProductionCompleted(order, actualQty)
```

> stock이 이미 quantity 이상인 경우(수동 재고 추가 등): actualQty = 0 또는 음수 방지를 위해
> `max(0, shortage)` 처리 — shortage ≤ 0 이면 actualQty = 0, stock은 변경 없음.

---

## 5. ProductionView::showProductionMenu() 업데이트

```
========================================
  [생산 라인]
========================================
 1. 생산 현황
 2. 생산 대기 큐
 3. 생산 완료 처리
 0. 돌아가기
========================================
```

---

## 6. vcxproj 신규 등록

```xml
<ClCompile Include="test\PL03Test.cpp" />
```

---

## 7. 테스트 계획 (gtest)

### PL03Test — MockProductionView + MockSampleRepository + MockOrderRepository

| 테스트 ID | 설명 |
|-----------|------|
| `PL03Test.CompletesProduction` | PRODUCING → updateStatus(CONFIRMED) + stock 증가 + showProductionCompleted |
| `PL03Test.RejectsNonProducingOrder` | CONFIRMED 주문 완료 시도 → showInvalidInput |
| `PL03Test.RejectsNonExistentOrder` | 미존재 주문 ID → showInvalidInput |

---

## 8. 구현 순서 (TDD 사이클)

```
1. PL03Test 작성 → RED
2. ProductionView 메서드 추가 구현
3. ProductionController::completeProduction() 구현 → GREEN
4. runProductionMenu() 메뉴 3번 연결
5. ProductionView::showProductionMenu() 텍스트 업데이트
6. vcxproj 등록
7. 전체 빌드 + 테스트 확인
```
