#include <charconv>
#include <algorithm>
#include <string_view>
#include <locale>
#include <map>

#include "electricity_rate_provider.hpp"
#include "mqtt.hpp"

static std::map<std::string, lv_color_t> colorTable =
{
    {
        "bleu", lv_color_hex(0x0074ba)
    },
    {
        "blanc", lv_color_hex(0xffffff)
    },
    {
        "rouge", lv_color_hex(0xff0000)
    },
};

lv_color_t stringToColor(std::string stringColor)
{
    std::transform(stringColor.begin(), stringColor.end(), stringColor.begin(), [](unsigned char c){ return std::tolower(c); });
    if (colorTable.find(stringColor) == colorTable.end())
    {
        return lv_color_hex(0x000000);
    }
    return colorTable[stringColor];
}

HomeAssistantElectricityRateProvider::HomeAssistantElectricityRateProvider(MQTTClient &mqtt_client)
{
    mqtt_client.subscribe("power_meter/current_rate", [&](auto payload)
    {
        _current_rates[0].name = payload;
        _current_rates[0].color = stringToColor(_current_rates[0].name);
        notify_rates_changed();
    });
    mqtt_client.subscribe("power_meter/next_rate", [&](auto payload)
    {
        _current_rates[1].name = payload;
        _current_rates[1].color = stringToColor(_current_rates[1].name);
        notify_rates_changed();
    });
    mqtt_client.subscribe("power_meter/price", [&](auto payload)
    {
        float price = 0.0f;
        std::from_chars(payload.data(), payload.data() + payload.length(), price);
        _current_rates[0].price = price;
        notify_rates_changed();
    });

    _current_rates =
    {
        {
            .name = "",
            .startTime = 
            {
                .hours = 22,
                .minutes = 0
            },
            .price = 0.0,
            .color = lv_color_hex(0xFF0000)
        },
        {
            .name = "",
            .startTime = 
            {
                .hours = 6,
                .minutes = 0
            },
            .price = 0.0,
            .color = lv_color_hex(0x0000FF)
        }
    };
}

void HomeAssistantElectricityRateProvider::notify_rates_changed()
{
    for (auto &&s : _subscribers)
    {
        s(_current_rates);
    }
}

std::vector<ElectricityRate> const &HomeAssistantElectricityRateProvider::get_rates() const
{
    return _current_rates;
}

void HomeAssistantElectricityRateProvider::subscribe(IElectricityRateProvider::subscription_fn subscribe_fn)
{
    _subscribers.push_back(subscribe_fn);
}
