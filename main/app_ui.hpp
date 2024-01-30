#pragma once

#include <array>
#include <mutex>
#include <lvgl.h>
#include <esp_timer.h>

class AppUi
{
private:
    struct Phase
    {
        lv_obj_t *arc;
        lv_obj_t *label;
        lv_anim_t arc_anim;
    };
    std::array<Phase, 3> _phases;
    lv_obj_t * ui_Remaining_time;
    lv_obj_t * ui_next_color;
    lv_obj_t * ui_Tarif;
    esp_timer_handle_t _lvgl_tick_timer;
    std::mutex _ui_mutex;
public:
    AppUi();
    void init();
    void start();
    void set_phase_color(uint8_t phase_id, lv_color_t phase_color);
    void set_phase_power(uint8_t phase_id, float power);
};