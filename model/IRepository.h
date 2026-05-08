#pragma once
#include <vector>
#include <optional>

template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual T                    create(T entity)        = 0;
    virtual std::vector<T>       findAll()               = 0;
    virtual std::optional<T>     findById(int id)        = 0;
    virtual bool                 update(const T& entity) = 0;
    virtual bool                 remove(int id)          = 0;
};
