#include <chrono>
#include "date/date.h"
#include "date/ptz.h"
#include "electricity_rate_provider.hpp"

std::vector<ElectricityRate> get_rates_hp_hc()
{
    return 
    {
        {
            .name = "Heures creuses",
            .startTime = 
            {
                .hours = 1,
                .minutes = 50
            },
            .price = 0.18f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Heures pleines",
            .startTime = 
            {
                .hours = 6,
                .minutes = 50
            },
            .price = 0.246f,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "Heures creuses",
            .startTime = 
            {
                .hours = 13,
                .minutes = 50
            },
            .price = 0.18f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Heures pleines",
            .startTime = 
            {
                .hours = 15,
                .minutes = 50
            },
            .price = 0.246f,
            .color = lv_color_hex(0xFF0000)
        },
    };
}

std::vector<ElectricityRate> get_rates_tempo()
{
    auto today = date::zoned_time{ Posix::time_zone{"CET-1CEST,M3.5.0,M10.5.0/3"}, std::chrono::system_clock::now() };
    auto x =  std::chrono::floor<std::chrono::days>(today.get_local_time());
    date::hh_mm_ss hhmmss{ std::chrono::floor<std::chrono::milliseconds>(x - today.get_local_time()) };

    return
    {
        {
            .name = "Bleu HC",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 1
            },
            .price = 0.1231f,
            .color = lv_color_hex(0x0000FF)
        },
        {
            .name = "Blanc HP",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 2
            },
            .price = 0.1773f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Blanc HC",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 3
            },
            .price = 0.1412f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Rouge HP",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 4
            },
            .price = 0.6274f,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "Rouge HC",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 5
            },
            .price = 0.1509f,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "Bleu HP",
            .startTime = 
            {
                .hours = (int)hhmmss.hours().count(),
                .minutes = (int)hhmmss.minutes().count() + 6
            },
            .price = 0.1498f,
            .color = lv_color_hex(0x0000FF)
        },
    };
}

DummyElectricityRateProvider::DummyElectricityRateProvider()
: _rates(get_rates_tempo())
{
}

std::vector<ElectricityRate> const &DummyElectricityRateProvider::get_rates() const
{
    return _rates;
}

void DummyElectricityRateProvider::subscribe(IElectricityRateProvider::subscription_fn subscribe_fn)
{
    _subscribers.push_back(subscribe_fn);
}
