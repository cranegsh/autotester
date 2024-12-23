/*
 * app_main.h
 *
 *  Created on: Dec 21, 2024
 *      Author: crane
 */

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include <stdint.h>
#include <time.h>

#include "app_canfd.h"

typedef struct {
	char *dev;
	msg_mode_t mode;		/* CAN message ID */
	uint32_t option;		/* not used */
	int32_t val;			/* CAN message value */
	uint32_t num;			/* loop number */
	uint32_t interval;		/* interval in ms when repeating message submission */
	uint32_t period;		/* total running time in seconds */
	uint32_t function;		/* function selection from command option */
} app_opt_t;

typedef struct {
	msg_mode_t mode;		/* CAN message ID */
	uint32_t option;		/* not used */
	int32_t val;			/* CAN message value */
	uint32_t num;			/* loop number */
	uint32_t interval;		/* interval in ms when repeating message submission */
	timer_t timer_id;		/* ID of timer for counting interval */
	int timer_count;
	BOOL_INT32 timer_mark;
} msg_opt_t;

typedef struct {
	uint32_t project_id;
	int canfd_status;
} sysData_type;

void app_main_displayHelp(const char *app);
void app_main_initData(sysData_type *sdata);
int app_main_processOption(int numOpt, app_opt_t *appOpt, char *strArg, int *num);
void app_main_sendCommand(uint32_t prj_num, sysData_type *sdata, int cmd);
void app_main_initMsg(uint32_t prj_num, int num, app_opt_t *appOpt);
int app_main_checkMsg(int num, int mark);
void app_main_print_canVeh(uint32_t prj_num, int msgNum, int msgCount);
int app_main_remoteControl(uint32_t prj_num, int cmd, sysData_type *sdata);
void app_main_test(void);

#endif /* APP_MAIN_H_ */
