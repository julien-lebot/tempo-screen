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

static lv_color_t white = lv_color_hex(0xFFFFFF);
static lv_color_t black = lv_color_hex(0x000000);
static lv_color_t light_gray = lv_color_hex(0xc3c3c3);
static lv_color_t dark_gray = lv_color_hex(0x191919);

static int phase_color_order[2][3] =
{
    // ligther
    {3, 2, 1},
    // darker
    {7, 6, 5},
};

LV_IMAGE_DECLARE(Wifi);

LV_FONT_DECLARE(CourierPrimeRegular14);
LV_FONT_DECLARE(CourierPrimeBold14);
LV_FONT_DECLARE(CourierPrimeBold40);

bool is_background_dark(lv_color_t const &background_color)
{
    if ((background_color.red * 0.299 + background_color.green * 0.587 + background_color.blue * 0.114) > 186)
    {
        return false;
    }
    return true;
}

void AppUi::init()
{
    lv_obj_set_style_bg_color(lv_screen_active(), black, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(lv_screen_active(), white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(lv_screen_active(), 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(lv_screen_active(), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(lv_screen_active(), LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_color_t font_color = light_gray;

    lv_anim_init(&ui_Background_anim);
    lv_anim_set_var(&ui_Background_anim, lv_screen_active());
    lv_anim_set_exec_cb(&ui_Background_anim, lv_anim_exec_xcb_t(animate_gradient_stop));
    lv_anim_set_duration(&ui_Background_anim, 450);
    lv_anim_set_path_cb(&ui_Background_anim, lv_anim_path_ease_in_out);

    uint8_t size = 235;

    int32_t x = 92;
    int32_t y = 107;


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
        lv_arc_set_bg_angles(_phases[phase_index].arc, 110, 250);
        lv_arc_set_mode(_phases[phase_index].arc, LV_ARC_MODE_SYMMETRICAL);
        lv_obj_set_style_arc_color(_phases[phase_index].arc, dark_gray, LV_PART_MAIN);
        lv_obj_set_style_arc_color(_phases[phase_index].arc, phase_colors[phase_index], LV_PART_INDICATOR);

        lv_obj_set_style_arc_width(_phases[phase_index].arc, 15, LV_PART_MAIN);
        lv_obj_set_style_arc_width(_phases[phase_index].arc, 15, LV_PART_INDICATOR);

        lv_arc_set_range(_phases[phase_index].arc, -8000, 8000);

        lv_anim_init(&_phases[phase_index].arc_anim);
        lv_anim_set_var(&_phases[phase_index].arc_anim, _phases[phase_index].arc);
        lv_anim_set_exec_cb(&_phases[phase_index].arc_anim, lv_anim_exec_xcb_t(lv_arc_set_value));
        lv_anim_set_duration(&_phases[phase_index].arc_anim, 450);
        lv_anim_set_path_cb(&_phases[phase_index].arc_anim, lv_anim_path_ease_out);

        _phases[phase_index].label = lv_label_create(lv_screen_active());
        lv_obj_set_width(_phases[phase_index].label, LV_SIZE_CONTENT);
        lv_obj_set_height(_phases[phase_index].label, LV_SIZE_CONTENT);
        lv_obj_set_x(_phases[phase_index].label, x);
        lv_obj_set_y(_phases[phase_index].label, y);
        lv_obj_set_align(_phases[phase_index].label, LV_ALIGN_LEFT_MID);
        lv_label_set_text(_phases[phase_index].label, "0W");
        lv_obj_set_style_text_color(_phases[phase_index].label, font_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(_phases[phase_index].label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(_phases[phase_index].label, &CourierPrimeBold14, LV_PART_MAIN | LV_STATE_DEFAULT);

        size -= 40;
        x += 7;
        y -= 20;
    }
    
    ui_next_color = lv_arc_create(lv_screen_active());
    lv_obj_set_width(ui_next_color, 235);
    lv_obj_set_height(ui_next_color, 235);
    lv_obj_set_align(ui_next_color, LV_ALIGN_CENTER);
    lv_arc_set_value(ui_next_color, 100);
    lv_arc_set_bg_angles(ui_next_color, 340, 380);
    lv_obj_remove_style(ui_next_color, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(ui_next_color, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ui_next_color, black, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ui_next_color, dark_gray, LV_PART_MAIN);
    lv_arc_set_range(ui_next_color, 0, 100);

    ui_Remaining_time = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Remaining_time, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Remaining_time, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Remaining_time, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Remaining_time, "-00:00");
    lv_obj_set_x(ui_Remaining_time, 12);
    lv_obj_set_y(ui_Remaining_time, 9);
    lv_obj_set_style_text_color(ui_Remaining_time, white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Remaining_time, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Remaining_time, &CourierPrimeBold40, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Remaining_time_seconds = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Remaining_time_seconds, LV_SIZE_CONTENT); 
    lv_obj_set_height(ui_Remaining_time_seconds, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Remaining_time_seconds, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Remaining_time_seconds, "00");
    lv_obj_set_x(ui_Remaining_time_seconds, 92);
    lv_obj_set_y(ui_Remaining_time_seconds, 16);
    lv_obj_set_style_text_color(ui_Remaining_time_seconds, white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Remaining_time_seconds, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Remaining_time_seconds, &CourierPrimeRegular14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Tarif = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_Tarif, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Tarif, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_Tarif, 21);
    lv_obj_set_y(ui_Tarif, -24);
    lv_obj_set_align(ui_Tarif, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Tarif, "--");
    lv_obj_set_style_text_color(ui_Tarif, white, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Tarif, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Tarif, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Wifi = lv_img_create(lv_screen_active());
    lv_img_set_src(ui_Wifi, &Wifi);
    lv_obj_set_width(ui_Wifi, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Wifi, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_Wifi, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_Wifi, LV_OBJ_FLAG_ADV_HITTEST); // Add LV_OBJ_FLAG_HIDDEN to hide
    lv_obj_clear_flag(ui_Wifi, LV_OBJ_FLAG_SCROLLABLE);

    xTaskCreatePinnedToCore(TaskFunction_t(&run_ui), "ui_task", 4096, (void*)&_ui_mutex, 100, nullptr, 1);
}

void AppUi::set_phase_color(uint8_t phase_id, lv_color_t const &phase_color)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
}

void AppUi::set_phase_power(uint8_t phase_id, int16_t power)
{
    std::lock_guard<std::mutex> guard_ui(_ui_mutex);
    lv_label_set_text_fmt(_phases[phase_id].label, "%dW", power);
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

    lv_color_t font_color = dark_gray;

    lv_color_t palette[9];
    palette[4] = bg_color;
    create_hue_shift(&palette[0], 0, 10, 12);
    const int* selected_color_order = phase_color_order[0];

    if (is_background_dark(bg_color))
    {
        font_color = light_gray;
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
