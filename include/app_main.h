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
	uint32_t addr;
	int16_t val;
	uint16_t num;			/* loop number */
	uint16_t interval;		/* interval in ms between each message submission */
	uint16_t period;		/* total running time in seconds */
	uint16_t option;
	msg_mode_t mode;
} app_opt_t;

typedef struct {
	msg_mode_t mode;
	timer_t timer_id;
	int32_t val;
	uint32_t num;			/* loop number */
	uint32_t interval;		/* interval in ms between each message submission */
	uint32_t option;
	int timer_count;
	BOOL_INT32 timer_mark;
} msg_opt_t;

typedef struct {
	int canfd_status;
#ifdef PROJECT_ID4
	struct canfdData_id4 *dataCan;
#endif
#ifdef PROJECT_C3
	struct canfdData_c3 *dataCan;
#endif
#ifdef PROJECT_NAVY
	struct canfdData_navy *dataCan;
#endif
} sysData_type;

void app_main_displayHelp(const char *app);
void app_main_initData(sysData_type *sdata);
int app_main_processOption(int numOpt, app_opt_t *appOpt, char *strArg, int *num);
void app_main_sendCommand(sysData_type *sdata, int cmd);
void app_main_initMsg(int num, app_opt_t *appOpt);
int app_main_checkMsg(int num, int mark);
int app_main_remoteControl(app_opt_t *appOpt, int cmd, sysData_type *sdata);
void app_main_test(void);

#endif /* APP_MAIN_H_ */
