# 반도체 시료 생산주문관리 시스템

반도체 시료의 생산 주문을 디지털로 관리하는 C++20 콘솔 애플리케이션입니다.  
주문 담당자와 생산 담당자 두 역할로 분리된 워크플로를 제공합니다.

---

## 빌드 환경

| 항목 | 내용 |
|------|------|
| 언어 | C++20 |
| IDE / 빌드 도구 | Visual Studio 2022 (v145 toolset) / MSBuild |
| 플랫폼 | Windows x64 |
| 테스트 프레임워크 | Google Test + GMock 1.11.0 (NuGet) |

---

## 빌드 및 실행

```powershell
# 빌드 (Debug|x64)
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
  SampleOrderSystem.vcxproj /p:Configuration=Debug /p:Platform=x64

# 앱 실행
.\x64\Debug\SampleOrderSystem.exe

# 전체 테스트 실행
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=*

# 특정 테스트 스위트 실행
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=SampleRepositoryTest.*
```

---

## 주요 기능

### 주문 담당자

| 기능 | 설명 |
|------|------|
| 주문 접수 | 시료 ID·고객명·수량 입력, 초기 상태 RESERVED |
| 접수 주문 목록 | RESERVED 상태 주문만 표시 |
| 출고 처리 | CONFIRMED 주문을 RELEASED로 전환, 재고 차감 |
| 모니터링 | 상태별 주문 현황 / 시료별 재고 상태 |

### 생산 담당자

| 기능 | 설명 |
|------|------|
| 시료 관리 | 시료 등록·목록 조회·이름 검색 |
| 주문 승인·거절 | 재고 충분 → CONFIRMED, 부족 → PRODUCING 자동 분기 |
| 생산 라인 | 생산 현황·대기 큐(FIFO) 확인 |
| 생산 완료 처리 | PRODUCING → CONFIRMED 전이, 실 생산량만큼 재고 증가 |
| 모니터링 | 상태별 주문 현황 / 시료별 재고 상태 |

---

## 주문 상태 전이

```
RESERVED ──→ CONFIRMED   (승인 + 재고 충분: stock >= quantity)
         ──→ PRODUCING   (승인 + 재고 부족 → 생산 라인 자동 등록)
         ──→ REJECTED    (거절)
PRODUCING ──→ CONFIRMED  (생산 완료 처리)
CONFIRMED ──→ RELEASED   (출고 처리)
```

---

## 생산량 계산 공식

```
실 생산량  = ⌈(quantity - stock) / (yield × 0.9)⌉
총 생산시간 = avgProductionTime × 실 생산량  (분)
```

---

## 프로젝트 구조

```
SampleOrderSystem/
├── src/                 소스 코드 루트
│   ├── main.cpp         진입점 (앱 모드 / 테스트 모드 분기)
│   ├── controller/      MVC Controller 계층
│   │   ├── AppController    역할 선택 및 전체 흐름 제어
│   │   ├── OrderController  주문 담당자 기능 (OR, RL, MO)
│   │   └── ProductionController  생산 담당자 기능 (SM, OR-03·04, PL, MO)
│   ├── model/           도메인 모델 및 Repository
│   │   ├── Sample.h / Order.h / OrderStatus.h / ProductionEntry.h
│   │   ├── IRepository.h / JsonRepository.h / IOrderRepository.h
│   │   ├── SampleRepository / OrderRepository
│   │   └── lib/json.hpp     nlohmann/json v3.11.3
│   ├── view/            콘솔 I/O View 계층
│   │   ├── MainView         역할 선택 메뉴
│   │   ├── SampleView       시료 관련 I/O
│   │   ├── OrderView        주문 관련 I/O
│   │   ├── MonitorView      모니터링 출력
│   │   └── ProductionView   생산 라인 I/O
│   └── test/            gtest 단위 테스트
├── data/                런타임 생성 JSON 파일 (gitignore)
│   ├── samples.json
│   └── orders.json
└── docs/
    ├── PRD.md           제품 요구사항
    ├── PLAN.md          Phase별 개발 계획
    ├── features/        기능별 상세 명세
    └── design/          Phase별 설계 문서
```

---

## 아키텍처

**MVC + Repository 패턴**

- **Controller** — 비즈니스 로직 처리, 콘솔 I/O 직접 호출 금지 (View에 위임)
- **View** — 모든 콘솔 입출력 담당, `virtual` 메서드로 Mock 테스트 지원
- **Repository** — `JsonRepository<T>` 기반 JSON 파일 영속성, 생성 즉시 로드·변경 즉시 저장

---

## 데이터 영속성

- `data/samples.json` — 시료 데이터 (자동 생성)
- `data/orders.json` — 주문 데이터 (자동 생성)
- 프로그램 재시작 후에도 데이터 유지됨

---

## 테스트 현황

총 **62개** gtest 케이스, 전체 PASS

| 테스트 스위트 | 케이스 수 |
|--------------|-----------|
| SampleRepositoryTest | 7 |
| OrderRepositoryTest | 7 |
| SM01Test | 5 |
| SM02Test / SM03Test | 5 |
| OR01Test / OR02Test | 6 |
| OR03Test / OR04Test | 6 |
| RL01Test / RL02Test | 5 |
| MO01Test / MO02Test | 4 |
| PL01Test / PL02Test | 4 |
| PL03Test | 3 |
| AppControllerTest / AppControllerSummaryTest | 6 |
| OrderControllerTest | 2 |
| ProductionControllerTest | 2 |
| **합계** | **62** |
