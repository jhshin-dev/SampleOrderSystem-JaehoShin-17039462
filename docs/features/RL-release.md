# RL — 출고 처리 (Release)

## 개요

재고가 충분해져 `CONFIRMED` 상태가 된 주문에 대해 출고를 처리한다. 출고된 주문은 `RELEASED`로 전이되며 이후 삭제 불가다.

---

## 기능 목록

| ID | 기능 | 진입점 |
|----|------|--------|
| RL-01 | CONFIRMED 주문 목록 | 메인 메뉴 → 출고 처리 → 1 |
| RL-02 | 출고 실행 | 메인 메뉴 → 출고 처리 → 2 |

---

## RL-01 CONFIRMED 주문 목록

출고 가능한 주문(`CONFIRMED` 상태)만 필터링하여 출력한다.

### 출력 항목

| 컬럼 | 설명 |
|------|------|
| 주문 ID | 주문 고유 식별자 |
| 시료명 | sampleId로 조회한 Sample.name |
| 고객명 | customerName |
| 수량 | quantity |
| 승인일시 | updatedAt (CONFIRMED 전이 시각) |

```
OrderRepository::findAll() → status == CONFIRMED 필터
    │
    ├─ 결과 있음 → 테이블 출력
    └─ 결과 없음 → "출고 가능한 주문이 없습니다" 메시지 출력
```

---

## RL-02 출고 실행

### 입력값

| 항목 | 타입 | 제약 |
|------|------|------|
| 주문 ID | int | 필수, CONFIRMED 상태여야 함 |

### 처리 흐름

```
주문 ID 입력
    │
    ├─ 주문 미존재 → "존재하지 않는 주문입니다" 오류 출력
    │
    ├─ status != CONFIRMED → "출고 가능한 상태가 아닙니다" 오류 출력
    │
    └─ 유효 → OrderRepository::updateStatus(RELEASED)
                  └─ updatedAt 갱신
                  └─ orders.json 저장
                  └─ "출고 처리가 완료되었습니다" 출력
```

### 제약

- `RELEASED` 상태로의 전이는 `CONFIRMED`에서만 가능 (`isValidTransition` 검증)
- `RELEASED` 주문은 `OrderRepository::remove()` 호출 시 거부됨

---

## 상태 전이

```
CONFIRMED ──→ RELEASED   (출고 처리 완료)
```

---

## 관련 Repository

- `OrderRepository::findAll()` — RL-01
- `OrderRepository::findById()` — RL-02 상태 검증
- `OrderRepository::updateStatus()` — RL-02 출고 실행
