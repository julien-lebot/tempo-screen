#pragma once

#include <cstdint>

struct lcd_init_cmd_t
{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
};
