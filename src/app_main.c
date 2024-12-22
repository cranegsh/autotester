/*
 * app_main.c
 *
 *  Created on: Dec 21, 2024
 *      Author: crane
 */
#include <stdio.h>
#include <stdlib.h>

#include "app_main.h"
#include "sysconfig.h"
#include "utility.h"
#include "app_config.h"
#include "app_log.h"
#include "app_timer.h"
#include "app_main_id4.h"
#include "app_main_c3.h"
#include "app_main_navy.h"

msg_opt_t msg[CAN_VEH_MSG_NUM] = {
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
#ifdef PROJECT_C3
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
#endif
};

int timer_display = 0;

void app_main_displayHelp(const char *app)
{
	fprintf(stderr,
		"Usage: %s [ options <param> ] argument\n"
		"Options:\n"
		"\t -l <number>: loop numbers (must option, 0 for infinite loop).\n"
		"\t -i <number>: interval time (millisecond, default in source code). \n"
		"\t -p <number>: total running time (seconds, default 60, 0 for continuous run).\n"
		"\t -c <number>: Send Start of Charge (percentage).\n"
		"\t -f <number>: Send FSH Status (0 or 1).\n"
		"\t -a <number>: Send Ambient Temperature (celsius degree).\n"
		"\t -v <number>: Send Battery Voltage (V).\n"
		"\t -s <number>: Send Speed (Km/h).\n"
		"\t -t <number>: Send Cabin Temperature (celsius degree).\n"
		"\t -h <number>: Send Humidity (percentage).\n"
#ifdef PROJECT_C3
		"\t -d <number>: Send System ID.\n"
		"\t -r <number>: Send Current (V).\n"
		"\t -o <number>: Send Op Mode (0 or 1).\n"
#endif
		"\t -H: display usage information.\n"
		"Argument (must):\n"
		"\t id4: id4 project\n"
		"\t g3:  g3 project\n"
		"\t g4r: g4r project\n"
		"\t c3:  c3 project\n"
		"\t navy:navy project\n",
		app);
}

void app_main_initData(sysData_type *sdata)
{
#ifdef PROJECT_ID4
	sdata->dataCan = msg_canfd_getData_id4();
#endif
#ifdef PROJECT_C3
	sdata->dataCan = msg_canfd_getData_c3();
#endif
#ifdef PROJECT_NAVY
	sdata->dataCan = msg_canfd_getData_navy();
#endif
}

