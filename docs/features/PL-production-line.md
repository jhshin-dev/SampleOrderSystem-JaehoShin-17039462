# PL — 생산 라인 (Production Line)

## 개요

주문 승인 시 재고가 부족하면 생산 라인에 자동 등록된다. 수율과 오차를 반영한 실 생산량을 산출하고, 생산 완료 시 주문 상태를 `CONFIRMED`로 전이한다. 대기 주문은 FIFO 방식으로 처리된다.

---

## 기능 목록

| ID | 기능 | 진입점 |
|----|------|--------|
| PL-01 | 생산 현황 조회 | 메인 메뉴 → 생산 라인 → 1 |
| PL-02 | 생산 대기 큐 조회 | 메인 메뉴 → 생산 라인 → 2 |

---

## 생산량 계산 공식

재고 부족으로 생산 라인에 등록될 때 아래 공식을 적용한다.

| 항목 | 공식 | 설명 |
|------|------|------|
| 부족분 | `quantity - stock` | 주문 수량에서 현재 재고를 뺀 값 |
| 실 생산량 | `⌈부족분 / (yield × 0.9)⌉` | 수율에 10% 오차 마진을 적용해 올림 |
| 총 생산시간 | `avgProductionTime × 실 생산량` | 단위: 분 |

**예시**: 주문 100개, 재고 40개, 수율 0.9, 평균 생산시간 5분
```
부족분      = 100 - 40 = 60
실 생산량   = ⌈60 / (0.9 × 0.9)⌉ = ⌈74.07⌉ = 75개
총 생산시간 = 5 × 75 = 375분
```

---

## PL-01 생산 현황 조회

현재 `PRODUCING` 상태인 주문과 연결된 시료 정보를 출력한다.

### 출력 항목

| 컬럼 | 설명 |
|------|------|
| 주문 ID | 주문 고유 식별자 |
| 시료명 | Sample.name |
| 고객명 | customerName |
| 주문 수량 | quantity |
| 부족분 | quantity - stock |
| 실 생산량 | `⌈부족분 / (yield × 0.9)⌉` |
| 총 생산시간 | `avgProductionTime × 실 생산량` (분) |

### 처리 흐름

```
OrderRepository::findAll() → status == PRODUCING 필터
    │
    ├─ 각 주문에 대해 SampleRepository::findById(sampleId) 로 시료 조회
    ├─ 실 생산량 / 총 생산시간 계산
    │
    ├─ 결과 있음 → 테이블 출력
    └─ 결과 없음 → "현재 생산 중인 주문이 없습니다" 메시지 출력
```

---

## PL-02 생산 대기 큐 조회

생산 등록 순서(FIFO)대로 대기 중인 주문 목록을 출력한다.

> 대기 큐 = `PRODUCING` 상태 주문을 `createdAt` 오름차순으로 정렬한 목록

### 출력 항목

| 컬럼 | 설명 |
|------|------|
| 순번 | FIFO 순서 (1부터) |
| 주문 ID | 주문 고유 식별자 |
| 시료명 | Sample.name |
| 고객명 | customerName |
| 수량 | quantity |
| 등록일시 | createdAt |

### 처리 흐름

```
OrderRepository::findAll() → status == PRODUCING 필터 → createdAt 오름차순 정렬
    │
    ├─ 결과 있음 → 순번과 함께 테이블 출력
    └─ 결과 없음 → "생산 대기 중인 주문이 없습니다" 메시지 출력
```

---

## 생산 완료 처리

생산이 완료되면 주문 상태를 `PRODUCING → CONFIRMED`로 전이한다.

```
OrderRepository::updateStatus(CONFIRMED)
    └─ updatedAt 갱신
    └─ Sample.stock += 실 생산량  (재고 반영)
    └─ orders.json / samples.json 저장
```

---

## 상태 전이

```
PRODUCING ──→ CONFIRMED   (생산 완료)
```

---

## 관련 Repository

- `OrderRepository::findAll()` — PL-01, PL-02
- `OrderRepository::updateStatus()` — 생산 완료 처리
- `SampleRepository::findById()` — 시료 정보 및 수율 조회
- `SampleRepository::update()` — 생산 완료 후 stock 반영
