#pragma once

#include <string>
#include <functional>
#include <vector>
#include <lvgl.h>

// Defines a point in time where the electricity rate changes
// There is no day field as the TimePoint lies strictly within
// a 24h period.
struct TimePoint
{
    int hours;
    int minutes;
};

struct ElectricityRate
{
    std::string name;
    TimePoint startTime;
    float price;
    lv_color_t color;
};

class IElectricityRateProvider
{
protected:
    virtual ~IElectricityRateProvider() {}
public:
    typedef std::function<void(std::vector<ElectricityRate> const &)> subscription_fn;
    virtual std::vector<ElectricityRate> const &get_rates() const = 0;
    virtual void subscribe(subscription_fn subscribe_fn) = 0;
};

class DummyElectricityRateProvider : public IElectricityRateProvider
{
private:
    std::vector<ElectricityRate> _rates;
    std::vector<IElectricityRateProvider::subscription_fn> _subscribers;
public:
    DummyElectricityRateProvider();
    std::vector<ElectricityRate> const &get_rates() const;
    void subscribe(IElectricityRateProvider::subscription_fn subscribe_fn);
};