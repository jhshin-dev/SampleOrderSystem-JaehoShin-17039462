#pragma once
#include "JsonRepository.h"
#include "Sample.h"
#include <string>

class SampleRepository : public JsonRepository<Sample> {
public:
    explicit SampleRepository(const std::string& filePath = "data/samples.json");
    Sample create(Sample entity) override;
};
