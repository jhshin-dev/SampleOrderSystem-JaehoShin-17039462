# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++20 기반 반도체 시료 생산주문관리 시스템. Visual Studio 2022 (v145 toolset), Windows x64 콘솔 애플리케이션.

역할 분리된 두 사용자: **주문 담당자(Order Manager)** 와 **생산 담당자(Production Manager)**.

| 역할 | 담당 기능 |
|------|-----------|
| 주문 담당자 | 주문 접수(OR-01·02), 출고 처리(RL-01·02), 모니터링(MO-01·02) |
| 생산 담당자 | 시료 관리(SM-01~03), 주문 승인·거절(OR-03·04), 생산 라인(PL-01~03), 모니터링(MO-01·02) |

## Build & Test Commands

MSBuild 경로: `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe`

```powershell
# 빌드 (Debug|x64 기본)
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" SampleOrderSystem.vcxproj /p:Configuration=Debug /p:Platform=x64

# 전체 재빌드
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" SampleOrderSystem.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Rebuild

# 전체 테스트 실행
.\x64\Debug\SampleOrderSystem.exe

# 특정 테스트만 실행
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=OrderTest.CreateOrder

# 테스트 스위트 전체 실행
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=OrderTest.*
```

## Domain Model

### 핵심 엔티티

**Sample** — 반도체 시료
```cpp
struct Sample {
    int         id                = 0;
    std::string name;
    int         avgProductionTime = 0;   // 평균 생산시간 (분)
    double      yield             = 0.0; // 수율 (0.0 ~ 1.0)
    int         stock             = 0;
};
```

> 수율(yield): 정상 시료 수 / 총 생산 시료 수. 예) 100개 중 90개 정상 → 0.9

**Order** — 생산 주문
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

**OrderStatus** — 상태 전이 규칙 (state machine)
```
RESERVED ──→ CONFIRMED   (승인 + 재고 충분: stock >= quantity)
         ──→ PRODUCING   (승인 + 재고 부족: stock < quantity → 생산 라인 자동 등록)
         ──→ REJECTED    (거절)
PRODUCING ──→ CONFIRMED  (생산 완료 시 자동 전이)
CONFIRMED ──→ RELEASED   (출고 처리)
```
`isValidTransition(from, to)` 으로 전이 유효성 검증. RELEASED 상태의 주문은 삭제 불가.

**생산량 계산** (재고 부족 시 생산 라인 등록 시점)
```
실 생산량  = ⌈부족분 / (수율 × 0.9)⌉
총 생산시간 = avgProductionTime × 실 생산량
```

## Architecture

### MVC + Repository 패턴

```
controller/
    OrderController       주문 접수·목록(OR-01·02), 출고 처리(RL-01·02), 모니터링(MO)
    ProductionController  시료 관리(SM), 주문 승인·거절(OR-03·04), 생산 라인(PL), 모니터링(MO)
model/
    Sample.h / Order.h / OrderStatus.h
    SampleRepository / OrderRepository   (JsonRepository<T> 상속)
view/
    OrderView / ProductionView
lib/
    json.hpp             nlohmann/json v3.11.3 (헤더 온리)
data/
    samples.json / orders.json           런타임 생성
```

Controller는 콘솔 I/O를 직접 하지 않고 View에 위임. Repository는 생성 즉시 JSON 파일을 로드하고 변경마다 저장.

### Repository 계층

**`IRepository<T>`** (인터페이스)
```cpp
virtual T                  create(T entity)        = 0;
virtual std::vector<T>     findAll()               = 0;
virtual std::optional<T>   findById(int id)        = 0;
virtual bool               update(const T& entity) = 0;
virtual bool               remove(int id)          = 0;
```

**`JsonRepository<T>`** (구현체) — nlohmann/json으로 파일 직렬화, auto-increment ID 관리.

**`OrderRepository`** — `JsonRepository<Order>` 상속, 상태 전이 검증 및 ISO 8601 타임스탬프 자동 설정.

