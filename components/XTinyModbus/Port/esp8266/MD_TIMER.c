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

void md_timer_init(void (* _MDTimeHandler100US)(void *arg)){
    hw_timer_init(_MDTimeHandler100US, NULL);
    hw_timer_alarm_us(100, true);
}

