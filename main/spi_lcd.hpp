#pragma once

#include <cstdint>
#include <cstring>
#include <driver/spi_master.h>
#include <freertos/task.h>

#include "spi_transactors.hpp"
#include "colors.hpp"

extern "C" void lcd_spi_pre_transfer_callback(spi_transaction_t *t);

enum disp_spi_send_flag
{
    DISP_SPI_SEND_QUEUED        = 0x00000000,
    DISP_SPI_SEND_POLLING       = 0x00000001,
    DISP_SPI_SEND_SYNCHRONOUS   = 0x00000002,
};

template <class Device>
class SpiLcd
{
private:
    uint16_t* _blackImage;
    spi_device_handle_t _spi;
    SpiPollingTransactor _pollingTransactor;
    SpiPooledTransactor _pooledTransactor;

    static spi_device_handle_t init_spi()
    {
        spi_device_handle_t spi;
        spi_bus_config_t buscfg =
        {
            .mosi_io_num = Device::DeviceDefinitions::mosi_pin,
            .miso_io_num = Device::DeviceDefinitions::miso_pin,
            .sclk_io_num = Device::DeviceDefinitions::clk_pin,
            .max_transfer_sz = Device::DeviceDefinitions::screen_width * 40 * 3
        };
        ESP_ERROR_CHECK(spi_bus_initialize(Device::DeviceDefinitions::spi_host, &buscfg, SPI_DMA_CH_AUTO));

        spi_device_interface_config_t devcfg =
        {
            .mode = 0,
            .clock_speed_hz = 10 * 1000 * 1000,
            .spics_io_num = Device::DeviceDefinitions::cs_pin,
            .queue_size = 50,
            .pre_cb=lcd_spi_pre_transfer_callback
        };
        ESP_ERROR_CHECK(spi_bus_add_device(Device::DeviceDefinitions::spi_host, &devcfg, &spi));
        return spi;
    }

    void reset()
    {
        gpio_set_level(Device::DeviceDefinitions::rst_pin, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(Device::DeviceDefinitions::rst_pin, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(Device::DeviceDefinitions::rst_pin, 1);
        gpio_set_level(Device::DeviceDefinitions::cs_pin, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
public:
    SpiLcd()
    : _spi(init_spi())
    , _pollingTransactor(_spi)
    , _pooledTransactor(_spi)
    {
        _blackImage = reinterpret_cast<uint16_t*>(heap_caps_malloc(Device::DeviceDefinitions::screen_width * Device::DeviceDefinitions::screen_height * sizeof(uint16_t), MALLOC_CAP_DMA));
        reset();
        send_commands(Device::DeviceDefinitions::lcd_init_cmds, DISP_SPI_SEND_POLLING);
        gpio_set_level(Device::DeviceDefinitions::bl_pin, 1);
        clear(Colors::BLACK, DISP_SPI_SEND_POLLING);
    }

    void clear(Colors color, disp_spi_send_flag display_flag)
    {
        uint16_t color16 = static_cast<uint16_t>(color);
        color16 = ((color16<<8)&0xff00)|(color16>>8);

        for (int j = 0; j < Device::DeviceDefinitions::screen_width * Device::DeviceDefinitions::screen_height; j++)
        {
            _blackImage[j] = color16;
        }

        display(Device::DeviceDefinitions::screen_width, Device::DeviceDefinitions::screen_height, _blackImage, display_flag);
    }

    void display(uint16_t width, uint16_t height, const uint16_t* image, disp_spi_send_flag display_flag)
    {
        set_window(0, 0, width, height, display_flag);
        if (display_flag & DISP_SPI_SEND_POLLING)
        {
            for(int j = 0; j < height; j++)
            {
                _pollingTransactor.send_data((uint8_t *)(&image[j * width]), width * sizeof(uint16_t));
            }
        }
        else
        {
            for(int j = 0; j < height; j++)
            {
                _pooledTransactor.send_data((uint8_t *)(&image[j * width]), width * sizeof(uint16_t));
            }
        }
    }

    void set_window(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, disp_spi_send_flag display_flag)
    {
        LCDCommand commands[] = 
        {
            {0x2A, { uint8_t((startx >> 8) & 0xFF), uint8_t(startx & 0xFF), uint8_t((endx >> 8) & 0xFF), uint8_t(endx & 0xFF) }, 4},
            {0x2B, { uint8_t((starty >> 8) & 0xFF), uint8_t(starty & 0xFF), uint8_t((endy >> 8) & 0xFF), uint8_t(endy & 0xFF) }, 4},
            {0x2C, {0}, 0},
            {0, {0}, 0xff}
        };
        send_commands(commands, display_flag);
    }

    void send_commands(const LCDCommand* commands, disp_spi_send_flag display_flag)
    {
        if (display_flag & DISP_SPI_SEND_POLLING)
        {
            _pooledTransactor.wait_for_pending_transactions();
            send_commands_with_transactor(commands, _pollingTransactor);
        }
        else
        {
            send_commands_with_transactor(commands, _pooledTransactor);
        }
    }

    void send_data(const uint8_t *data, int len, disp_spi_send_flag display_flag)
    {
        if (display_flag & DISP_SPI_SEND_POLLING)
        {
            _pooledTransactor.wait_for_pending_transactions();
            _pollingTransactor.send_data(data, len);
        }
        else
        {
            _pooledTransactor.send_data(data, len);
        }
    }

private:

    template <class Transactor>
    void send_commands_with_transactor(const LCDCommand* commands, Transactor &transactor)
    {
        int cmdIdx = 0;
        while (commands[cmdIdx].databytes!=0xff)
        {
            transactor.send_command(commands[cmdIdx].cmd);
            transactor.send_data(commands[cmdIdx].data, commands[cmdIdx].databytes & 0x1F);
            if (commands[cmdIdx].databytes & 0x80)
            {
                vTaskDelay(120 / portTICK_PERIOD_MS);
            }
            cmdIdx++;
        }
    }
};

