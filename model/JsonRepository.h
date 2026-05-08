#pragma once
#include "IRepository.h"
#include "../lib/json.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

template<typename T>
class JsonRepository : public IRepository<T> {
public:
    explicit JsonRepository(const std::string& filePath)
        : filePath_(filePath) {
        load();
    }

    T create(T entity) override {
        entity.id = nextId_++;
        items_.push_back(entity);
        save();
        return entity;
    }

    std::vector<T> findAll() override {
        return items_;
    }

    std::optional<T> findById(int id) override {
        auto it = std::find_if(items_.begin(), items_.end(),
                               [id](const T& t) { return t.id == id; });
        if (it == items_.end()) return std::nullopt;
        return *it;
    }

    bool update(const T& entity) override {
        auto it = std::find_if(items_.begin(), items_.end(),
                               [&](const T& t) { return t.id == entity.id; });
        if (it == items_.end()) return false;
        *it = entity;
        save();
        return true;
    }

    bool remove(int id) override {
        auto it = std::find_if(items_.begin(), items_.end(),
                               [id](const T& t) { return t.id == id; });
        if (it == items_.end()) return false;
        items_.erase(it);
        save();
        return true;
    }

protected:
    std::vector<T> items_;

private:
    std::string filePath_;
    int         nextId_ = 1;

    void load() {
        if (!std::filesystem::exists(filePath_)) return;
        std::ifstream f(filePath_);
        if (!f.is_open()) return;
        try {
            nlohmann::json j;
            f >> j;
            items_ = j.get<std::vector<T>>();
            for (const auto& item : items_)
                if (item.id >= nextId_) nextId_ = item.id + 1;
        } catch (...) {}
    }

    void save() const {
        std::filesystem::create_directories(
            std::filesystem::path(filePath_).parent_path());
        std::ofstream f(filePath_);
        f << nlohmann::json(items_).dump(2);
    }
};
