# OR — 주문 (Order)

## 개요

고객이 시료를 요청하면 주문 담당자가 주문을 생성한다. 접수된 주문은 승인 또는 거절로 처리되며, 승인 시 재고 상황에 따라 상태가 자동으로 분기된다.

---

## 기능 목록

| ID | 기능 | 담당 역할 | 진입점 |
|----|------|-----------|--------|
| OR-01 | 주문 접수 (시료 예약) | 주문 담당자 | 주문 담당자 메뉴 → 1 |
| OR-02 | 접수 주문 목록 조회 | 주문 담당자 | 주문 담당자 메뉴 → 2 |
| OR-03 | 주문 승인 | 생산 담당자 | 생산 담당자 메뉴 → 2 → 1 |
| OR-04 | 주문 거절 | 생산 담당자 | 생산 담당자 메뉴 → 2 → 2 |

---

## OR-01 주문 접수 (시료 예약)

### 입력값

| 항목 | 타입 | 제약 |
|------|------|------|
| 시료 ID | int | 필수, SampleRepository에 존재하는 ID |
| 고객명 | string | 필수, 비어있으면 거부 |
| 주문 수량 | int | 필수, 1 이상 |

### 처리 흐름

```
입력값 검증
    │
    ├─ 시료 ID 미존재 → "존재하지 않는 시료입니다" 오류 출력
    │
    ├─ 수량 0 이하  → "수량은 1 이상이어야 합니다" 오류 출력
    │
    └─ 유효 → OrderRepository::create() 호출
                  └─ status = RESERVED
                  └─ createdAt / updatedAt = 현재 ISO 8601 타임스탬프
                  └─ orders.json 저장
                  └─ 접수 완료 메시지 출력
```

---

## OR-02 접수 주문 목록 조회

`RESERVED` 상태의 주문만 필터링하여 출력한다.

### 출력 항목

| 컬럼 | 설명 |
|------|------|
| 주문 ID | 주문 고유 식별자 |
| 시료명 | sampleId로 조회한 Sample.name |
| 고객명 | customerName |
| 수량 | quantity |
| 접수일시 | createdAt |

```
OrderRepository::findAll() → status == RESERVED 필터
    │
    ├─ 결과 있음 → 테이블 출력
    └─ 결과 없음 → "접수된 주문이 없습니다" 메시지 출력
```

---

## OR-03 주문 승인

`RESERVED` 주문을 선택해 승인하면 재고 상태에 따라 자동으로 분기된다.

### 승인 분기 로직

```
주문 ID 입력 → RESERVED 상태 확인
    │
    ├─ 재고 충분 (Sample.stock >= Order.quantity)
    │       └─ OrderRepository::updateStatus(CONFIRMED)
    │       └─ "주문이 승인되었습니다 (즉시 출고 대기)" 출력
    │
    └─ 재고 부족 (Sample.stock < Order.quantity)
            └─ 생산 라인에 자동 등록 (FIFO 큐)
            └─ OrderRepository::updateStatus(PRODUCING)
            └─ "재고 부족 — 생산 라인에 등록되었습니다" 출력
```

### 생산 라인 등록 시 계산값

| 항목 | 공식 |
|------|------|
| 부족분 | `quantity - stock` |
| 실 생산량 | `⌈부족분 / (yield × 0.9)⌉` |
| 총 생산시간 | `avgProductionTime × 실 생산량` (분) |

---

## OR-04 주문 거절

`RESERVED` 주문을 선택해 즉시 거절한다.

```
주문 ID 입력 → RESERVED 상태 확인
    │
    └─ OrderRepository::updateStatus(REJECTED)
           └─ updatedAt 갱신
           └─ "주문이 거절되었습니다" 출력
```

### 제약

- REJECTED 전이는 `RESERVED` 상태에서만 가능
- 거절된 주문은 모니터링(MO-01)에서 제외됨

---

## 상태 전이 요약

```
RESERVED ──→ CONFIRMED   OR-03 승인 + 재고 충분
         ──→ PRODUCING   OR-03 승인 + 재고 부족
         ──→ REJECTED    OR-04 거절
```

---

## 관련 엔티티

```cpp
struct Order {
    int         id           = 0;
    int         sampleId     = 0;
    std::string customerName;
    int         quantity     = 0;
    OrderStatus status       = OrderStatus::RESERVED;
    std::string createdAt;
    std::string updatedAt;
};
```

## 관련 Repository

- `OrderRepository::create()` — OR-01
- `OrderRepository::findAll()` — OR-02
- `OrderRepository::updateStatus()` — OR-03, OR-04
- `SampleRepository::findById()` — OR-01 시료 ID 검증, OR-03 재고 확인