int app_main_processOption(int numOpt, app_opt_t *appOpt, char *strArg, int *num)
{
	int retVal = *num;
	switch(numOpt) {
		case 'l':
			appOpt->num = atoi(strArg);
			break;
		case 'i':
			appOpt->interval = atoi(strArg);
			break;
		case 'p':
			appOpt->period = atoi(strArg);
			break;
		case 'f':
			appOpt->mode = APP_OPT_DEV_SEND_FSH;
			appOpt->val = atoi(strArg);
			if(1 >= appOpt->val) {
				dData.display = appOpt->val + 1;
				dData.number = appOpt->val;
				retVal++;
			}
			else {
				/* invalid option parameter */
				retVal = -1;
				iPrintf("Invalid FSH option %d!\r\n", appOpt->val);
			}
			ndPrintf("\r\nFSH Sts:\tNo.%d mode %d", num, appOpt->mode);
			break;
		case 'a':
			appOpt->mode = APP_OPT_DEV_SEND_ATEMP;
			appOpt->val = atoi(strArg);
			dData.display = 3;
			dData.number = appOpt->val;
			ndPrintf("\r\nAmb.Temp.:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 'v':
			appOpt->mode = APP_OPT_DEV_SEND_VOLTAGE;
			ndPrintf("\r\nVoltage:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 'c':
			appOpt->mode = APP_OPT_DEV_SEND_SOC;
			ndPrintf("\r\nS.Charge:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 's':
			appOpt->mode = APP_OPT_DEV_SEND_SPEED;
			ndPrintf("\r\nV. Speed:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 't':
			appOpt->mode = APP_OPT_DEV_SEND_CTEMP;
			ndPrintf("\r\nCab Temp.:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 'h':
			appOpt->mode = APP_OPT_DEV_SEND_HUMIDITY;
			ndPrintf("\r\nHumidity:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
#ifdef PROJECT_C3
		case 'd':
			appOpt->mode = APP_OPT_DEV_SEND_SYSID;
			ndPrintf("\r\nSystem ID:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 'r':
			appOpt->mode = APP_OPT_DEV_SEND_CURRENT;
			ndPrintf("\r\nCurrent:\tNo.%d mode %d", num, appOpt->mode);
			retVal++;
			break;
		case 'o':
			appOpt->mode = APP_OPT_DEV_SEND_OPMODE;
			if(1 >= appOpt->val) {
				retVal++;
			}
			else {
				/* invalid option parameter */
				iPrintf("Invalid operation mode option %d!\r\n", appOpt->val);
				retVal = -1;
			}
			ndPrintf("\r\nOp.mode:\tNo.%d mode %d", num, appOpt->mode);
			break;
#endif
		case 'H':
			retVal = -1;
			break;
		case '?':
			// unknown option or missing argument
			iPrintf("Unknown option or missing argument!\n");
			retVal = 0;
			break;
		default:							/* it won't come here if there is no argument at all, why? */
			appOpt->mode = APP_OPT_UNKNOWN;
			retVal = 0;
			break;
	}

	if((*num + 1) == retVal) {
		/* get an option with parameter */
		appOpt->val = atoi(strArg);
		msg[*num].mode = appOpt->mode;
		msg[*num].val = appOpt->val;
		ndPrintf("\t Value %d", appOpt->val);
		*num = retVal;
	}

	return retVal;
}

void app_main_sendCommand(sysData_type *sdata, int cmd)
{
	if((0 == sdata->canfd_status) && (sdata->dataCan->updated)) {
		ndPrintf("\n Sending data '%c' to CAN ...", cmd);
		msg_canfd_send_tester((uint32_t)cmd);
		ndPrintf("\n data '%c' to CAN sent!", cmd);
		sdata->dataCan->updated = BOOL_FALSE;
	}
}

void app_main_initMsg(int num, app_opt_t *appOpt)
{
	dPrintf("\nTotal msg #: %d", num);
	ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
	iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

	for(int i=0; i<num; i++) {
		/* get the default interval and total number and display msg information */
#ifdef PROJECT_ID4
		app_main_id4_getMsginfo(&msg[i]);
#endif
#ifdef PROJECT_C3
		app_main_c3_getMsginfo(&msg[i]);
#endif
		/* use the command interval and total number */
		//msg[i].interval = appOpt->interval;
		msg[i].num = appOpt->num;
	}

	for(int i=0; i<num; i++) {
		start_timer(&msg, i, num);
	}
}

int app_main_checkMsg(int num, int mark)
{
	for(int i=0; i<num; i++) {
		/* search all the timer to check if any one completed the submission */
		if( (msg[i].timer_count >= msg[i].num) && (0 != msg[i].num) ) {
			if(BOOL_TRUE == msg[i].timer_mark) {
				/* if this timer is not stopped, stop it */
				stop_timer(&msg[i].timer_id);

				/* mark it after it is stopped */
				msg[i].timer_mark = BOOL_FALSE;

    			/* update the total number of the timer left */
				mark++;
        		ndPrintf(" | stopped %d\n", mark);
			}
		}
	}

	return mark;
}

int app_main_remoteControl(app_opt_t *appOpt, int cmd, sysData_type *sdata)
{
	int ret = 0;

	switch(cmd) {
		case COMMAND_C:
		case COMMAND_F:
			break;
		case COMMAND_P:
#ifdef PROJECT_ID4
			app_main_id4_commandP();
#endif
			break;
		case COMMAND_D444:
#ifdef PROJECT_ID4
			app_main_id4_commandD_log();
#endif
			break;
		case COMMAND_D333:
#ifdef PROJECT_ID4
			app_main_id4_commandD_error();
#endif
			break;
		case COMMAND_D222:
			/* receive CAN messages */
			if(0 == sdata->canfd_status) {
				msg_canfd_receive();
			}

			if(1 < (time(NULL) - timer_display)) {
#ifdef PROJECT_ID4
				app_main_id4_print();
#endif
#ifdef PROJECT_NAVY
				msg_canfd_navy_print();
#endif
				timer_display = time(NULL);
			}
			ret = 1;
			break;
		default:
			/* not valid command */
			ret = -1;
	}

	return ret;
}

void app_main_test(void)
{
#ifdef DEVELOP_VERSION
    dPrintf("RPI: uint8_t: %d | uint16_t: %d | uint32_t: %d\n", \
            (int)sizeof(uint8_t), (int)sizeof(uint16_t), (int)sizeof(uint32_t));
    dPrintf("RPI: bool: %d | short: %d | int: %d | long int: %d | float: %d\n", \
    		(int)sizeof(bool), (int)sizeof(short), (int)sizeof(int), (int)sizeof(long int), (int)sizeof(float));
    dPrintf("RPI: struct petdConfig: %d | struct pwmConfig: %d | struct id4DataIn_Cfg_type: %d\n", \
    		PETD_CONFIG_LEN, PWM_CONFIG_LEN, CAN_DATA_IN_CFG_LEN);
#endif
}


