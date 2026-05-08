# Phase 1 설계 — 메인 메뉴 (역할 선택 + 네비게이션)

## 1. 목표

프로그램을 실행하면 역할 선택 화면이 출력되고, 역할별 메뉴로 진입할 수 있다.
기능은 아직 없으므로 메뉴 항목 선택 시 "준비 중" 메시지를 출력하고 복귀한다.

---

## 2. 생성할 파일

```
main.cpp                          ← 기존 파일 교체 (gtest 더미 제거)
controller/
    AppController.h
    AppController.cpp
    OrderController.h
    OrderController.cpp
    ProductionController.h
    ProductionController.cpp
view/
    MainView.h
    MainView.cpp
test/
    AppControllerTest.cpp         ← gtest 테스트
```

> 새 `.cpp` 파일은 모두 `SampleOrderSystem.vcxproj`에 `<ClCompile>`로 등록 필요.

---

## 3. main.cpp — 앱 모드 / 테스트 모드 분기

현재 `main.cpp`는 gtest runner만 실행한다.  
Phase 1부터는 **인수 없이 실행하면 앱**, `--gtest_*` 인수가 있으면 테스트로 동작한다.

```cpp
#include <gtest/gtest.h>
#include "controller/AppController.h"
#include <string>

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).rfind("--gtest", 0) == 0) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    AppController app;
    app.run();
    return 0;
}
```

**실행 방법**
```powershell
.\x64\Debug\SampleOrderSystem.exe                        # 앱 실행
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=*       # 전체 테스트
.\x64\Debug\SampleOrderSystem.exe --gtest_filter=AppControllerTest.*  # 특정 테스트
```

---

## 4. View 설계 — MainView

Controller는 콘솔 I/O를 직접 하지 않는다. 모든 출력·입력은 `MainView`를 통한다.

### MainView.h

```cpp
#pragma once
#include <string>

class MainView {
public:
    void showRoleMenu();
    void showOrderManagerMenu(int sampleCount, int totalStock);
    void showProductionManagerMenu(int sampleCount, int totalStock);
    void showComingSoon();
    void showInvalidInput();
    int  getMenuInput();  // 정수 입력. 비정수 입력 시 -1 반환
};
```

### 출력 형식

```
역할 선택:
========================================
  반도체 시료 생산주문관리 시스템
========================================
 1. 주문 담당자
 2. 생산 담당자
 0. 종료
========================================
선택: _

주문 담당자 메뉴:
========================================
  [주문 담당자]  시료: 0종  총 재고: 0개
========================================
 1. 주문 접수
 2. 접수 주문 목록
 3. 출고 처리
 4. 모니터링
 0. 역할 선택으로 돌아가기
========================================
선택: _

생산 담당자 메뉴:
========================================
  [생산 담당자]  시료: 0종  총 재고: 0개
========================================
 1. 시료 관리
 2. 주문 승인 · 거절
 3. 생산 라인
 4. 모니터링
 0. 역할 선택으로 돌아가기
========================================
선택: _
```

### getMenuInput() 처리 규칙

- `std::cin`으로 입력받은 뒤 정수 변환 시도
- 변환 실패(문자 입력 등) → `cin` 상태 초기화 후 `-1` 반환
- Controller가 `-1` 또는 범위 외 숫자를 받으면 `showInvalidInput()` 호출

---

## 5. Controller 설계

### AppController

역할 선택 루프를 담당한다.

```cpp
// controller/AppController.h
#pragma once
#include "OrderController.h"
#include "ProductionController.h"
#include "../view/MainView.h"

class AppController {
public:
    explicit AppController(MainView& view);
    void run();
private:
    MainView&            view_;
    OrderController      orderCtrl_;
    ProductionController prodCtrl_;
};
```

```
AppController::run() 흐름:
    루프
    ├─ view_.showRoleMenu()
    ├─ input = view_.getMenuInput()
    ├─ 0 → 루프 종료
    ├─ 1 → orderCtrl_.run()
    ├─ 2 → prodCtrl_.run()
    └─ else → view_.showInvalidInput()
```

### OrderController (Phase 1 — stub)

```cpp
// controller/OrderController.h
#pragma once
#include "../view/MainView.h"

class OrderController {
public:
    explicit OrderController(MainView& view);
    void run();
private:
    MainView& view_;
};
```

