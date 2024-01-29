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

static void anim_x_cb(lv_obj_t * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void anim_size_cb(lv_obj_t * var, int32_t v)
{
    lv_obj_set_size(var, v, v);
}

void display_animation()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);

    lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 10, 50);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_playback_duration(&a, 300);
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    lv_anim_set_exec_cb(&a, lv_anim_exec_xcb_t(anim_size_cb));
    lv_anim_start(&a);
    lv_anim_set_exec_cb(&a, lv_anim_exec_xcb_t(anim_x_cb));
    lv_anim_set_values(&a, 10, 240);
    lv_anim_start(&a);
}

void display_spinner()
{
    lv_obj_t * spinner = lv_spinner_create(lv_screen_active());
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_center(spinner);
    lv_spinner_set_anim_params(spinner, 10000, 200);
}

void display_round_thermometer()
{
    lv_obj_t * scale = lv_scale_create(lv_screen_active());
    lv_obj_set_size(scale, 150, 150);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_center(scale);

    lv_scale_set_total_tick_count(scale, 21);
    lv_scale_set_major_tick_every(scale, 5);

    lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale, 0, 100);

    static const char * custom_labels[] = {"0 °C", "25 °C", "50 °C", "75 °C", "100 °C", NULL};
    lv_scale_set_text_src(scale, custom_labels);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);      /*Tick length*/
    lv_style_set_line_width(&indicator_style, 2U);  /*Tick width*/
    lv_obj_add_style(scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);         /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U);    /*Tick width*/
    lv_obj_add_style(scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    /* Main line properties */
    lv_style_set_arc_color(&main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 2U); /*Tick width*/
    lv_obj_add_style(scale, &main_line_style, LV_PART_MAIN);

    /* Add a section */
    static lv_style_t section_minor_tick_style;
    static lv_style_t section_label_style;
    static lv_style_t section_main_line_style;

    lv_style_init(&section_label_style);
    lv_style_init(&section_minor_tick_style);
    lv_style_init(&section_main_line_style);

    /* Label style properties */
    lv_style_set_text_font(&section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_set_line_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_label_style, 5U); /*Tick width*/

    lv_style_set_line_color(&section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&section_minor_tick_style, 4U); /*Tick width*/

    /* Main line properties */
    lv_style_set_arc_color(&section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_arc_width(&section_main_line_style, 4U); /*Tick width*/

    /* Configure section styles */
    lv_scale_section_t * section = lv_scale_add_section(scale);
    lv_scale_section_set_range(section, 75, 100);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &section_label_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &section_minor_tick_style);
    lv_scale_section_set_style(section, LV_PART_MAIN, &section_main_line_style);
}

LV_FONT_DECLARE(ui_font_H1);
LV_FONT_DECLARE(ui_font_Title);

LV_IMAGE_DECLARE(hc);

void create_arc()
{
    lv_obj_t *ui_arc_spo2 = lv_arc_create(lv_screen_active());
    lv_obj_set_width(ui_arc_spo2, 240);
    lv_obj_set_height(ui_arc_spo2, 240);
    lv_obj_set_align(ui_arc_spo2, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_arc_spo2, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_value(ui_arc_spo2, 91);
    lv_arc_set_bg_angles(ui_arc_spo2, 145, 335);
    lv_obj_set_style_arc_color(ui_arc_spo2, lv_color_hex(0x414141), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc_spo2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_arc_spo2, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc_spo2, lv_color_hex(0x41A1E6), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc_spo2, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_arc_spo2, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc_spo2, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc_spo2, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
}

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
#if false
    lv_obj_t * ui_bg_4 = lv_img_create(lv_screen_active());
    lv_img_set_src(ui_bg_4, &hc);
    lv_obj_set_width(ui_bg_4, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_bg_4, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_bg_4, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_bg_4, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_bg_4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
#else
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
#endif
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