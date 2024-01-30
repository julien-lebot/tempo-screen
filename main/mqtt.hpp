#pragma once

#include <functional>
#include <map>
#include <string>
#include <mqtt_client.h>
#include <esp_err.h>

class MQTTClient;

extern "C"
void mqtt_event_handler(MQTTClient *mqttClient, esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data);

class MQTTClient
{
public:
    typedef std::function<void(esp_mqtt_client_config_t&)> config_fn;
    typedef std::function<void(std::string_view)> subscription_fn;
private:
    esp_mqtt_client_handle_t _mqtt_client_handle;
    std::map<std::string, subscription_fn, std::less<>> _subscriptions;

    void handle_event(esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data);
    friend void mqtt_event_handler(MQTTClient *mqttClient, esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data);
public:
    MQTTClient();
    esp_err_t connect(config_fn config);
    void subscribe(std::string topic, subscription_fn subscription);
};
