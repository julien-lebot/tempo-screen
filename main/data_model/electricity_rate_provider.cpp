#include <ctime>

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
                .hours = 16,
                .minutes = 50
            },
            .price = 0.246f,
            .color = lv_color_hex(0xFF0000)
        },
    };
}

std::vector<ElectricityRate> get_rates_tempo()
{
    int hour = 2;
    int minutes = 33;
    return
    {
        {
            .name = "Bleu HC",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 1
            },
            .price = 0.1231f,
            .color = lv_color_hex(0x0000FF)
        },
        {
            .name = "Blanc HP",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 2
            },
            .price = 0.1773f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Blanc HC",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 3
            },
            .price = 0.1412f,
            .color = lv_color_hex(0xFFFFFF)
        },
        {
            .name = "Rouge HP",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 4
            },
            .price = 0.6274f,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "Rouge HC",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 5
            },
            .price = 0.1509f,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "Bleu HP",
            .startTime = 
            {
                .hours = hour,
                .minutes = minutes + 6
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