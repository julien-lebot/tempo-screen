#pragma once

#include <functional>
#include <string>
#include <esp_wifi.h>

class WifiClient
{
public:
    typedef std::function<void(wifi_config_t&)> config_fn;
public:
    WifiClient();
    void scan();
    void connect(std::string ssid, std::string password);
};