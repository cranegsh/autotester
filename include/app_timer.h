/*
 * app_timer.h
 *
 *  Created on: Dec 21, 2024
 *      Author: crane
 */

#ifndef APP_TIMER_H_
#define APP_TIMER_H_

#include "app_canfd.h"

void start_timer(msg_opt_t (*appid)[CAN_VEH_MSG_NUM], int index, int total);
void start_singletimer(timer_t *timerid, int interval, void (*handler)());
void stop_timer(timer_t *timerid);
void delete_timer(timer_t *timerid);


#endif /* APP_TIMER_H_ */
