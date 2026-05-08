# Phase 3 설계 — 시료 조회 · 검색 + 요약 정보 갱신

## 1. 목표

등록된 시료 목록을 확인하고 이름으로 검색할 수 있다.
메인 메뉴 상단의 요약 정보(시료 수, 총 재고)가 실제 데이터를 반영한다.

---

## 2. 생성할 파일

신규 파일 없음. 기존 파일을 수정한다.

수정 파일:
```
view/SampleView.h           showSampleList(), inputSearchKeyword(), showNoResult() 추가
view/SampleView.cpp         위 메서드 구현
controller/ProductionController.cpp  runSampleMenu()에서 SM-02·03 연결
controller/AppController.h  요약 정보 계산을 위해 SampleRepository 접근
controller/AppController.cpp run() 에서 매 루프마다 시료 수·총 재고 동적 조회
test/AppControllerTest.cpp  요약 정보 관련 기존 테스트 조정 (하드코딩 0 → 동적)
test/SM02SM03Test.cpp       SM-02·03 흐름 신규 테스트 (신규 파일)
SampleOrderSystem.vcxproj  test\SM02SM03Test.cpp 등록
```

---

## 3. SampleView 추가 메서드

```cpp
// view/SampleView.h 추가
virtual void        showSampleList(const std::vector<Sample>& samples);
virtual std::string inputSearchKeyword();
virtual void        showNoResult();
```

**showSampleList() 출력 형식:**

```
  ┌────┬──────────────┬──────────┬──────┬──────┐
  │ ID │ 이름         │ 생산시간 │ 수율 │ 재고 │
  ├────┼──────────────┼──────────┼──────┼──────┤
  │  1 │ Silicon-A    │    30분  │ 0.95 │    0 │
  │  2 │ Germanium-B  │    20분  │ 0.85 │    0 │
  └────┴──────────────┴──────────┴──────┴──────┘
```

- 데이터 없으면 "등록된 시료가 없습니다." 출력

**inputSearchKeyword() 처리 규칙:**
- `std::cin.ignore` + `std::getline`으로 입력
- 빈 문자열 반환 허용 (Controller가 처리)

---

## 4. ProductionController — runSampleMenu() 수정

```
루프
├─ sampleView_.showSampleMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → registerSample()        (SM-01, 기존)
├─ 2 → listSamples()           (SM-02, 신규)
├─ 3 → searchSamples()         (SM-03, 신규)
└─ else → mainView_.showInvalidInput()
```

**listSamples():**
```cpp
void ProductionController::listSamples() {
    auto samples = sampleRepo_.findAll();
    sampleView_.showSampleList(samples);
}
```

**searchSamples():**
```cpp
void ProductionController::searchSamples() {
    std::string keyword = sampleView_.inputSearchKeyword();
    auto all = sampleRepo_.findAll();
    std::vector<Sample> result;
    for (const auto& s : all)
        if (s.name.find(keyword) != std::string::npos)
            result.push_back(s);
    if (result.empty())
        sampleView_.showNoResult();
    else
        sampleView_.showSampleList(result);
}
```

---

## 5. AppController — 요약 정보 동적 계산

현재 `showProductionManagerMenu(0, 0)` / `showOrderManagerMenu(0, 0)` 으로
하드코딩된 값을 SampleRepository에서 동적으로 계산해 전달한다.

```cpp
void AppController::run() {
    // ...
    while (true) {
        // 매 루프마다 최신 요약 정보 계산
        auto samples   = sampleRepo_.findAll();
        int  count     = static_cast<int>(samples.size());
        int  totalStock = 0;
        for (const auto& s : samples) totalStock += s.stock;

        mainView_.showRoleMenu(count, totalStock);  // 시그니처 변경
        // ...
    }
}
```

> **시그니처 변경**: `MainView::showRoleMenu()` → `showRoleMenu(int count, int totalStock)`

### 역할 메뉴 요약 정보 표시

