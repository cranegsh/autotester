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

int timer_num = 0;							/* each message has a timer and timer number is message number */
int timer_count = 0;						/* this is to count the message that its submission is complete */
int timer_display = 0;						/* timer to cound display interval */

void app_main_displayHelp(const char *app)
{
	fprintf(stderr,
		"Usage: %s [ options <param> ] argument\n"
		"Options:\n"
		"\t -m: run manual test.\n"
		"\t -z: run auto test.\n"
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

}

int app_main_processOption(int numOpt, app_opt_t *appOpt, char *strArg)
{
	int retVal = timer_num;
	switch(numOpt) {
		case 'm':
		case 'z':
			appOpt->function = numOpt;
			break;
		case 'l':
			appOpt->num = (uint32_t)atoi(strArg);
			break;
		case 'i':
			appOpt->interval = (uint32_t)atoi(strArg);
			break;
		case 'p':
			appOpt->period = (uint32_t)atoi(strArg);
			break;
		case 'f':
			appOpt->mode = APP_OPT_DEV_SEND_FSH;
			appOpt->val = (int32_t)atoi(strArg);
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
			appOpt->val = (int32_t)atoi(strArg);
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

	if((timer_num + 1) == retVal) {
		/* get an option with parameter */
		appOpt->val = (int32_t)atoi(strArg);
		msg[timer_num].mode = appOpt->mode;
		msg[timer_num].val = appOpt->val;
		ndPrintf("\t Value %d", appOpt->val);
		timer_num = retVal;
	}

	return retVal;
}

static void (*app_main_sendCommand_arr[PROJECT_ID_TOTAL])(sysData_type *, int) = {
		app_main_id4_sendCommand,
		NULL,
		NULL,
		app_main_c3_sendCommand,
		NULL,
};
void app_main_sendCommand(uint32_t prj_num, sysData_type *sdata, int cmd)
{
	if(app_main_sendCommand_arr[prj_num]) {
		app_main_sendCommand_arr[prj_num](sdata, cmd);
	}
	else {
		iPrintf("Function app_main_sendCommand No. %d not available!\r\n", prj_num);
	}
}

static void (*app_main_initMsg_arr[PROJECT_ID_TOTAL])(msg_opt_t *) = {
	app_main_id4_getMsginfo,
	app_main_g3_getMsginfo,
	app_main_g4r_getMsginfo,
	app_main_c3_getMsginfo,
	NULL,
};
void app_main_initMsg(uint32_t prj_num, app_opt_t *appOpt)
{
	dPrintf("\nTotal msg #: %d", timer_num);
	ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
	iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

	for(int i=0; i<timer_num; i++) {
		/* get the default interval and total number and display msg information */
		if(app_main_initMsg_arr[prj_num]) {
			app_main_initMsg_arr[prj_num](&msg[i]);
		}
		else {
			iPrintf("Function app_main_initMsg No. %d not available!\r\n", prj_num);
		}
		/* use the command interval and total number */
		//msg[i].interval = appOpt->interval;
		msg[i].num = appOpt->num;
	}

	for(int i=0; i<timer_num; i++) {
		start_timer(&msg, i, timer_num);
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

static void (*app_main_print_canVeh_arr[PROJECT_ID_TOTAL])(int, int) = {
	app_main_id4_print_canVeh,
	app_main_g3_print_canVeh,
	app_main_g4r_print_canVeh,
	app_main_c3_print_canVeh,
	NULL,
};
void app_main_print_canVeh(uint32_t prj_num, int msgNum, int msgCount)
{
    if(app_main_print_canVeh_arr[prj_num]){
    	app_main_print_canVeh_arr[prj_num](msgNum, msgCount);
    }
	else {
		iPrintf("Function app_main_print_canVeh No. %d not available!\r\n", prj_num);
	}
}

static int (*app_main_commandP_arr[PROJECT_ID_TOTAL])(void) = {
	app_main_id4_commandP,
	NULL,
	NULL,
	NULL,
	NULL,
};
static void (*app_main_commandD_log_arr[PROJECT_ID_TOTAL])(void) = {
	app_main_id4_commandD_log,
	NULL,
	NULL,
	NULL,
	NULL,
};
static int (*app_main_commandD_error_arr[PROJECT_ID_TOTAL])(void) = {
	app_main_id4_commandD_error,
	NULL,
	NULL,
	NULL,
	NULL,
};
static void (*app_main_print_err[PROJECT_ID_TOTAL])(void) = {
	app_main_id4_print,
	NULL,
	NULL,
	NULL,
	app_main_navy_print,
};
int app_main_remoteControl(uint32_t prj_num, int cmd, sysData_type *sdata)
{
	int ret = 0;

	switch(cmd) {
		case COMMAND_C:
		case COMMAND_F:
			break;
		case COMMAND_P:
		    if(app_main_commandP_arr[prj_num]){
		    	app_main_commandP_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandP No. %d not available!\r\n", prj_num);
			}
			break;
		case COMMAND_D444:
		    if(app_main_commandD_log_arr[prj_num]){
		    	app_main_commandD_log_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandD_log No. %d not available!\r\n", prj_num);
			}
			break;
		case COMMAND_D333:
		    if(app_main_commandD_error_arr[prj_num]){
		    	app_main_commandD_error_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandD_error No. %d not available!\r\n", prj_num);
			}
			break;
		case COMMAND_D222:
			/* receive CAN messages */
			if(0 == sdata->canfd_status) {
				msg_canfd_receive(prj_num);
			}

			if(1 < (time(NULL) - timer_display)) {
			    if(app_main_print_err[prj_num]){
			    	app_main_print_err[prj_num]();
			    }
				else {
					iPrintf("Function app_main_print No. %d not available!\r\n", prj_num);
				}
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

int app_main_canTest(app_opt_t *appOpt)
{
    time_t time_ori;

    time_ori = time(NULL);
	for(;;) {
		/* in continuous CAN test mode, check if all messages' all submission is complete */
		timer_count = app_main_checkMsg(timer_num, timer_count);
		if((timer_num == timer_count)
			|| ((0 != appOpt->period) && (appOpt->period < (time(NULL) - time_ori)))) {
			/* all the submission for all messages is complete or test time is up */
			printf("\n");
			return 0;
		}
	}
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


