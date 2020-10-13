/*
 * MD_TIMER.h
 *
 *  Created on: 7 oct. 2020
 *      Author: egonzalez
 */

#ifndef _MD_TIMER_H_
#define _MD_TIMER_H_

void md_timer_init(uint32_t uS, void (* _TimeHandler)(void *arg), bool reload);

#endif
