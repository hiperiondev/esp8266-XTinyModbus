/*
 * MD_UART.c
 *
 *  Created on: 7 oct. 2020
 *      Author: egonzalez
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp8266/uart_register.h"
#include "esp8266/uart_struct.h"
#include "MD_UART.h"

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

// UART interrupt subroutine
static void IRAM_ATTR uart_intr_handle(void *arg) {
    _SerialRecvByte = arg;
    uint16_t rx_fifo_len;

    rx_fifo_len = uart0.status.rxfifo_cnt; // read number of bytes in UART buffer

    while (rx_fifo_len) {
        (* _SerialRecvByte)(uart0.fifo.rw_byte);
        rx_fifo_len--;
    }

    // clear UART interrupt status
    uart_clear_intr_status(EX_UART_NUM, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
}

static void uart_interrupt_init(int baud_rate, uart_word_length_t data_bits, uart_parity_t parity, uart_stop_bits_t stop_bits,
        uart_hw_flowcontrol_t flow_ctrl, void (* _SerialRecvByte)(uint8_t byte)) {

    uart_config_t uart_config = {
            .baud_rate = baud_rate,
            .data_bits = data_bits,
            .parity    = parity,
            .stop_bits = stop_bits,
            .flow_ctrl = flow_ctrl
    };

    ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));;
    ESP_ERROR_CHECK(uart_isr_register(EX_UART_NUM, uart_intr_handle, _SerialRecvByte));
    ESP_ERROR_CHECK(uart_enable_rx_intr(EX_UART_NUM));
}

void uart_init(uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity, void (* _SerialRecvByte)(uint8_t byte)) {
    switch (dataBits) {
    case 5:
        dataBits = UART_DATA_5_BITS;
        break;
    case 6:
        dataBits = UART_DATA_6_BITS;
        break;
    case 7:
        dataBits = UART_DATA_7_BITS;
        break;
    case 8:
        dataBits = UART_DATA_8_BITS;
        break;
    }

    switch (stopBit) {
    case 1:
        stopBit = UART_STOP_BITS_1;
        break;
    case 2:
        stopBit = UART_STOP_BITS_2;
        break;
    }

    switch (parity) {
    case 0:
        parity = UART_PARITY_DISABLE;
        break;
    case 1:
        parity = UART_PARITY_EVEN;
        break;
    case 2:
        parity = UART_PARITY_ODD;
        break;
    }
    uart_interrupt_init(baud, dataBits, parity, stopBit, UART_HW_FLOWCTRL_DISABLE, _SerialRecvByte);
}