```cpp
// IRepository<T> 외 OrderRepository 전용 메서드
bool updateStatus(int id, OrderStatus newStatus);
```

### 모니터링 / 더미 데이터 도구 (PoC 참고)

| 도구 | 핵심 구조 |
|------|-----------|
| DataMonitor | `DataLoader`(정적 파일 로드) + `DisplayRenderer`(정적 렌더) + `MonitorApp`(커맨드 디스패치) |
| DummyDataGenerator | `ScenarioData` (샘플·주문·생산라인 묶음) + 4가지 시나리오 함수 (A: CONFIRMED/RELEASED, B: PRODUCING, C: REJECTED, D: 혼합) |

`ProductionLine`: id, 할당된 orderId, `LineStatus` (IDLE / RUNNING) 포함.

### Test Framework
- **GMock 1.11.0** (NuGet): `packages/gmock.1.11.0/` — gtest + gmock 헤더 및 소스 포함
- `gmock.targets`가 `gtest-all.cc`와 `gmock-all.cc`를 자동으로 컴파일하므로 별도 링크 불필요
- `gtest_main.cc`는 포함되지 않음 — `main.cpp`에서 직접 `main()` 제공 필수

```cpp
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

### 파일 추가 규칙
새 `.cpp` 파일을 만들면 반드시 `SampleOrderSystem.vcxproj`에 `<ClCompile>` 항목으로 등록해야 빌드에 포함됨:

```xml
<ItemGroup>
  <ClCompile Include="model/Order.cpp" />
</ItemGroup>
```

### 의존성
- `packages/` 디렉터리는 `.gitignore`에 포함되어 있어 git에서 제외됨
- 로컬 빌드 시 `packages/gmock.1.11.0/` 디렉터리가 존재해야 함

## Documentation

기능 구현 전 아래 문서를 참고한다.

| 문서 | 설명 |
|------|------|
| `AGENTS.md` | 문서 구조 인덱스, 읽기 순서, 핵심 비즈니스 규칙 빠른 참조 |
| `docs/PRD.md` | 전체 기능 범위, 상태 전이, 도메인 모델, 비기능 요구사항 |
| `docs/features/MM-main-menu.md` | 메인 메뉴 화면 구성 및 네비게이션 흐름 |
| `docs/features/SM-sample-management.md` | 시료 등록·조회·검색 입력값·검증·흐름 |
| `docs/features/OR-order.md` | 주문 접수·승인(재고 분기)·거절 흐름 및 제약 |
| `docs/features/MO-monitoring.md` | 주문량·재고량 현황, 재고 상태 3단계 기준 |
| `docs/features/RL-release.md` | 출고 처리 가드 조건 및 RELEASED 제약 |
| `docs/features/PL-production-line.md` | 생산량 공식, FIFO 큐, 생산 완료 stock 반영 |
| `docs/PLAN.md` | Phase별 개발 목표 및 고객 테스트 포인트 |

### Phase 설계 문서

각 Phase 구현 전 `docs/design/phase-N.md`를 작성하고 검토 후 개발한다.

| 문서 | Phase | 상태 |
|------|-------|------|
| `docs/design/phase-1.md` | 메인 메뉴 (역할 선택·네비게이션) | 설계 완료 |

## Development Workflow

모든 테스트는 **Google Test (gtest)** 로 작성한다. `TEST()` / `TEST_F()` 매크로를 사용하고, mock이 필요한 경우 gmock(`MOCK_METHOD`)을 함께 활용한다.

TDD 사이클로 개발: `/tdd` 스킬 참고.

모든 변경사항은 커밋으로 정리하며 `git push origin master`로 원격 반영:
- Remote: `https://github.com/jhshin-dev/SampleOrderSystem-JaehoShin-17039462.git`
- 커밋 컨벤션: 전역 CLAUDE.md의 Git 커밋 컨벤션 따름
