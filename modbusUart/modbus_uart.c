/*
 * modbus_uart.c
 *
 *  Created on: 7 sep. 2020
 *      Author: Emiliano Augusto Gonzalez (comercial@hiperion.com.ar)
 */
#include <strings.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/hw_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "modbus_uart.h"
#include "MDS_RTU_Serial.h"
#include "MDM_RTU_Serial.h"
static const char *TAG = "modbus_uart";

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define EX_UART_NUM UART_NUM_0
static QueueHandle_t uart0_queue;
static xTaskHandle modbus_uart_task_handle = NULL;

static void modbus_uart_event_task(void *pvParameters) {
    uart_event_t event;
    uint8_t *dtmp = (uint8_t*) malloc(RD_BUF_SIZE);

    for (;;) {
        if (xQueueReceive(uart0_queue, (void*) &event, (portTickType) portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);

            switch (event.type) {
            case UART_DATA:
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
#if !MODBUS_MASTER
                MDSSerialRecvByte(dtmp[0]);
#else
                MDMSerialRecvByte(dtmp[0]);
#endif
                break;
            case UART_FIFO_OVF:
                ESP_LOGD(TAG, "hw fifo overflow");
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGD(TAG, "ring buffer full");
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            case UART_PARITY_ERR:
                ESP_LOGD(TAG, "uart parity error");
                xQueueReset(uart0_queue);
                break;
            case UART_FRAME_ERR:
                ESP_LOGD(TAG, "uart frame error");
                xQueueReset(uart0_queue);
                break;
            default:
                ESP_LOGD(TAG, "uart event type: %d", event.type);
                xQueueReset(uart0_queue);
                break;
            }
        }
    }

    free(dtmp);
    hw_timer_disarm();
    hw_timer_deinit();
    vTaskDelete(NULL);
}

void TimeHandler100US(void *arg) {
#if !MODBUS_MASTER
    MDSTimeHandler100US();
#else
    MDMTimeHandler100US();
#endif
}

void modbus_uart_init(uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity) {
    unsigned char ucParity = UART_PARITY_DISABLE;
      unsigned char ucData = UART_DATA_8_BITS;

    switch (parity) {
        case MB_PAR_NONE:
            ucParity = UART_PARITY_DISABLE;
            break;
        case MB_PAR_ODD:
            ucParity = UART_PARITY_ODD;
            break;
        case MB_PAR_EVEN:
            ucParity = UART_PARITY_EVEN;
            break;
        }

        switch (dataBits) {
        case 5:
            ucData = UART_DATA_5_BITS;
            break;
        case 6:
            ucData = UART_DATA_6_BITS;
            break;
        case 7:
            ucData = UART_DATA_7_BITS;
            break;
        case 8:
            ucData = UART_DATA_8_BITS;
            break;
        default:
            ucData = UART_DATA_8_BITS;
            break;
        }

    uart_config_t uart_config = {
            .baud_rate = baud,
            .data_bits = ucData,
               .parity = ucParity,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart0_queue, 0);

    xTaskCreate(
            modbus_uart_event_task,
            "modbus_uart_event_task",
            2048,
            NULL,
            12,
            &modbus_uart_task_handle
            );

    hw_timer_init(TimeHandler100US, NULL);
    hw_timer_alarm_us(100, true);
    hw_timer_enable(true);
}

void modbus_uart_send_bytes_by_isr(uint8_t *bytes, uint16_t num) {
    uart_write_bytes(EX_UART_NUM, (const char*) bytes, num);
}

void modbus_uart_send_bytes(uint8_t *bytes, uint16_t num) {
    uart_write_bytes(EX_UART_NUM, (const char*) bytes, num);
}