역할 선택 화면(showRoleMenu)과 역할별 메뉴(showOrderManagerMenu, showProductionManagerMenu) 모두 동일한 count·totalStock을 받아 표시한다.

```
========================================
  반도체 시료 생산주문관리 시스템
  [요약] 시료: 2종  총 재고: 0개
========================================
```

---

## 6. MainView 시그니처 변경

```cpp
// view/MainView.h — 변경
virtual void showRoleMenu(int sampleCount, int totalStock);
```

기존 `showRoleMenu()` (인수 없음) → `showRoleMenu(int, int)` 로 변경.

- `showOrderManagerMenu(int, int)` / `showProductionManagerMenu(int, int)` 는 이미 인수 있음 → 변경 없음
- AppController에서 role 선택 전 한 번, 역할 메뉴 진입 시 한 번 계산해서 전달

---

## 7. vcxproj 신규 등록

```xml
<ClCompile Include="test\SM02SM03Test.cpp" />
```

---

## 8. 테스트 계획 (gtest)

### SM02SM03Test — MockSampleRepository + MockSampleView 사용

```cpp
class MockSampleView : public SampleView { ... };       // SM01Test와 동일 패턴
class MockSampleRepository : public IRepository<Sample> { ... };
```

| 테스트 ID | 설명 |
|-----------|------|
| `SM02Test.ShowsAllSamples` | findAll() 반환값 → showSampleList() 호출 |
| `SM02Test.ShowsEmptyMessage` | findAll() 빈 배열 → showSampleList([]) 호출 |
| `SM03Test.FindsByKeyword` | keyword로 필터링 후 showSampleList(결과) 호출 |
| `SM03Test.ShowsNoResultWhenNotFound` | 매칭 없음 → showNoResult() 호출 |
| `SM03Test.EmptyKeywordMatchesAll` | 빈 keyword → 전체 결과 반환 |

### AppControllerSummaryTest — 요약 정보 동적 계산

| 테스트 ID | 설명 |
|-----------|------|
| `AppControllerSummaryTest.ShowsCountAndStockFromRepo` | findAll() 결과 count·stock 합산 → showRoleMenu(N, M) 호출 |
| `AppControllerSummaryTest.ShowsZeroWhenRepoEmpty` | findAll() 빈 배열 → showRoleMenu(0, 0) 호출 |

### 기존 테스트 수정

- `AppControllerTest.*` — `showRoleMenu()` 호출이 `showRoleMenu(int, int)` 로 변경되므로
  MockMainView MOCK_METHOD 시그니처 및 EXPECT_CALL 수정
- `ProductionControllerTest.*` — 기존 동작 유지 (시료 관리 1번 진입 흐름은 그대로)

---

## 9. 구현 순서 (TDD 사이클)

```
1. SM02SM03Test 작성 → RED
2. SampleView에 showSampleList / inputSearchKeyword / showNoResult 추가 → 빌드
3. ProductionController::listSamples() / searchSamples() 구현 → GREEN
4. AppControllerSummaryTest 작성 → RED
5. MainView::showRoleMenu(int, int) 시그니처 변경
6. AppController::run() 요약 정보 동적 계산 구현 → GREEN
7. 기존 AppControllerTest EXPECT_CALL 수정
8. vcxproj SM02SM03Test.cpp 등록
9. 전체 빌드 + 테스트 실행 확인
```

---

## 10. 검토 포인트

- `showRoleMenu()` 시그니처를 `showRoleMenu(int, int)` 로 변경하면
  기존 `AppControllerTest` Mock 정의도 수정해야 한다 — 동의하는가?
- 요약 정보를 **역할 선택 화면**에만 표시할지, **역할별 메뉴 상단**에도 표시할지?
  (현재 설계: 역할 선택 화면 + 역할별 메뉴 모두 표시)
- `searchSamples()`에서 빈 keyword 입력 시 전체 결과를 반환하는 것이 적절한가?
  (대안: 빈 keyword → 오류 메시지)
