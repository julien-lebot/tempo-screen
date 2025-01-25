#pragma once

#include <vector>
#include <chrono>

#include <esp_timer.h>

#include "data_model/electricity_rate_provider.hpp"
#include "app_ui.hpp"

#include "date/date.h"
#include "date/ptz.h"

class HomeAssistantElectricityRateViewModel
{
private:
    AppUi &_ui;
    std::vector<ElectricityRate> _rates;
    idf::esp_timer::ESPTimer _clockTimer;

    void clock_tick()
    {
        if (_rates.empty())
        {
            printf("No rates received yet\n");
            return;
        }

        auto today = date::zoned_time{ Posix::time_zone{"CET-1CEST,M3.5.0,M10.5.0/3"}, std::chrono::system_clock::now() };

        float min_price = 0.0f;
        float max_price = 1.0f;

        auto smallest_pos_diff = AppUi::duration_t::max();
        for (int i = 0; i < _rates.size(); ++i)
        {
            auto rateTime = std::chrono::floor<std::chrono::days>(today.get_local_time()) + std::chrono::hours{ _rates[i].startTime.hours } + std::chrono::minutes{ _rates[i].startTime.minutes };
            auto diff = std::chrono::duration_cast<AppUi::duration_t>(rateTime - today.get_local_time());
            if (diff > AppUi::duration_t::zero() && diff < smallest_pos_diff)
            {
                smallest_pos_diff = diff;
            }
        }

        _ui.set_background_color(_rates[0].color);
        float current_price = _rates[0].price;
        float mapped_price = (current_price - min_price) / (max_price - min_price);
        const int min_stop = 127;
        const int max_stop = 255;
        int gradient_stop = 255 - static_cast<int>(min_stop + (max_stop - min_stop) * mapped_price);
        _ui.set_gradient_stop(gradient_stop);
        _ui.set_tarif_name(_rates[0].name);
        _ui.set_next_color(_rates[1].color);
        _ui.set_remaining_duration(smallest_pos_diff);
    }

    void on_rates_changed(std::vector<ElectricityRate> const &newRates)
    {
        _rates = newRates;
    }
public:
    HomeAssistantElectricityRateViewModel(AppUi &ui, IElectricityRateProvider &electricityRateProvider)
    : _ui(ui)
    , _clockTimer(std::bind(&HomeAssistantElectricityRateViewModel::clock_tick, this))
    {
        _rates = electricityRateProvider.get_rates();
        electricityRateProvider.subscribe(std::bind(&HomeAssistantElectricityRateViewModel::on_rates_changed, this, std::placeholders::_1));
        _clockTimer.start_periodic(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(500)));
    }
};
