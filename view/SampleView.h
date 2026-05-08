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
    virtual void        showSampleList(const std::vector<Sample>& samples);
    virtual std::string inputSearchKeyword();
    virtual void        showNoResult();
};
