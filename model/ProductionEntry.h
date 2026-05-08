#pragma once
#include <string>

struct ProductionEntry {
    int         orderId;
    std::string sampleName;
    std::string customerName;
    int         quantity;
    int         shortage;
    int         actualQty;
    int         totalTime;
    std::string updatedAt;
};
