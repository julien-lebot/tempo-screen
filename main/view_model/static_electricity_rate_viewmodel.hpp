#pragma once

#include <vector>
#include <chrono>

#include <esp_timer.h>

#include "data_model/electricity_rate_provider.hpp"
#include "app_ui.hpp"

#include "date/date.h"
#include "date/ptz.h"

/// @brief The StaticElectricityRateViewModel displays a static range of tariffs.
/// It's great for demos.
class StaticElectricityRateViewModel
{
private:
    AppUi &_ui;
    std::vector<ElectricityRate> _rates;
    idf::esp_timer::ESPTimer _clockTimer;
    int _current_rate_index;

    void clock_tick()
    {
        if (_rates.empty())
        {
            printf("No rates received yet\n");
            return;
        }

        //const char* tz = getenv("TZ");
        auto today = date::zoned_time{ Posix::time_zone{"CET-1CEST,M3.5.0,M10.5.0/3"}, std::chrono::system_clock::now() };
        //std::cout << "The time now is " << today << std::endl;

        float min_price = std::numeric_limits<float>::max();
        float max_price = std::numeric_limits<float>::min();
        std::vector<std::tuple<const ElectricityRate*, date::local_time<AppUi::duration_t>>> clocks;
        for (const auto& rate : _rates)
        {
            clocks.emplace_back(std::make_tuple(&rate, std::chrono::floor<std::chrono::days>(today.get_local_time()) - std::chrono::days{ 1 } + std::chrono::hours{ rate.startTime.hours } + std::chrono::minutes{ rate.startTime.minutes }));
            clocks.emplace_back(std::make_tuple(&rate, std::chrono::floor<std::chrono::days>(today.get_local_time()) + std::chrono::hours{ rate.startTime.hours } + std::chrono::minutes{ rate.startTime.minutes }));
            clocks.emplace_back(std::make_tuple(&rate, std::chrono::floor<std::chrono::days>(today.get_local_time()) + std::chrono::days{ 1 } + std::chrono::hours{ rate.startTime.hours } + std::chrono::minutes{ rate.startTime.minutes }));
            min_price = std::min(min_price, rate.price);
            max_price = std::max(max_price, rate.price);
        }

        auto smallest_pos_diff = AppUi::duration_t::max();
        auto smallest_neg_diff = AppUi::duration_t::min();
        int current_rate_index = -1;
        int next_rate_index = -1;
        
        for (int i = 0; i < clocks.size(); ++i)
        {
            auto diff = std::chrono::duration_cast<AppUi::duration_t>(std::get<1>(clocks[i]) - today.get_local_time());
            if (diff > AppUi::duration_t::zero() && diff < smallest_pos_diff)
            {
                smallest_pos_diff = diff;
                next_rate_index = i;
            }
            if (diff < AppUi::duration_t::zero() && diff > smallest_neg_diff)
            {
                smallest_neg_diff = diff;
                current_rate_index = i;
            }
        }

        if (current_rate_index < 0 || next_rate_index < 0)
        {
            // Maybe the time has not been initialized yet
            printf("No valid rates detected: %d, %d\n", current_rate_index, next_rate_index);
            return;
        }

        if (_current_rate_index != current_rate_index)
        {
            // We changed rate
            _current_rate_index = current_rate_index;

            _ui.set_background_color(std::get<0>(clocks[current_rate_index])->color);
            float current_price = std::get<0>(clocks[current_rate_index])->price;
            float mapped_price = (current_price - min_price) / (max_price - min_price);
            const int min_stop = 127;
            const int max_stop = 255;
            int gradient_stop = 255 - static_cast<int>(min_stop + (max_stop - min_stop) * mapped_price);
            _ui.set_gradient_stop(gradient_stop);
            _ui.set_tarif_name(std::get<0>(clocks[current_rate_index])->name);
            _ui.set_next_color(std::get<0>(clocks[next_rate_index])->color);
        }
        _ui.set_remaining_duration(smallest_pos_diff);
    }

    void on_rates_changed(std::vector<ElectricityRate> const &newRates)
    {
        _rates = newRates;
    }
public:
    StaticElectricityRateViewModel(AppUi &ui, IElectricityRateProvider &electricityRateProvider)
    : _ui(ui)
    , _clockTimer(std::bind(&StaticElectricityRateViewModel::clock_tick, this))
    , _current_rate_index(-1)
    {
        _rates = electricityRateProvider.get_rates();
        electricityRateProvider.subscribe(std::bind(&StaticElectricityRateViewModel::on_rates_changed, this, std::placeholders::_1));
        _clockTimer.start_periodic(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(500)));
    }
};
