#include <cstdint>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include "device.hpp"
#include "esp32s3.hpp"
#include "spi_lcd.hpp"
#include "app_ui.hpp"

typedef Device<ESP32S3> ESPDevice;
static ESPDevice dev;
static SpiLcd<ESPDevice> lcd;
static AppUi ui;

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

extern "C"
void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    lcd.set_window(area->x1, area->y1, area->x2, area->y2, DISP_SPI_SEND_POLLING);
    auto size = lv_area_get_width(area) * lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(px_map, size);
    lcd.send_data(px_map, size * sizeof(uint16_t), DISP_SPI_SEND_POLLING);
    lv_display_flush_ready(disp);
}

static lv_color_t buf[ESPDevice::DeviceDefinitions::screen_width * ESPDevice::DeviceDefinitions::screen_height / 10];

extern "C"
void app_main(void)
{
    lv_init();
    lv_display_t * disp = lv_display_create(ESPDevice::DeviceDefinitions::screen_width, ESPDevice::DeviceDefinitions::screen_height);
    lv_display_set_flush_cb(disp, disp_flush);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lvgl_app_main();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &lvgl_tick,
      .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, lvgl_tick_period_ms * 1000);
}