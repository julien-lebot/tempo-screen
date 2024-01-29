#pragma once

#include <driver/gpio.h>
#include <esp_err.h>

template <class Definitions>
class Device
{
public:
    typedef Definitions DeviceDefinitions;
private:
    void init_gpio()
    {
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = ((1ULL<<Definitions::rst_pin) | (1ULL<<Definitions::dc_pin) | (1ULL<<Definitions::bl_pin));
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
    }
public:
    Device()
    {
        init_gpio();
    }
};
