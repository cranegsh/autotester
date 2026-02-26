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
	int32_t val;			/* CAN message parameter value, TODO: upgrade to float type */
	uint32_t num;			/* loop number */
	uint32_t interval;		/* interval in ms when repeating message submission */
	uint32_t period;		/* total running time in seconds */
	uint32_t function;		/* function selection from command option */
} app_opt_t;

typedef struct {
	msg_mode_t mode;		/* CAN message ID */
	uint32_t option;		/* not used */
	int64_t val;			/* CAN message parameter value, TODO: upgrade to float type */
	uint32_t num;			/* loop number */
	uint32_t interval;		/* interval in ms when repeating message submission */
	timer_t timer_id;		/* ID of timer for counting interval */
	int timer_count;
	BOOL_INT32 timer_mark;
} msg_opt_t;

typedef struct {
	uint32_t project_id;	/* project ID */
	uint32_t project_func;	/* project function: manual test, automatic test, remote control */
	uint32_t msg_num;		/* CAN message number requested in command options; each message has a timer and timer number is message number */
	int canfd_status;		/* CAN communication status */
	int update_default;     /* 1 to update CAN message default value */
} sysData_type;

struct dataRaw {
    int32_t bat_soc;
    int32_t bat_vol;
    int32_t fsh;
    int32_t outside_temp;
    int32_t inside_temp;
    int32_t humidity;
    int32_t ws_temp;
    int32_t speed;
};

void app_main_displayHelp(const char *app);
void app_main_initData(sysData_type *sdata);
int app_main_getMsgnum(sysData_type *sdata);
int app_main_parseOption(sysData_type *sdata, int numOpt, char *strArg, app_opt_t *appOpt);
void app_main_sendCommand(sysData_type *sdata, int cmd);
void app_main_print_canVeh(sysData_type *sdata, int msgNum, int msgCount);
int app_main_remoteControl(sysData_type *sdata, int cmd);
int app_main_canTest(sysData_type *sdata, app_opt_t *appOpt);
void app_main_manualTest(sysData_type *sdata);
int app_main_autoTest(sysData_type *sdata);
void app_main_test(void);
void app_main_testApp(void);

#endif /* APP_MAIN_H_ */
