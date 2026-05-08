# MO — 모니터링 (Monitoring)

## 개요

담당자가 현재 시스템 상태를 한눈에 파악할 수 있도록 주문 현황과 재고 현황을 통합 제공한다.

---

## 기능 목록

| ID | 기능 | 진입점 |
|----|------|--------|
| MO-01 | 주문량 확인 | 메인 메뉴 → 모니터링 → 1 |
| MO-02 | 재고량 확인 | 메인 메뉴 → 모니터링 → 2 |

---

## MO-01 주문량 확인

유효한 주문(REJECTED 제외)을 상태별로 분류하여 출력한다.

### 출력 상태

| 상태 | 의미 |
|------|------|
| RESERVED | 접수 후 승인/거절 대기 중 |
| CONFIRMED | 승인 완료, 출고 대기 중 |
| PRODUCING | 재고 부족으로 생산 진행 중 |
| RELEASED | 출고 완료 |

> REJECTED는 유효 주문이 아니므로 제외한다.

### 처리 흐름

```
OrderRepository::findAll()
    │
    └─ REJECTED 제외 후 상태별 그룹핑
            │
            ├─ 각 상태별 건수 요약 출력
            └─ 상태별 주문 목록 출력 (주문 ID, 시료명, 고객명, 수량)
```

---

## MO-02 재고량 확인

시료별 현재 재고와 주문 대비 상태를 함께 출력한다.

### 재고 상태 기준

| 상태 | 조건 | 표기 |
|------|------|------|
| 여유 | `stock >= 유효 주문 총 수량` | ✅ 여유 |
| 부족 | `0 < stock < 유효 주문 총 수량` | ⚠️ 부족 |
| 고갈 | `stock == 0` | ❌ 고갈 |

> 유효 주문 = RESERVED + CONFIRMED + PRODUCING 상태 주문의 합산 수량

### 출력 항목

| 컬럼 | 설명 |
|------|------|
| 시료 ID | 식별자 |
| 시료명 | Sample.name |
| 현재 재고 | Sample.stock |
| 유효 주문량 | 해당 시료의 유효 주문 수량 합계 |
| 상태 | 여유 / 부족 / 고갈 |

### 처리 흐름

```
SampleRepository::findAll()
OrderRepository::findAll() → REJECTED / RELEASED 제외 후 sampleId별 수량 합산
    │
    └─ 시료별 (stock vs 유효 주문량) 비교 → 상태 판정 → 테이블 출력
```

---

## 관련 Repository

- `OrderRepository::findAll()` — MO-01, MO-02
- `SampleRepository::findAll()` — MO-02
