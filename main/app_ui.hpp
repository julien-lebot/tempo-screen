#pragma once

#include <array>
#include <chrono>
#include <string>
#include <mutex>
#include <lvgl.h>
#include <esp_timer.h>

#include "date/date.h"

class AppUi
{
public:
    typedef std::chrono::duration<long long> duration_t;
private:
    struct Phase
    {
        lv_obj_t *arc;
        lv_obj_t *label;
        lv_anim_t arc_anim;
    };
    std::array<Phase, 3> _phases;
    lv_anim_t ui_Background_anim;
    lv_obj_t * ui_Remaining_time;
    lv_obj_t * ui_Remaining_time_seconds;
    lv_obj_t * ui_next_color;
    lv_obj_t * ui_Tarif;
    lv_obj_t * ui_Wifi;
    esp_timer_handle_t _lvgl_tick_timer;
    std::mutex _ui_mutex;
public:
    AppUi();
    void init();
    void start();
    void set_phase_color(uint8_t phase_id, lv_color_t const &phase_color);
    void set_phase_power(uint8_t phase_id, uint16_t power);
    void set_remaining_duration(duration_t duration);
    void set_background_color(lv_color_t const &bg_color);
    void set_next_color(lv_color_t const &color);
    void set_tarif_name(std::string const &name);
    void set_gradient_stop(int32_t value);
};