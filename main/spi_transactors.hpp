#pragma once

#include <cstdint>
#include <cstring>
#include <freertos/queue.h>
#include <driver/spi_master.h>

class SpiPollingTransactor
{
private:
    spi_device_handle_t _spi;
public:
    SpiPollingTransactor(spi_device_handle_t spi)
    : _spi(spi)
    {
    }

    void send_command(const uint8_t cmd)
    {
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));         //Zero out the transaction
        t.length = 8;                     //Command is 8 bits
        t.tx_buffer = &cmd;               //The data is the cmd itself
        t.user = (void*)0;                //D/C needs to be set to 0
        ESP_ERROR_CHECK(spi_device_polling_transmit(_spi, &t));
    }

    void send_data(const uint8_t *data, int len)
    {
        spi_transaction_t t;
        if (len == 0) 
        {
            return;                     //no need to send anything
        }
        memset(&t, 0, sizeof(t));       //Zero out the transaction
        t.length = len * 8;             //Len is in bytes, transaction length is in bits.
        t.tx_buffer = data;             //Data
        t.user = (void*)1;              //D/C needs to be set to 1
        ESP_ERROR_CHECK(spi_device_polling_transmit(_spi, &t));
    }
};

class SpiPooledTransactor
{
private:
    spi_device_handle_t _spi;
    QueueHandle_t _transaction_pool;

    const int max_queued_items = 50;

    void ensure_free_transactions(int min_items_availablility)
    {
        spi_transaction_t *result;
        while (uxQueueMessagesWaiting(_transaction_pool) < min_items_availablility)
        {
            if (spi_device_get_trans_result(_spi, &result, portMAX_DELAY) == ESP_OK)
            {
                xQueueSend(_transaction_pool, &result, portMAX_DELAY);
            }
        }
    }
public:
    SpiPooledTransactor(spi_device_handle_t spi)
    : _spi(spi)
    {
        _transaction_pool = xQueueCreate(max_queued_items, sizeof(spi_transaction_t*));
        assert(_transaction_pool != nullptr);
        for (int i = 0; i < max_queued_items; ++i)
        {
            spi_transaction_t* transaction = (spi_transaction_t*)heap_caps_malloc(sizeof(spi_transaction_t), MALLOC_CAP_DMA);
            assert(transaction != nullptr);
            memset(transaction, 0, sizeof(spi_transaction_t));
            xQueueSend(_transaction_pool, &transaction, portMAX_DELAY);
        }
    }

    void wait_for_pending_transactions()
    {
        ensure_free_transactions(max_queued_items);
    }

    void send_command(const uint8_t cmd)
    {
        if (uxQueueMessagesWaiting(_transaction_pool) == 0)
        {
            ensure_free_transactions(max_queued_items / 10);
        }

        spi_transaction_t *transaction = nullptr;
		xQueueReceive(_transaction_pool, &transaction, portMAX_DELAY);
        memset(transaction, 0, sizeof(spi_transaction_t));

        transaction->flags = SPI_TRANS_USE_TXDATA;
        transaction->tx_data[0] = cmd;
        transaction->length = 8;
        transaction->user = (void*)0;

        if (spi_device_queue_trans(_spi, (spi_transaction_t *) transaction, portMAX_DELAY) != ESP_OK)
        {
			xQueueSend(_transaction_pool, &transaction, portMAX_DELAY);
        }
    }

    void send_data(const uint8_t *data, int len)
    {
        if (len == 0)
        {
            return;
        }

        if (uxQueueMessagesWaiting(_transaction_pool) == 0)
        {
            ensure_free_transactions(max_queued_items / 10);
        }

        spi_transaction_t t = {0};
        if (len <= 4)
        {
            t.flags = SPI_TRANS_USE_TXDATA;
            memcpy(t.tx_data, data, len);
        }
        else
        {
            t.tx_buffer = data;
        }
        t.length = len * 8;
        t.user = (void*)1;

        spi_transaction_t *transaction = nullptr;
		xQueueReceive(_transaction_pool, &transaction, portMAX_DELAY);
        memcpy(transaction, &t, sizeof(t));

        if (spi_device_queue_trans(_spi, (spi_transaction_t *) transaction, portMAX_DELAY) != ESP_OK)
        {
			xQueueSend(_transaction_pool, &transaction, portMAX_DELAY);
        }
    }
};