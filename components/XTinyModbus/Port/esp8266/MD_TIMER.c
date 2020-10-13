/*
 * MD_TIMER.c
 *
 *  Created on: 7 oct. 2020
 *      Author: egonzalez
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/hw_timer.h"

void md_timer_init(uint32_t uS, void (* _TimeHandler)(void *arg), bool reload){
    hw_timer_init(_TimeHandler, NULL);
    hw_timer_alarm_us(uS, reload);
}

