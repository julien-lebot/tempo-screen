#include <cstdio>
#include "app_ui.hpp"
#include "color_manip.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"

void run_ui(std::mutex *ui_mutex)
{
    while (1)
    {
        std::lock_guard<std::mutex> guard_ui(*ui_mutex);
        lv_task_handler();
        lv_timer_handler();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

AppUi::AppUi()
{
    
}

void animate_gradient_stop(lv_obj_t *obj, int32_t value)
{
    lv_obj_set_style_bg_main_stop(obj, value, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_color_t light_font_color = lv_color_hex(0xc3c3c3);
static lv_color_t dark_font_color = lv_color_hex(0x191919);

static int phase_color_order[2][3] =
{
    // ligther
    {3, 2, 1},
    // darker
    {7, 6, 5},
};

LV_IMAGE_DECLARE(Wifi);

bool is_background_dark(lv_color_t const &background_color)
{
    if ((background_color.red * 0.299 + background_color.green * 0.587 + background_color.blue * 0.114) > 186)
    {
        return true;
    }
    return false;
}

void AppUi::init()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(lv_screen_active(), 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(lv_screen_active(), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(lv_screen_active(), LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_color_t font_color = dark_font_color;

    lv_anim_init(&ui_Background_anim);
    lv_anim_set_var(&ui_Background_anim, lv_screen_active());
    lv_anim_set_exec_cb(&ui_Background_anim, lv_anim_exec_xcb_t(animate_gradient_stop));
    lv_anim_set_duration(&ui_Background_anim, 450);
    lv_anim_set_path_cb(&ui_Background_anim, lv_anim_path_ease_in_out);

    uint8_t size = 230;
    int8_t rotation = -30;

    // Some default colors
    lv_color_t phase_colors[3] =
    {
        lv_color_hex(0xFF6A48),
        lv_color_hex(0xEF5350),
        lv_color_hex(0xE57373)
    };
    for (int phase_index = 0; phase_index < 3; ++phase_index)
    {
        _phases[phase_index].arc = lv_arc_create(lv_screen_active());
        lv_obj_set_width(_phases[phase_index].arc, size);
        lv_obj_set_height(_phases[phase_index].arc, size);
        
        lv_obj_set_align(_phases[phase_index].arc, LV_ALIGN_CENTER);
        lv_obj_remove_style(_phases[phase_index].arc, nullptr, LV_PART_KNOB);
        lv_obj_clear_flag(_phases[phase_index].arc, LV_OBJ_FLAG_CLICKABLE);
        lv_arc_set_value(_phases[phase_index].arc, 64);
        lv_arc_set_bg_angles(_phases[phase_index].arc, 145, 305);
        lv_arc_set_rotation(_phases[phase_index].arc, rotation);
        lv_obj_set_style_arc_color(_phases[phase_index].arc, lv_color_hex(0x191919), LV_PART_MAIN);
        lv_obj_set_style_arc_color(_phases[phase_index].arc, phase_colors[phase_index], LV_PART_INDICATOR);

        lv_obj_set_style_arc_width(_phases[phase_index].arc, 15, LV_PART_MAIN);
        lv_obj_set_style_arc_width(_phases[phase_index].arc, 15, LV_PART_INDICATOR);

        lv_arc_set_range(_phases[phase_index].arc, 0, 12000);

        lv_anim_init(&_phases[phase_index].arc_anim);
        lv_anim_set_var(&_phases[phase_index].arc_anim, _phases[phase_index].arc);
        lv_anim_set_exec_cb(&_phases[phase_index].arc_anim, lv_anim_exec_xcb_t(lv_arc_set_value));
        lv_anim_set_duration(&_phases[phase_index].arc_anim, 450);
        lv_anim_set_path_cb(&_phases[phase_index].arc_anim, lv_anim_path_ease_out);

        size -= 45;
    }
    
    ui_next_color = lv_arc_create(lv_screen_active());
    lv_obj_set_width(ui_next_color, 235);
    lv_obj_set_height(ui_next_color, 235);
    lv_obj_set_align(ui_next_color, LV_ALIGN_CENTER);
    lv_arc_set_value(ui_next_color, 100);
    lv_arc_set_bg_angles(ui_next_color, 0, 45);
    lv_arc_set_mode(ui_next_color, LV_ARC_MODE_REVERSE);
    lv_arc_set_rotation(ui_next_color, 337);
    lv_obj_remove_style(ui_next_color, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(ui_next_color, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ui_next_color, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ui_next_color, lv_color_hex(0x191919), LV_PART_MAIN);
    lv_arc_set_range(ui_next_color, 0, 100);

    _phases[0].label = lv_label_create(lv_screen_active());
    lv_obj_set_width(_phases[0].label, LV_SIZE_CONTENT);
    lv_obj_set_height(_phases[0].label, LV_SIZE_CONTENT);
    lv_obj_set_x(_phases[0].label, -3);
    lv_obj_set_y(_phases[0].label, 100);
    lv_obj_set_align(_phases[0].label, LV_ALIGN_CENTER);
    lv_label_set_text(_phases[0].label, "0W");
    lv_obj_set_style_text_color(_phases[0].label, font_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(_phases[0].label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    _phases[1].label = lv_label_create(lv_screen_active());
    lv_obj_set_width(_phases[1].label, LV_SIZE_CONTENT);
    lv_obj_set_height(_phases[1].label, LV_SIZE_CONTENT);
    lv_obj_set_x(_phases[1].label, 5);
    lv_obj_set_y(_phases[1].label, 82);
    lv_obj_set_align(_phases[1].label, LV_ALIGN_CENTER);
    lv_label_set_text(_phases[1].label, "0W");
    lv_obj_set_style_text_color(_phases[1].label, font_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(_phases[1].label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    _phases[2].label = lv_label_create(lv_screen_active());
    lv_obj_set_width(_phases[2].label, LV_SIZE_CONTENT);
    lv_obj_set_height(_phases[2].label, LV_SIZE_CONTENT);
    lv_obj_set_x(_phases[2].label, 13);
    lv_obj_set_y(_phases[2].label, 64);
    lv_obj_set_align(_phases[2].label, LV_ALIGN_CENTER);
    lv_label_set_text(_phases[2].label, "0W");
    lv_obj_set_style_text_color(_phases[2].label, font_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(_phases[2].label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Remaining_time = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Remaining_time, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Remaining_time, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Remaining_time, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Remaining_time, "-00:00");
    lv_obj_set_x(ui_Remaining_time, 10);
    lv_obj_set_y(ui_Remaining_time, 14);
    lv_obj_set_style_text_color(ui_Remaining_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Remaining_time, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Remaining_time, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Remaining_time_seconds = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Remaining_time_seconds, LV_SIZE_CONTENT); 
    lv_obj_set_height(ui_Remaining_time_seconds, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Remaining_time_seconds, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Remaining_time_seconds, "00");
    lv_obj_set_x(ui_Remaining_time_seconds, 85);
    lv_obj_set_y(ui_Remaining_time_seconds, 20);
    lv_obj_set_style_text_color(ui_Remaining_time_seconds, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Remaining_time_seconds, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Remaining_time_seconds, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Tarif = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Tarif, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Tarif, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_Tarif, 24);
    lv_obj_set_y(ui_Tarif, -24);
    lv_obj_set_align(ui_Tarif, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Tarif, "--");
    lv_obj_set_style_text_color(ui_Tarif, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Tarif, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Tarif, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Wifi = lv_img_create(lv_screen_active());
    lv_img_set_src(ui_Wifi, &Wifi);
    lv_obj_set_width(ui_Wifi, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Wifi, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_Wifi, 38);
    lv_obj_set_y(ui_Wifi, -90);
    lv_obj_set_align(ui_Wifi, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Wifi, LV_OBJ_FLAG_ADV_HITTEST); // Add LV_OBJ_FLAG_HIDDEN to hide
    lv_obj_clear_flag(ui_Wifi, LV_OBJ_FLAG_SCROLLABLE);

    xTaskCreatePinnedToCore(TaskFunction_t(&run_ui), "ui_task", 4096, (void*)&_ui_mutex, 100, nullptr, 1);
}

void AppUi::set_phase_color(uint8_t phase_id, lv_color_t const &phase_color)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
}

void AppUi::set_phase_power(uint8_t phase_id, uint16_t power)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
    lv_label_set_text_fmt(_phases[phase_id].label, "%d W", power);
    lv_anim_set_values(&_phases[phase_id].arc_anim, lv_arc_get_value(_phases[phase_id].arc), power);
    lv_anim_start(&_phases[phase_id].arc_anim);
}

void AppUi::set_remaining_duration(AppUi::duration_t duration)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - minutes);

    lv_label_set_text_fmt(ui_Remaining_time, "-%02d:%02d", (int)hours.count(), (int)minutes.count());
    lv_label_set_text_fmt(ui_Remaining_time_seconds, "%02d", (int)seconds.count());
}

void AppUi::set_background_color(lv_color_t const &bg_color)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
    lv_obj_set_style_bg_grad_color(lv_screen_active(), bg_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_color_t &font_color = light_font_color;

    lv_color_t palette[9];
    palette[4] = bg_color;
    create_hue_shift(&palette[0], 0, 10, 12);
    const int* selected_color_order = phase_color_order[0];

    if (is_background_dark(bg_color))
    {
        font_color = dark_font_color;
        selected_color_order = phase_color_order[1];
    }

    for (int i = 0; i < 3; ++i)
    {
        lv_obj_set_style_text_color(_phases[i].label, font_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_color(_phases[i].arc, palette[selected_color_order[i]], LV_PART_INDICATOR);
    }   
}

void AppUi::set_next_color(lv_color_t const &color)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
    lv_obj_set_style_arc_color(ui_next_color, color, LV_PART_INDICATOR);
}

void AppUi::set_tarif_name(std::string const &name)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
    lv_label_set_text(ui_Tarif, name.c_str());
}

void AppUi::set_gradient_stop(int32_t value)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);

    lv_anim_set_values(&ui_Background_anim, lv_obj_get_style_bg_main_stop(lv_screen_active(), LV_PART_MAIN | LV_STATE_DEFAULT), value);
    lv_anim_start(&ui_Background_anim);
}

void set_signal_level()
{

}