```
OrderController::run() 흐름:
    루프
    ├─ view_.showOrderManagerMenu(0, 0)   // Phase 1: 하드코딩
    ├─ input = view_.getMenuInput()
    ├─ 0 → 루프 종료 (역할 선택으로 복귀)
    ├─ 1~4 → view_.showComingSoon()
    └─ else → view_.showInvalidInput()
```

### ProductionController (Phase 1 — stub)

`OrderController`와 동일한 구조. `showProductionManagerMenu(0, 0)` 호출.

---

## 6. Console 인코딩 설정

한글 출력을 위해 `AppController::run()` 진입 시 한 번 설정한다.

```cpp
#include <windows.h>

// AppController::run() 최상단
SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);
```

---

## 7. vcxproj 등록 목록

```xml
<ItemGroup>
  <ClCompile Include="controller\AppController.cpp" />
  <ClCompile Include="controller\OrderController.cpp" />
  <ClCompile Include="controller\ProductionController.cpp" />
  <ClCompile Include="view\MainView.cpp" />
  <ClCompile Include="test\AppControllerTest.cpp" />
</ItemGroup>
```

---

## 8. 테스트 계획 (gtest)

Phase 1에서 Controller의 I/O 로직은 `MainView`에 위임되므로, Controller 단위 테스트는 **View를 Mock으로 교체**하여 진행한다.

### MockMainView

```cpp
#include <gmock/gmock.h>
#include "../view/MainView.h"

class MockMainView : public MainView {
public:
    MOCK_METHOD(void, showRoleMenu,              (), (override));
    MOCK_METHOD(void, showOrderManagerMenu,      (int, int), (override));
    MOCK_METHOD(void, showProductionManagerMenu, (int, int), (override));
    MOCK_METHOD(void, showComingSoon,            (), (override));
    MOCK_METHOD(void, showInvalidInput,          (), (override));
    MOCK_METHOD(int,  getMenuInput,              (), (override));
};
```

> Mock을 사용하려면 `MainView`의 메서드가 `virtual`이어야 한다.

### 테스트 케이스

| 테스트 ID | 설명 | 검증 내용 |
|-----------|------|-----------|
| `AppControllerTest.ExitOnZero` | 역할 선택에서 0 입력 | `run()` 종료, showRoleMenu 1회 호출 |
| `AppControllerTest.InvalidInputShowsError` | 역할 선택에서 범위 외 입력 | `showInvalidInput()` 호출 |
| `AppControllerTest.SelectOrderManager` | 1 입력 시 주문 담당자 진입 | `showOrderManagerMenu()` 호출 |
| `AppControllerTest.SelectProductionManager` | 2 입력 시 생산 담당자 진입 | `showProductionManagerMenu()` 호출 |
| `OrderControllerTest.ExitOnZero` | 주문 담당자 메뉴에서 0 입력 | 루프 종료 |
| `OrderControllerTest.ComingSoonOnValidMenu` | 1~4 입력 시 | `showComingSoon()` 호출 |
| `OrderControllerTest.InvalidInputShowsError` | 범위 외 입력 | `showInvalidInput()` 호출 |

---

## 9. 구현 순서 (TDD 사이클)

```
1. MainView 구현 (I/O 없는 로직 없음 — 먼저 구현)
2. AppControllerTest.ExitOnZero 작성 → RED
3. AppController::run() 최소 구현 → GREEN
4. AppControllerTest.InvalidInputShowsError → RED → GREEN
5. AppControllerTest.SelectOrderManager → RED → GREEN
6. AppControllerTest.SelectProductionManager → RED → GREEN
7. OrderController 테스트 및 구현 (동일 사이클)
8. ProductionController 테스트 및 구현
9. main.cpp 듀얼 모드 전환
10. 전체 빌드 확인 후 수동 테스트
```

---

## 10. 검토 포인트

- `MainView`의 메서드를 `virtual`로 선언하는 것이 맞는가?
  (Mock 사용을 위해 필요하지만, 가상 함수 오버헤드가 있음)
- `AppController`가 `OrderController`와 `ProductionController`를 멤버로 소유하는 구조가 맞는가?
  (Phase 2 이후 Repository 주입 시 생성자 인터페이스 변경 예상)
- Phase 1에서 요약 정보(시료 수·총 재고)를 하드코딩 0으로 표시하는 것이 적절한가?
  (Phase 3에서 동적으로 교체 예정)
