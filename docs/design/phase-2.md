# Phase 2 설계 — 시료 등록 (SM-01)

## 1. 목표

시료(Sample)를 시스템에 등록할 수 있다. 등록 데이터는 JSON 파일에 영속 저장되어 프로그램 재시작 후에도 유지된다.

---

## 2. 생성할 파일

```
lib/
    json.hpp                    nlohmann/json v3.11.3 (헤더 온리, 외부에서 복사)
model/
    Sample.h                    Sample 구조체 + to_json / from_json
    IRepository.h               IRepository<T> 순수 가상 인터페이스
    JsonRepository.h            JsonRepository<T> 템플릿 구현 (헤더 온리)
    SampleRepository.h
    SampleRepository.cpp
view/
    SampleView.h
    SampleView.cpp
test/
    SampleRepositoryTest.cpp    JsonRepository + SampleRepository 단위 테스트
    SM01Test.cpp                SM-01 등록 흐름 통합 테스트
```

수정 파일:
```
controller/ProductionController.h   IRepository<Sample>& 주입 추가
controller/ProductionController.cpp 시료 관리 서브 메뉴 → SM-01 연결
controller/AppController.h          IRepository<Sample>& 주입 추가
controller/AppController.cpp        SampleRepository 생성 후 ProductionController에 전달
main.cpp                            SampleRepository 인스턴스 생성 후 AppController에 주입
test/AppControllerTest.cpp          MockSampleRepository 추가, 기존 테스트 생성자 수정
SampleOrderSystem.vcxproj           새 .cpp 파일 등록
```

> 새 `.cpp` 파일은 모두 `SampleOrderSystem.vcxproj`에 `<ClCompile>`로 등록 필요.

---

## 3. 의존성 — json.hpp

`lib/json.hpp`는 nlohmann/json v3.11.3 단일 헤더 파일이다.

획득 방법:
```powershell
# DataPersistence PoC 저장소에서 복사 (동일 버전 사용 중)
# 또는 공식 릴리스에서 다운로드
# https://github.com/nlohmann/json/releases/tag/v3.11.3
# single_include/nlohmann/json.hpp → lib/json.hpp 로 저장
```

> vcxproj에 등록 불필요 (헤더 온리).

---

## 4. 도메인 모델 — Sample.h

```cpp
#pragma once
#include <string>
#include "../lib/json.hpp"

struct Sample {
    int         id                = 0;
    std::string name;
    int         avgProductionTime = 0;   // 분, 양수
    double      yield             = 0.0; // 0.0 초과 1.0 이하
    int         stock             = 0;   // 항상 0으로 초기화
};

inline void to_json(nlohmann::json& j, const Sample& s) {
    j = { {"id", s.id}, {"name", s.name},
          {"avgProductionTime", s.avgProductionTime},
          {"yield", s.yield}, {"stock", s.stock} };
}

inline void from_json(const nlohmann::json& j, Sample& s) {
    j.at("id").get_to(s.id);
    j.at("name").get_to(s.name);
    j.at("avgProductionTime").get_to(s.avgProductionTime);
    j.at("yield").get_to(s.yield);
    j.at("stock").get_to(s.stock);
}
```

---

## 5. Repository 계층

### IRepository<T> — 순수 가상 인터페이스

```cpp
// model/IRepository.h
#pragma once
#include <vector>
#include <optional>

template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual T                create(T entity)        = 0;
    virtual std::vector<T>   findAll()               = 0;
    virtual std::optional<T> findById(int id)        = 0;
    virtual bool             update(const T& entity) = 0;
    virtual bool             remove(int id)          = 0;
};
```

### JsonRepository<T> — 헤더 온리 템플릿

```cpp
// model/JsonRepository.h
#pragma once
#include "IRepository.h"
#include "../lib/json.hpp"
#include <fstream>
#include <string>
#include <filesystem>

template<typename T>
class JsonRepository : public IRepository<T> {
public:
    explicit JsonRepository(const std::string& filePath);
    T                create(T entity)        override;
    std::vector<T>   findAll()               override;
    std::optional<T> findById(int id)        override;
    bool             update(const T& entity) override;
    bool             remove(int id)          override;
protected:
    std::vector<T> items_;
private:
    std::string filePath_;
    int         nextId_ = 1;
    void load();
    void save() const;
};
```

