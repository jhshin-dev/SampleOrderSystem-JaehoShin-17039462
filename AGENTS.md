# AGENTS.md

이 파일은 AI 에이전트가 이 저장소에서 작업할 때 참고해야 할 문서 구조와 핵심 컨텍스트를 기록한다.

---

## 문서 구조

```
docs/
├── PRD.md                          전체 제품 요구사항 (기능·도메인·비기능)
├── PLAN.md                         Phase별 개발 목표 및 계획
├── design/
│   └── phase-1.md                  Phase 1 설계 (파일 구조·클래스 인터페이스·테스트 계획)
└── features/
    ├── MM-main-menu.md             메인 메뉴 화면 구성 및 네비게이션 흐름
    ├── SM-sample-management.md     시료 등록·조회·검색
    ├── OR-order.md                 주문 접수·승인·거절 및 재고 기반 자동 분기
    ├── MO-monitoring.md            주문량·재고량 현황 모니터링
    ├── RL-release.md               출고 처리 (CONFIRMED → RELEASED)
    └── PL-production-line.md       생산 현황·대기 큐·생산량 계산 공식
```

---

## 문서별 역할

| 문서 | 내용 요약 |
|------|-----------|
| `docs/PRD.md` | 시스템 전체 요구사항. 메인 메뉴 구조, 기능 ID 목록, 도메인 모델, 상태 전이, 비기능 요구사항, 범위 외 항목 포함 |
| `docs/PLAN.md` | Phase별 개발 목표를 세운 계획 파일 |
| `docs/design/phase-N.md` | 각 Phase 구현 전 작성하는 설계 문서. 파일 구조·클래스 인터페이스·TDD 테스트 계획 포함. 구현 전 반드시 검토 후 확정 |
| `docs/features/MM-main-menu.md` | 프로그램 진입점. 요약 정보(시료 수·총 재고) 표시, 메뉴 선택 처리, 루프 흐름 |
| `docs/features/SM-sample-management.md` | 시료 등록 입력값·검증, 조회 출력 항목, 이름 기반 검색 로직 |
| `docs/features/OR-order.md` | 주문 접수 입력값, 승인 시 재고 충분/부족 분기 흐름, 거절 제약, 생산 라인 자동 등록 계산값 |
| `docs/features/MO-monitoring.md` | 상태별 주문 현황(REJECTED 제외), 재고 상태 3단계 기준(여유·부족·고갈), 유효 주문량 계산 |
| `docs/features/RL-release.md` | CONFIRMED 주문 목록, 출고 실행 가드 조건, RELEASED 삭제 불가 제약 |
| `docs/features/PL-production-line.md` | 실 생산량 공식 `⌈부족분 / (yield × 0.9)⌉`, FIFO 큐, 생산 완료 시 stock 반영 |

---

## 작업 전 필독 순서

1. `CLAUDE.md` — 빌드·테스트 명령, 도메인 모델(struct), 아키텍처 패턴, 파일 추가 규칙
2. `docs/PRD.md` — 전체 기능 범위와 상태 전이 규칙 파악
3. `docs/PLAN.md` — 현재 Phase 목표와 고객 테스트 포인트 확인
4. `docs/design/phase-N.md` — 해당 Phase 설계 문서 (파일 구조·클래스·테스트 계획)
5. 해당 기능의 `docs/features/<XX>-*.md` — 구현 전 입력·출력·흐름·제약 확인

## Phase 개발 워크플로

```
설계 문서 작성 → [커밋]
→ /doc-verify → [변경 시 커밋]
→ 구현(TDD)
→ /compliance-verify N ┐ 병렬 실행 → [각각 설계 문서 변경 시 커밋]
   /test-verify N      ┘
→ [구현 코드 커밋]  ← 아래 3가지 조건 모두 충족 후
→ 사람 리뷰
```

**구현 코드 커밋 조건 (모두 충족 필요):**
1. 설계 문서 변경사항 커밋 완료
2. `/compliance-verify` PASS
3. `/test-verify` PASS

---

## 핵심 비즈니스 규칙 (빠른 참조)

- 등록된 시료만 주문 가능 (`sampleId` 존재 여부 검증 필수)
- 주문 승인 시 재고 자동 분기: `stock >= quantity` → CONFIRMED, `stock < quantity` → PRODUCING
- 실 생산량: `⌈(quantity - stock) / (yield × 0.9)⌉`
- REJECTED 주문은 모니터링에서 제외
- RELEASED 주문은 삭제 불가
- 모든 상태 전이는 `isValidTransition(from, to)` 통과 필수
