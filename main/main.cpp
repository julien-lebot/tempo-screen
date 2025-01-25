
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
#include "view_model/ha_electricity_rate_viewmodel.hpp"

#include "date/date.h"
#include "date/ptz.h"

typedef Device<ESP32S3> ESPDevice;
static ESPDevice dev;
static SpiLcd<ESPDevice> lcd;
static AppUi ui;
static MQTTClient mqtt;

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

void setup_network(void *)
{
    //time_t now = time(nullptr);
    //printf("Time acquired, it is: %s\n", ctime(&now));

    mqtt.connect([](esp_mqtt_client_config_t &cfg) {
        cfg.broker.address.uri = "mqtt://10.10.42.2";
        cfg.credentials.username = "julien.lebot";
        cfg.credentials.authentication.password = "h[kCY-wv";
    });
    mqtt.subscribe("power_meter/phase_1_power", [](auto payload)
    {
        int16_t power = 0;
        std::from_chars(payload.data(), payload.data() + payload.length(), power);
        ui.set_phase_power(0, power);
    });
    mqtt.subscribe("power_meter/phase_2_power", [](auto payload)
    {
        int16_t power = 0;
        std::from_chars(payload.data(), payload.data() + payload.length(), power);
        ui.set_phase_power(1, power);
    });
    mqtt.subscribe("power_meter/phase_3_power", [](auto payload)
    {
        int16_t power = 0;
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

    static HomeAssistantElectricityRateProvider p(mqtt);
    new HomeAssistantElectricityRateViewModel(ui, p);
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

    WifiClient wifi;
    wifi.connect("HOME_LEGACY", "cra2qm5q");

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    esp_netif_sntp_sync_wait(portMAX_DELAY);

    xTaskCreatePinnedToCore(TaskFunction_t(&setup_network), "setup_network", 4096, nullptr, 10, nullptr, 0);
    setup_ui(nullptr);
}