**핵심 동작:**
- 생성자에서 `load()` 호출 → 파일 없으면 빈 상태로 시작
- `create()` → `nextId_` 부여, `items_`에 추가, `save()`
- `save()` → `nlohmann::json` 배열로 직렬화 후 파일 쓰기
- `load()` → 파일 파싱 후 `items_` 채우기, `nextId_` 계산

### SampleRepository

```cpp
// model/SampleRepository.h
#pragma once
#include "JsonRepository.h"
#include "Sample.h"

class SampleRepository : public JsonRepository<Sample> {
public:
    explicit SampleRepository(const std::string& filePath
                              = "data/samples.json");
};
```

`SampleRepository.cpp`에서 생성자만 정의. `data/` 디렉터리는 `save()` 시 자동 생성.

---

## 6. View — SampleView

```cpp
// view/SampleView.h
#pragma once
#include <string>
#include "../model/Sample.h"

class SampleView {
public:
    virtual ~SampleView() = default;
    virtual void        showSampleMenu();
    virtual std::string inputName();
    virtual int         inputAvgProductionTime();
    virtual double      inputYield();
    virtual void        showRegistered(const Sample& s);
    virtual void        showInvalidInput(const std::string& msg);
    virtual void        showComingSoon();
};
```

**입력 처리 규칙:**
- `inputName()` → 빈 문자열이면 빈 string 반환 (Controller가 검증)
- `inputAvgProductionTime()` → 정수 변환 실패 시 `-1` 반환
- `inputYield()` → double 변환 실패 시 `-1.0` 반환

---

## 7. Controller 변경

### ProductionController

```cpp
// controller/ProductionController.h
#pragma once
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/Sample.h"

class ProductionController {
public:
    ProductionController(MainView& mainView,
                         SampleView& sampleView,
                         IRepository<Sample>& sampleRepo);
    void run();
private:
    MainView&             mainView_;
    SampleView&           sampleView_;
    IRepository<Sample>&  sampleRepo_;
    void runSampleMenu();   // 시료 관리 서브 메뉴
    void registerSample();  // SM-01
};
```

**SM-01 검증 규칙 (registerSample 내부):**

| 항목 | 조건 | 오류 메시지 |
|------|------|------------|
| 이름 | 비어있으면 거부 | "이름을 입력해주세요." |
| 평균 생산시간 | 1 이상 | "평균 생산시간은 1분 이상이어야 합니다." |
| 수율 | 0.0 초과 1.0 이하 | "수율은 0.0 초과 1.0 이하로 입력해주세요." |

**runSampleMenu() 흐름:**
```
루프
├─ sampleView_.showSampleMenu()
├─ input = mainView_.getMenuInput()
├─ 0 → 루프 종료
├─ 1 → registerSample()   (SM-01)
├─ 2~3 → sampleView_.showComingSoon()
└─ else → mainView_.showInvalidInput()
```

### AppController

```cpp
// controller/AppController.h
class AppController {
public:
    AppController(MainView& mainView,
                  SampleView& sampleView,
                  IRepository<Sample>& sampleRepo);
    void run();
private:
    MainView&            mainView_;
    OrderController      orderCtrl_;
    ProductionController prodCtrl_;
};
```

### main.cpp

```cpp
SampleRepository sampleRepo;   // data/samples.json 자동 로드
MainView   mainView;
SampleView sampleView;
AppController app(mainView, sampleView, sampleRepo);
app.run();
```

---

## 8. vcxproj 신규 등록 목록

```xml
<ClCompile Include="model\SampleRepository.cpp" />
<ClCompile Include="view\SampleView.cpp" />
<ClCompile Include="test\SampleRepositoryTest.cpp" />
<ClCompile Include="test\SM01Test.cpp" />
```

---

## 9. 테스트 계획 (gtest)

### SampleRepositoryTest — JsonRepository + 영속성

