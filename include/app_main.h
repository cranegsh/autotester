/*
 * app_main.h
 *
 *  Created on: Dec 21, 2024
 *      Author: crane
 */

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include "app_main_id4.h"
#include "app_main_c3.h"
#include "app_main_navy.h"

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

struct sysData {
#ifdef PROJECT_ID4
	struct canfdData_id4 *dataCan;
#endif
#ifdef PROJECT_C3
	struct canfdData_c3 *dataCan;
#endif
#ifdef PROJECT_NAVY
	struct canfdData_navy *dataCan;
#endif
};


void app_main_displayHelp(const char *app);
void app_main_initData(struct sysData *sdata);
int app_main_processOption(int numOpt, app_opt_t *appOpt, char *strArg, int num);
void app_main_test(void);

#endif /* APP_MAIN_H_ */
