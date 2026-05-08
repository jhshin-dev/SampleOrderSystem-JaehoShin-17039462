#pragma once
#include <string>
#include "../lib/json.hpp"

struct Sample {
    int         id                = 0;
    std::string name;
    int         avgProductionTime = 0;
    double      yield             = 0.0;
    int         stock             = 0;

    bool operator==(const Sample& o) const {
        return id == o.id && name == o.name
            && avgProductionTime == o.avgProductionTime
            && yield == o.yield && stock == o.stock;
    }
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
