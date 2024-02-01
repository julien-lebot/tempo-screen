
#include <ctime>
#include <cstdint>
#include <charconv>
#include <chrono>
#include <vector>
#include <iostream>
#include <tuple>

#include <esp_system.h>
#include <esp_timer.h>
#include <esp_event.h>
#include <esp_netif_sntp.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_netif.h>

#include <esp_timer_cxx.hpp>

#include "device.hpp"
#include "esp32s3.hpp"
#include "spi_lcd.hpp"
#include "wifi.hpp"
#include "mqtt.hpp"
#include "app_ui.hpp"
#include "data_model/electricity_rate_provider.hpp"

#include "date/date.h"
#include "date/ptz.h"

typedef Device<ESP32S3> ESPDevice;
static ESPDevice dev;
static SpiLcd<ESPDevice> lcd;
static AppUi ui;
static MQTTClient mqtt;
static DummyElectricityRateProvider rateProvider;

void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int user = (int)t->user;
    gpio_set_level(ESPDevice::DeviceDefinitions::dc_pin, user);
}

#include <lvgl.h>

const int lvgl_tick_period_ms = 2;

void lvgl_tick(void *arg)
{
    lv_tick_inc(lvgl_tick_period_ms);
}

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    lcd.set_window(area->x1, area->y1, area->x2, area->y2, DISP_SPI_SEND_POLLING);
    auto size = lv_area_get_width(area) * lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(px_map, size);
    lcd.send_data(px_map, size * sizeof(uint16_t), DISP_SPI_SEND_POLLING);
    lv_display_flush_ready(disp);
}

static lv_color_t buf[ESPDevice::DeviceDefinitions::screen_width * ESPDevice::DeviceDefinitions::screen_height / 10];

time_t make_time(struct tm *src, int hour, int min)
{
    struct tm dest;
    dest.tm_mday = src->tm_mday;
    dest.tm_mon = src->tm_mon;
    dest.tm_year = src->tm_year;
    dest.tm_mday = src->tm_mday;
    dest.tm_hour = hour;
    dest.tm_min = min;
    dest.tm_sec = 0;
    return mktime(&dest);
}

class ElectricityRateViewModel
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
    ElectricityRateViewModel(AppUi &ui, IElectricityRateProvider &electricityRateProvider)
    : _ui(ui)
    , _clockTimer(std::bind(&ElectricityRateViewModel::clock_tick, this))
    , _current_rate_index(-1)
    {
        _rates = electricityRateProvider.get_rates();
        electricityRateProvider.subscribe(std::bind(&ElectricityRateViewModel::on_rates_changed, this, std::placeholders::_1));
        _clockTimer.start_periodic(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(500)));
    }
};

void setup_network(void *)
{
    WifiClient wifi;

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    esp_netif_sntp_sync_wait(portMAX_DELAY);

    //time_t now = time(nullptr);
    //printf("Time acquired, it is: %s\n", ctime(&now));

    mqtt.connect([](esp_mqtt_client_config_t &cfg) {
    });
    mqtt.subscribe("power_meter/phase_1_power", [](auto payload)
    {
        uint16_t power = 0;
        std::from_chars(payload.data(), payload.data() + payload.length(), power);
        ui.set_phase_power(0, power);
    });
    mqtt.subscribe("power_meter/phase_2_power", [](auto payload)
    {
        uint16_t power = 0;
        std::from_chars(payload.data(), payload.data() + payload.length(), power);
        ui.set_phase_power(1, power);
    });
    mqtt.subscribe("power_meter/phase_3_power", [](auto payload)
    {
        uint16_t power = 0;
        std::from_chars(payload.data(), payload.data() + payload.length(), power);
        ui.set_phase_power(2, power);
    });

    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

void setup_ui(void *)
{
    lv_init();
    lv_display_t * disp = lv_display_create(ESPDevice::DeviceDefinitions::screen_width, ESPDevice::DeviceDefinitions::screen_height);
    lv_display_set_flush_cb(disp, disp_flush);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    ui.init();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &lvgl_tick,
      .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, lvgl_tick_period_ms * 1000);

    new ElectricityRateViewModel(ui, rateProvider);
}

extern "C"
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // TODO: Make configurable
    setenv("TZ", "CET-1", 1);
    tzset();

    xTaskCreatePinnedToCore(TaskFunction_t(&setup_network), "setup_network", 4096, nullptr, 10, nullptr, 0);
    setup_ui(nullptr);
}
