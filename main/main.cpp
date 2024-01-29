#include <cstdint>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include "device.hpp"
#include "esp32s3.hpp"
#include "spi_lcd.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"

typedef Device<ESP32S3> ESPDevice;
static ESPDevice dev;
static SpiLcd<ESPDevice> lcd;

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

struct phase_definition
{
    lv_obj_t *phase_object;
    lv_obj_t *outline_object;
    lv_color_t phase_color;
    lv_anim_t demo_anim;
    uint32_t time;
    uint32_t playback_time;
};

void lvgl_app_main()
{
    lv_obj_t * ui_Screen1 = lv_screen_active();
    lv_obj_t * ui_Phase_1_power;
    lv_obj_t * ui_Phase_2_power;
    lv_obj_t * ui_Phase_3_power;
    lv_obj_t * ui_Remaining_time;
    lv_obj_t * ui_next_color;
    lv_obj_t * ui_Tarif;

    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Screen1, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Screen1, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Screen1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen1, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    uint8_t size = 230;
    int8_t rotation = -30;
    phase_definition phases[] = {
        {
            .phase_color = lv_color_hex(0xFF6A48),
            .time = 2000,
            .playback_time = 500
        },
        {
            .phase_color = lv_color_hex(0xEF5350),
            .time = 1000,
            .playback_time = 1000
        },
        {
            .phase_color = lv_color_hex(0xE57373),
            .time = 1000,
            .playback_time = 2000
        }
    };
    for (auto phase : phases)
    {
#if outline
        phase.outline_object = lv_arc_create(ui_Screen1);
        lv_obj_set_width(phase.outline_object, size);
        lv_obj_set_height(phase.outline_object, size);
        lv_obj_set_align(phase.outline_object, LV_ALIGN_CENTER);
        lv_obj_remove_style(phase.outline_object, nullptr, LV_PART_KNOB);
        lv_obj_clear_flag(phase.outline_object, LV_OBJ_FLAG_CLICKABLE);
        lv_arc_set_value(phase.outline_object, 0);
        lv_arc_set_bg_angles(phase.outline_object, 145, 305);
        lv_arc_set_rotation(phase.outline_object, rotation);
        lv_obj_set_style_arc_color(phase.outline_object, lv_color_hex(0x808080), LV_PART_MAIN);
        lv_obj_set_x(phase.outline_object, -2);
        lv_obj_set_y(phase.outline_object, -2);
        lv_obj_set_style_arc_width(phase.outline_object, 20, LV_PART_MAIN);
#endif

        phase.phase_object = lv_arc_create(ui_Screen1);
        lv_obj_set_width(phase.phase_object, size);
        lv_obj_set_height(phase.phase_object, size);
        
        lv_obj_set_align(phase.phase_object, LV_ALIGN_CENTER);
        lv_obj_remove_style(phase.phase_object, nullptr, LV_PART_KNOB);
        lv_obj_clear_flag(phase.phase_object, LV_OBJ_FLAG_CLICKABLE);
        lv_arc_set_value(phase.phase_object, 64);
        lv_arc_set_bg_angles(phase.phase_object, 145, 305);
        lv_arc_set_rotation(phase.phase_object, rotation);
        lv_obj_set_style_arc_color(phase.phase_object, lv_color_hex(0x191919), LV_PART_MAIN);
        lv_obj_set_style_arc_color(phase.phase_object, phase.phase_color, LV_PART_INDICATOR);

        lv_obj_set_style_arc_width(phase.phase_object, 15, LV_PART_MAIN);
        lv_obj_set_style_arc_width(phase.phase_object, 15, LV_PART_INDICATOR);


        lv_anim_init(&phase.demo_anim);
        lv_anim_set_exec_cb(&phase.demo_anim, lv_anim_exec_xcb_t(lv_arc_set_value));
        lv_anim_set_values(&phase.demo_anim, 0, 100);
        lv_anim_set_repeat_delay(&phase.demo_anim, 100);
        lv_anim_set_playback_delay(&phase.demo_anim, 100);
        lv_anim_set_repeat_count(&phase.demo_anim, LV_ANIM_REPEAT_INFINITE);

        lv_anim_set_time(&phase.demo_anim, phase.time);
        lv_anim_set_playback_time(&phase.demo_anim, phase.playback_time);
        lv_anim_set_var(&phase.demo_anim, phase.phase_object);
        lv_anim_start(&phase.demo_anim);

        size -= 45;
    }

    ui_next_color = lv_arc_create(ui_Screen1);
    lv_obj_set_width(ui_next_color, 235);
    lv_obj_set_height(ui_next_color, 235);
    lv_obj_set_align(ui_next_color, LV_ALIGN_CENTER);
    lv_arc_set_value(ui_next_color, 45);
    lv_arc_set_bg_angles(ui_next_color, 0, 45);
    lv_arc_set_mode(ui_next_color, LV_ARC_MODE_REVERSE);
    lv_arc_set_rotation(ui_next_color, 337);
    lv_obj_remove_style(ui_next_color, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(ui_next_color, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ui_next_color, lv_color_hex(0x2196F3), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ui_next_color, lv_color_hex(0x191919), LV_PART_MAIN);

    ui_Phase_1_power = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Phase_1_power, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Phase_1_power, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Phase_1_power, -3);
    lv_obj_set_y(ui_Phase_1_power, 100);
    lv_obj_set_align(ui_Phase_1_power, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Phase_1_power, "12000W");
    lv_obj_set_style_text_color(ui_Phase_1_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Phase_1_power, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Phase_2_power = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Phase_2_power, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Phase_2_power, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Phase_2_power, 5);
    lv_obj_set_y(ui_Phase_2_power, 82);
    lv_obj_set_align(ui_Phase_2_power, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Phase_2_power, "12000W");
    lv_obj_set_style_text_color(ui_Phase_2_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Phase_2_power, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Phase_3_power = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Phase_3_power, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Phase_3_power, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Phase_3_power, 13);
    lv_obj_set_y(ui_Phase_3_power, 64);
    lv_obj_set_align(ui_Phase_3_power, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Phase_3_power, "12000W");
    lv_obj_set_style_text_color(ui_Phase_3_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Phase_3_power, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Remaining_time = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Remaining_time, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Remaining_time, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Remaining_time, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Remaining_time, "-01:34");
    lv_obj_set_x(ui_Remaining_time, 24);
    lv_obj_set_y(ui_Remaining_time, 14);
    lv_obj_set_style_text_color(ui_Remaining_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Remaining_time, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Remaining_time, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Tarif = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Tarif, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Tarif, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Tarif, 24);
    lv_obj_set_y(ui_Tarif, -24);
    lv_obj_set_align(ui_Tarif, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Tarif, "HEURES CREUSES");
    lv_obj_set_style_text_color(ui_Tarif, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Tarif, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Tarif, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
}

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
    while (1)
    {
        uint32_t time_till_next = lv_timer_handler();
        vTaskDelay(time_till_next / portTICK_PERIOD_MS);
    }
}