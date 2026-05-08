#include "SampleRepository.h"

SampleRepository::SampleRepository(const std::string& filePath)
    : JsonRepository<Sample>(filePath) {}

Sample SampleRepository::create(Sample entity) {
    entity.stock = 0;
    return JsonRepository<Sample>::create(entity);
}
