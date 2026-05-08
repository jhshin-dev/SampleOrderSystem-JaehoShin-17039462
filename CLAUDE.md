# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++20 기반 반도체 시료 생산주문관리 시스템. Visual Studio 2022 (v145 toolset), Windows x64 콘솔 애플리케이션.

역할 분리된 두 사용자: **주문 담당자(Order Manager)** 와 **생산 담당자(Production Manager)**.

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
    int         id    = 0;
    std::string name;
    std::string spec;   // 규격
    int         stock = 0;
};
```

**Order** — 생산 주문
```cpp
struct Order {
    int         id        = 0;
    int         sampleId  = 0;
    int         quantity  = 0;
    OrderStatus status    = OrderStatus::RESERVED;
    std::string createdAt;
    std::string updatedAt;
};
```

**OrderStatus** — 상태 전이 규칙 (state machine)
```
RESERVED ──→ CONFIRMED
         ──→ PRODUCING
         ──→ REJECTED
PRODUCING ──→ CONFIRMED
CONFIRMED ──→ RELEASED
```
`isValidTransition(from, to)` 으로 전이 유효성 검증. RELEASED 상태의 주문은 삭제 불가.

## Architecture

### MVC + Repository 패턴

```
controller/
    OrderController      주문 등록·목록·출고(processRelease)
    ProductionController  시료 등록·주문 승인/반려·생산 목록·생산 완료
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

## Development Workflow

모든 테스트는 **Google Test (gtest)** 로 작성한다. `TEST()` / `TEST_F()` 매크로를 사용하고, mock이 필요한 경우 gmock(`MOCK_METHOD`)을 함께 활용한다.

TDD 사이클로 개발: `/tdd` 스킬 참고.

모든 변경사항은 커밋으로 정리하며 `git push origin master`로 원격 반영:
- Remote: `https://github.com/jhshin-dev/SampleOrderSystem-JaehoShin-17039462.git`
- 커밋 컨벤션: 전역 CLAUDE.md의 Git 커밋 컨벤션 따름