| 테스트 ID | 설명 |
|-----------|------|
| `SampleRepositoryTest.CreateAssignsAutoIncrementId` | 첫 create → id=1, 두 번째 → id=2 |
| `SampleRepositoryTest.FindAllEmptyOnStart` | 파일 없이 시작 시 빈 배열 반환 |
| `SampleRepositoryTest.FindAllReturnsCreatedItems` | create 후 findAll에 포함 |
| `SampleRepositoryTest.FindByIdReturnsItem` | create 후 findById 성공 |
| `SampleRepositoryTest.FindByIdReturnsNulloptForMissing` | 존재하지 않는 id → nullopt |
| `SampleRepositoryTest.PersistsAcrossInstances` | 첫 인스턴스 create → 두 번째 인스턴스 findAll에 포함 |
| `SampleRepositoryTest.StockAlwaysInitializedToZero` | create 시 stock 강제 0 |

> JsonRepository는 헤더 온리 템플릿이므로 SampleRepository를 통해 함께 검증한다.
> 테스트용 임시 파일 경로: `data/test_samples.json` — SetUp/TearDown으로 생성·삭제

### SM01Test — 등록 흐름 (MockSampleView + MockSampleRepository)

```cpp
class MockSampleView : public SampleView {
    MOCK_METHOD(void,        showSampleMenu,         (), (override));
    MOCK_METHOD(std::string, inputName,              (), (override));
    MOCK_METHOD(int,         inputAvgProductionTime, (), (override));
    MOCK_METHOD(double,      inputYield,             (), (override));
    MOCK_METHOD(void,        showRegistered,  (const Sample&), (override));
    MOCK_METHOD(void,        showInvalidInput, (const std::string&), (override));
    MOCK_METHOD(void,        showComingSoon,  (), (override));
};

class MockSampleRepository : public IRepository<Sample> {
    MOCK_METHOD(Sample,              create,   (Sample),        (override));
    MOCK_METHOD(std::vector<Sample>, findAll,  (),              (override));
    MOCK_METHOD(std::optional<Sample>, findById, (int),         (override));
    MOCK_METHOD(bool,                update,   (const Sample&), (override));
    MOCK_METHOD(bool,                remove,   (int),           (override));
};
```

| 테스트 ID | 설명 |
|-----------|------|
| `SM01Test.RegisterSampleSuccess` | 유효 입력 → create 호출, showRegistered 호출 |
| `SM01Test.RejectsEmptyName` | 이름 빈 문자열 → showInvalidInput, create 미호출 |
| `SM01Test.RejectsNonPositiveAvgTime` | avgTime ≤ 0 → showInvalidInput |
| `SM01Test.RejectsYieldZero` | yield = 0.0 → showInvalidInput |
| `SM01Test.RejectsYieldAboveOne` | yield > 1.0 → showInvalidInput |

### 기존 테스트 수정

`AppControllerTest`, `ProductionControllerTest` — 생성자에 `MockSampleView`, `MockSampleRepository` 추가 주입.

---

## 10. 구현 순서 (TDD 사이클)

```
1. lib/json.hpp 배치
2. model/Sample.h, IRepository.h 작성
3. SampleRepositoryTest 작성 → RED
4. JsonRepository.h, SampleRepository 구현 → GREEN
5. SampleView 작성 (pure I/O, 먼저 구현)
6. SM01Test 작성 → RED
7. ProductionController::registerSample() 구현 → GREEN
8. AppController·ProductionController 생성자 업데이트
9. 기존 테스트(AppControllerTest, ProductionControllerTest) 수정
10. main.cpp 업데이트
11. vcxproj 등록
12. 전체 빌드 + 테스트 실행 확인
```

---

## 11. 검토 포인트

- `JsonRepository<T>`를 헤더 온리 템플릿으로 구현 (`.cpp` 없음) — 동의하는가?
- `ProductionController`가 `IRepository<Sample>&`를 받는 구조 (SampleRepository 구체 클래스 대신) — 동의하는가?
- `data/` 디렉터리를 `.gitignore`에 추가할 것인가? (런타임 생성 데이터)
- 테스트용 임시 파일(`data/test_samples_*.json`) 생성·삭제 전략 — `SetUp/TearDown` 사용
