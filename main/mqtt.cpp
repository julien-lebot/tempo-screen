#include <esp_netif.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>

#include "mqtt.hpp"

static const char *TAG = "mqtt";

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_event_handler(MQTTClient *mqttClient, esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data)
{
    mqttClient->handle_event(base, event_id, event_data);
}

MQTTClient::MQTTClient()
: _mqtt_client_handle(nullptr)
{

}

void MQTTClient::handle_event(esp_event_base_t base, int32_t event_id, esp_mqtt_event_handle_t event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_BEFORE_CONNECT:
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            for (auto sub : _subscriptions)
            {
                esp_mqtt_client_subscribe(_mqtt_client_handle, sub.first.c_str(), 0);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event_data->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event_data->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event_data->msg_id);
            break;
        case MQTT_EVENT_DATA:
            {
                ESP_LOGD(TAG, "MQTT_EVENT_DATA");
                ESP_LOGD(TAG, "TOPIC=%.*s\r\n", event_data->topic_len, event_data->topic);
                ESP_LOGD(TAG, "DATA=%.*s\r\n", event_data->data_len, event_data->data);

                auto topic = std::string_view(event_data->topic, event_data->topic_len);
                auto sub = _subscriptions.find(topic);
                if (sub != _subscriptions.end())
                {
                    sub->second(std::string_view(event_data->data, event_data->data_len));
                }
            }
            break;
        case MQTT_EVENT_DELETED:
            break;
        case MQTT_USER_EVENT:
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event_data->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event_data->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event_data->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event_data->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event_data->error_handle->esp_transport_sock_errno));
            }
            break;
        case MQTT_EVENT_ANY:
            break;
    }
}

void MQTTClient::subscribe(std::string topic, subscription_fn subscription)
{
    _subscriptions[topic] = subscription;
}

esp_err_t MQTTClient::connect(MQTTClient::config_fn config)
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    config(mqtt_cfg);
    _mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);

    esp_err_t err = esp_mqtt_client_register_event(_mqtt_client_handle, esp_mqtt_event_id_t(ESP_EVENT_ANY_ID), esp_event_handler_t(mqtt_event_handler), this);
    if (err != ESP_OK)
    {
        return err;
    }
    return esp_mqtt_client_start(_mqtt_client_handle);
}