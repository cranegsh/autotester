/*
 * app_main.c
 *
 *  Created on: Dec 21, 2024
 *      Author: crane
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "app_main.h"
#include "sysconfig.h"
#include "utility.h"
#include "app_config.h"
#include "app_log.h"
#include "app_timer.h"
#include "app_main_id4.h"
#include "app_main_c3.h"
#include "app_main_navy.h"

static msg_opt_t msg[CAN_VEH_MSG_NUM] = {
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
	{ .timer_count = 0, .timer_mark = BOOL_TRUE },
};

static char *project_canopt[PROJECT_ID_TOTAL] = {
	PROJECT_CANOPT_ID4,
	PROJECT_CANOPT_ID4,
	PROJECT_CANOPT_ID4,
	PROJECT_CANOPT_C3,
	PROJECT_CANOPT_NAVY,
};

void app_main_displayHelp(const char *app)
{
	fprintf(stderr,
		"Usage: %s argument [ CAN message option(s) <param> ] [ Function option ]\n"
		"Options for help\n"
		"\t -H: display help information.\n"
		"Argument (must):\n"
		"\t id4:    id4 project\n"
		"\t g3:     g3 project\n"
		"\t g4r:    g4r project\n"
		"\t c3:     c3 project\n"
		"\t navy:   navy project\n"
		"Options for sending CAN message(s):\n"
		"\t -l <number>: loop numbers (default 1; 0 for infinite loop).\n"
		"\t -i <number>: interval time (millisecond, default in spec.). \n"
		"\t -p <number>: total running time (seconds, default 60; 0 for continuous run).\n"
		"\t -c <number>: Send State of Charge (%%).\n"
		"\t -f <0 / 1>: Send FSH Status (0 or 1).\n"
		"\t -a <number>: Send Ambient Temperature (celsius degree).\n"
		"\t -v <number>: Send Battery Voltage (V).\n"
		"\t -s <number>: Send Speed (Km/h).\n"
		"\t -t <number>: Send Cabin Temperature (celsius degree).\n"
		"\t -h <number>: Send Humidity (%%).\n"
		"\t -d <number>: Send System ID.\n"
		"\t -r <number>: Send Current (A).\n"
		"\t -o <0 / 1>: Send Op Mode (0 or 1).\n"
		"Options for functions (default to run remote control):\n"
		"\t -m: run manual test.\n"
		"\t -z: run auto test.\n"
		"\t -y: run CAN test.\n",
		app);
}

void app_main_initData(sysData_type *sdata)
{

}

int app_main_getMsgnum(sysData_type *sdata)
{
	return (int)strlen(project_canopt[sdata->project_id]);
}

static int app_main_checkOpt(uint32_t prj_num, char ch)
{
	int status = -1;

	if(NULL != project_canopt[prj_num]) {
		if(NULL != strchr(project_canopt[prj_num], ch)) {
			/* find the option */
			status = 0;
		}
	}

	ndPrintf("Project %s option -%c -> status %d\r\n", project_name[prj_num], ch, status);

	return status;
}

/* interrupt command options:
 * return	0: valid CAN message parameter option
 * 			1: valid CAN option
 * 			2: valid function option
 * 			3: no valid option
 * 		   -1: help or wrong parameter value
 */
int app_main_parseOption(sysData_type *sdata, int numOpt, char *strArg, app_opt_t *appOpt)
{
	int retVal = -1;
	switch(numOpt) {
		case PROJECT_FUNC_MT:
		case PROJECT_FUNC_AT:
		case PROJECT_FUNC_CAN:
			appOpt->function = numOpt;
			retVal = 2;
			break;
		case 'l':
			appOpt->num = (uint32_t)atoi(strArg);
			retVal = 1;
			break;
		case 'i':
			appOpt->interval = (uint32_t)atoi(strArg);
			retVal = 1;
			break;
		case 'p':
			appOpt->period = (uint32_t)atoi(strArg);
			retVal = 1;
			break;
		case 'f':
			appOpt->mode = APP_OPT_DEV_SEND_FSH;
			appOpt->val = (int32_t)atoi(strArg);
			if(1 >= appOpt->val) {
				dData.display = appOpt->val + 1;
				dData.number = appOpt->val;
				if(0 == app_main_checkOpt(sdata->project_id, 'f')) {
					retVal++;
				}
			}
			else {
				/* invalid option parameter */
				retVal = -1;
				iPrintf("Invalid FSH option %d!\r\n", appOpt->val);
			}
			ndPrintf("\r\nFSH Sts:\tNo.%d mode %d", timer_num, appOpt->mode);
			break;
		case 'a':
			appOpt->mode = APP_OPT_DEV_SEND_ATEMP;
			appOpt->val = (int32_t)atoi(strArg);
			dData.display = 3;
			dData.number = appOpt->val;
			ndPrintf("\r\nAmb.Temp.:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'a')) {
				retVal++;
			}
			break;
		case 'v':
			appOpt->mode = APP_OPT_DEV_SEND_VOLTAGE;
			ndPrintf("\r\nVoltage:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'v')) {
				retVal++;
			}
			break;
		case 'c':
			appOpt->mode = APP_OPT_DEV_SEND_SOC;
			ndPrintf("\r\nS.Charge:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'c')) {
				retVal++;
			}
			break;
		case 's':
			appOpt->mode = APP_OPT_DEV_SEND_SPEED;
			ndPrintf("\r\nV. Speed:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 's')) {
				retVal++;
			}
			break;
		case 't':
			appOpt->mode = APP_OPT_DEV_SEND_CTEMP;
			ndPrintf("\r\nCab Temp.:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 't')) {
				retVal++;
			}
			break;
		case 'h':
			appOpt->mode = APP_OPT_DEV_SEND_HUMIDITY;
			ndPrintf("\r\nHumidity:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'h')) {
				retVal++;
			}
			break;
		case 'd':
			appOpt->mode = APP_OPT_DEV_SEND_SYSID;
			ndPrintf("\r\nSystem ID:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'd')) {
				retVal++;
			}
			else {
				retVal = 1;
			}
			break;
		case 'r':
			appOpt->mode = APP_OPT_DEV_SEND_CURRENT;
			ndPrintf("\r\nCurrent:\tNo.%d mode %d", timer_num, appOpt->mode);
			if(0 == app_main_checkOpt(sdata->project_id, 'r')) {
				retVal++;
			}
			else {
				retVal = 1;
			}
			break;
		case 'o':
			appOpt->mode = APP_OPT_DEV_SEND_OPMODE;
			appOpt->val = (int32_t)atoi(strArg);
			if(1 >= appOpt->val) {
				if(0 == app_main_checkOpt(sdata->project_id, 'o')) {
					retVal++;
				}
				else {
					retVal = 1;
				}
			}
			else {
				/* invalid option parameter */
				iPrintf("\r\nInvalid operation mode option %d!\r\n", appOpt->val);
				retVal = -1;
			}
			ndPrintf("\r\nOp.mode:\tNo.%d mode %d", timer_num, appOpt->mode);
			break;
		case 'H':
			retVal = -1;
			break;
		case '?':
			// unknown option or missing argument
			iPrintf("Unknown option or missing argument!\n");
			retVal = 1;
			break;
		default:							/* it won't come here if there is no argument at all, why? */
			appOpt->mode = APP_OPT_UNKNOWN;
			retVal = 3;
			break;
	}

	if(0 == retVal) {
		/* get a valid option with parameter */
		appOpt->val = (int32_t)atoi(strArg);
		msg[sdata->msg_num].mode = appOpt->mode;
		msg[sdata->msg_num].val = appOpt->val;
		ndPrintf("\t Value %d", appOpt->val);
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
void app_main_sendCommand(sysData_type *sdata, int cmd)
{
	if(app_main_sendCommand_arr[sdata->project_id]) {
		app_main_sendCommand_arr[sdata->project_id](sdata, cmd);
	}
	else {
		iPrintf("Function app_main_sendCommand for %s not available!\r\n", project_name[sdata->project_id]);
		abort_program();
	}
}

static void (*app_main_getMsgvalue_arr[PROJECT_ID_TOTAL])(msg_opt_t *) = {
	NULL,
	NULL,
	NULL,
	app_main_c3_getMsgvalue,
	NULL,
};
static void app_main_getMsgvalue(sysData_type *sdata)
{
	/* init CAN messages and start sending them */
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		msg[i].mode = i;
		msg[i].val = 0;
	}
#if 0
	/* assign initial values; TODO: put these values in canfd data structure */
	msg[(uint32_t)APP_OPT_DEV_SEND_FSH].val = 0;
	msg[(uint32_t)APP_OPT_DEV_SEND_ATEMP].val = -10;
	msg[(uint32_t)APP_OPT_DEV_SEND_SPEED].val = 25;
#else
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		/* get the default value in initialized data structure */
		if(app_main_getMsgvalue_arr[sdata->project_id]) {
			app_main_getMsgvalue_arr[sdata->project_id](&msg[i]);
		}
		else {
			iPrintf("Function app_main_getMsgvalue for %s not available!\r\n", project_name[sdata->project_id]);
			abort_program();
		}
	}
#endif
}

static void (*app_main_getMsginfo_arr[PROJECT_ID_TOTAL])(msg_opt_t *) = {
	app_main_id4_getMsginfo,
	app_main_g3_getMsginfo,
	app_main_g4r_getMsginfo,
	app_main_c3_getMsginfo,//app_main_id4_getMsginfo, // replace with id4's for debugging the issue of exiting program
	NULL,
};
#if 0
static void app_main_initMsgsend(sysData_type *sdata)
{
	dPrintf("\nTotal msg #: %d", sdata->msg_num);
	ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
	iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

	for(uint32_t i=0; i<sdata->msg_num; i++) {
		/* get the default interval and total number and display msg information */
		if(app_main_getMsginfo_arr[sdata->project_id]) {
			app_main_getMsginfo_arr[sdata->project_id](&msg[i]);
		}
		else {
			iPrintf("Function app_main_getMsginfo for %s not available!\r\n", project_name[sdata->project_id]);
			abort_program();
		}
	}

	/* start timer to use timer to control CAN message submission */
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		start_timer(&msg, i, sdata);
	}
}
#endif

static void app_main_initMsg(sysData_type *sdata)
{
	dPrintf("\nTotal msg #: %d", sdata->msg_num);
	ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
	iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

	for(uint32_t i=0; i<sdata->msg_num; i++) {
		/* get the default interval and total number and display msg information */
		if(app_main_getMsginfo_arr[sdata->project_id]) {
			app_main_getMsginfo_arr[sdata->project_id](&msg[i]);
		}
		else {
			iPrintf("Function app_main_getMsginfo for %s not available!\r\n", project_name[sdata->project_id]);
			abort_program();
		}
	}
}

/* send all CAN messages once */
static void app_main_sendMsg(sysData_type *sdata)
{
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		msg_canfd_send_veh(sdata->project_id, msg[i].mode, msg[i].val);
	}
}

static void app_main_startMsgtimer(sysData_type *sdata)
{
	/* start timer to use timer to control CAN message submission */
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		start_timer(&msg, i, sdata);
	}
}

static uint32_t app_main_checkMsgTimer(uint32_t num, uint32_t count)
{
	for(uint32_t i=0; i<num; i++) {
		/* search all the timer to check if any one completed the submission */
		if( (msg[i].timer_count >= msg[i].num) && (0 != msg[i].num) ) {
			if(BOOL_TRUE == msg[i].timer_mark) {
				/* if this timer is not stopped, stop it */
				stop_timer(&msg[i].timer_id);

				/* mark it after it is stopped */
				msg[i].timer_mark = BOOL_FALSE;

    			/* update the total number of the timer left */
				count++;
        		ndPrintf(" | stopped %d\n", mark);
			}
		}
	}

	return count;
}

static void (*app_main_print_canVeh_arr[PROJECT_ID_TOTAL])(uint32_t, int, int) = {
	app_main_id4_print_canVeh,
	app_main_g3_print_canVeh,
	app_main_g4r_print_canVeh,
	app_main_c3_print_canVeh,
	NULL,
};
void app_main_print_canVeh(sysData_type *sdata, int msgNum, int msgCount)
{
    if(app_main_print_canVeh_arr[sdata->project_id]){
    	app_main_print_canVeh_arr[sdata->project_id](sdata->project_func, msgNum, msgCount);
    }
	else {
		iPrintf("Function app_main_print_canVeh for %s not available!\r\n", project_name[sdata->project_id]);
		abort_program();
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
static void (*app_main_print_arr[PROJECT_ID_TOTAL])(void) = {
	app_main_id4_print,
	NULL,
	NULL,
	NULL,
	app_main_navy_print,
};

static void app_main_periodicDisplay(uint32_t prj_num)
{
	static int timer_display = 0;						/* timer to count display interval */
	if(1 < (time(NULL) - timer_display)) {
	    if(app_main_print_arr[prj_num]){
	    	app_main_print_arr[prj_num]();
	    }
		else {
			iPrintf("Function app_main_print for %s not available!\r\n", project_name[prj_num]);
			abort_program();
		}
		timer_display = time(NULL);
	}
}

int app_main_remoteControl(sysData_type *sdata, int cmd)
{
	int ret = 0;
	uint32_t prj_num;

	prj_num = sdata->project_id;
	switch(cmd) {
		case COMMAND_C:
		case COMMAND_F:
			break;
		case COMMAND_P:
		    if(app_main_commandP_arr[prj_num]){
		    	app_main_commandP_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandP for %s not available!\r\n", project_name[prj_num]);
				abort_program();
			}
			break;
		case COMMAND_D444:
		    if(app_main_commandD_log_arr[prj_num]){
		    	app_main_commandD_log_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandD_log for %s not available!\r\n", project_name[prj_num]);
				abort_program();
			}
			break;
		case COMMAND_D333:
		    if(app_main_commandD_error_arr[prj_num]){
		    	app_main_commandD_error_arr[prj_num]();
		    }
			else {
				iPrintf("Function app_main_commandD_error for %s not available!\r\n", project_name[prj_num]);
				abort_program();
			}
			break;
		case COMMAND_D222:
			/* receive CAN messages */
			if(-1 == msg_canfd_receive(prj_num)) {
				msg_canfd_cleanData(prj_num);
			}
			app_main_periodicDisplay(prj_num);
			ret = 1;
			break;
		default:
			/* not valid command */
			ret = -1;
	}

	return ret;
}

int app_main_canTest(sysData_type *sdata, app_opt_t *appOpt)
{
    time_t time_ori;							/* CAN message submission start timing */
    uint32_t msg_done_count = 0;				/* this is to count the message that its submission is complete */

    if(PROJECT_FUNC_CAN == sdata->project_func) {
    	/* use the value in the spec (source code) */
    	dPrintf("app_main_canTest: set message mode and value\r\n");
    	app_main_getMsgvalue(sdata);
    	appOpt->period = 0;
    }

#if 0
    app_main_initMsgsend(sdata);
#else
    app_main_initMsg(sdata);
    app_main_startMsgtimer(sdata);
#endif

    /* use the interval and loop number from command */
	for(uint32_t i=0; i<sdata->msg_num; i++) {
		if(PROJECT_FUNC_CAN != sdata->project_func) {
			/* use the num from command -l option */
			//msg[i].interval = appOpt->interval;	/* from -i option, default is in source code (structure initialized) */
			msg[i].num = appOpt->num;				/* from -l option, default is 1 (0 is infinite loop) */
		}
		ndPrintf("msg count: #%d %d\r\n", i, msg[i].num);
	}

    time_ori = time(NULL);
	for(;;) {
		/* in continuous CAN test mode, check if all messages' all submission is complete */
		msg_done_count = app_main_checkMsgTimer(sdata->msg_num, msg_done_count);
		if((sdata->msg_num == msg_done_count)
			|| ((0 != appOpt->period) && (appOpt->period < (time(NULL) - time_ori)))) {
			/* all the submission for all messages is complete or test time is up */
			fflush(stdout);			/* TODO: check how to flush the stdout buffer? */
			ndPrintf("Compare %d to %d\r\n", msg_done_count, sdata->msg_num);
			ndPrintf("Test time %d[s] is up. \r\n", appOpt->period);
			break;
		}

		/* exit the while loop after pressing 'x' and ENTER */
		if('x' == getc(stdin)) {
			dPrintf("Exiting the program ...\r\n");
			break;
		}
	}

	printf("\r\n");
	return 0;
}

static void (*app_main_displayMsg_arr[PROJECT_ID_TOTAL])(msg_opt_t *) = {
	NULL,
	app_main_displayMsg_g3,
	NULL,
	app_main_displayMsg_c3,
	NULL,
};

static void app_main_displayMsg(sysData_type *sdata)
{
	dPrintf("\nTotal msg #: %d", sdata->msg_num);
	iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

	for(uint32_t i=0; i<sdata->msg_num; i++) {
		if(app_main_displayMsg_arr[sdata->project_id]) {
			app_main_displayMsg_arr[sdata->project_id](&msg[i]);
		}
		else {
			iPrintf("Function app_main_displayMsg for %s not available!\r\n", project_name[sdata->project_id]);
			abort_program();
		}
	}
}

static int app_main_updateMsg(int cmd, uint32_t total)
{
	uint32_t i;
	int retVal = 0;

	switch(cmd) {
		case PROJECT_MT_FUNC1:
		case (PROJECT_MT_FUNC1 - 32):
			ndPrintf("Push defrost ON ...\r\n");
			for(i=0; i<total; i++) {
				if(APP_OPT_DEV_SEND_FSH == msg[i].mode) {
					if(0 == msg[i].val) {
						msg[i].val = 1;
						retVal = cmd;
					}
				}
			}
			break;
		case PROJECT_MT_FUNC2:
		case (PROJECT_MT_FUNC2 - 32):
			ndPrintf("Push defrost OFF ...\r\n");
			for(i=0; i<total; i++) {
				if(APP_OPT_DEV_SEND_FSH == msg[i].mode) {
					if(1 == msg[i].val) {
						msg[i].val = 0;
						retVal = cmd;
					}
				}
			}
			break;
		case PROJECT_MT_FUNC3:
		case (PROJECT_MT_FUNC3 - 32):
			ndPrintf("Set ambient temperature ...\r\n");
			for(i=0; i<total; i++) {
				if(APP_OPT_DEV_SEND_ATEMP == msg[i].mode) {
					msg[i].val = get_a_number_mt("ambient temperature");
					retVal = cmd;
				}
			}
			break;
		case PROJECT_MT_FUNC4:
		case (PROJECT_MT_FUNC4 - 32):
			ndPrintf("Set vehicle speed ...\r\n");
			for(i=0; i<total; i++) {
				if(APP_OPT_DEV_SEND_SPEED == msg[i].mode) {
					msg[i].val = get_a_number_mt("vehicle speed");
					retVal = cmd;
				}
			}
			break;
		default:
			break;
	}

	ndPrintf("Returning %d\r\n", retVal);
	return retVal;
}

void app_main_resetMsginfo(sysData_type *sdata, int cmd)
{
	/* update message and send it out, no need for project navy */
	if(PROJECT_ID_NAVY != sdata->project_id) {
		/* execute the command to update messages with new value */
		app_main_updateMsg(cmd, sdata->msg_num);
		/* update messages' display */
		app_main_displayMsg(sdata);
	}
	else {
		iPrintf("Function app_main_updateMsg for %s not available!\r\n", project_name[sdata->project_id]);
		abort_program();
	}
}

#if 0
static void app_main_displayResult(uint32_t prj_num)
{
	static struct timespec time_prev, time_current;
	int hitkey;

	/* only need for project c3 as it sends out the result through CAN while others don't */
	if(PROJECT_ID_C3 == prj_num) {
		do {
			hitkey = (int)get_a_char_nb();

			msg_canfd_receive(prj_num);

			/* display the results every one second */
			clock_gettime(CLOCK_MONOTONIC, &time_current);
			if(1 < (time_current.tv_sec - time_prev.tv_sec)) {
			//if(20000000 < (time_current.tv_nsec - time_prev.tv_nsec)) {		/* not working welll */
				/* display results */
				app_main_displayResult_c3();
				clock_gettime(CLOCK_MONOTONIC, &time_prev);
			}
		} while(!(((PROJECT_MT_FUNCq == hitkey) || ((PROJECT_MT_FUNCq - 32) == hitkey))));
	}
}
#endif
static void app_main_displayResult(uint32_t prj_num)
{
	long int timer_display = 0;
	int hitkey;

	/* only need for project c3 as it sends out the result through CAN while others don't */
	if(PROJECT_ID_C3 == prj_num) {
		do {
			hitkey = (int)get_a_char_nb();

			msg_canfd_receive(prj_num);

			/* display the results every one second */
			if(1 < (time(NULL) - timer_display)) {			/* about 1.6s */
				/* display results */
				app_main_displayResult_c3();
				timer_display = time(NULL);
			}
		} while(!(((PROJECT_MT_FUNCq == hitkey) || ((PROJECT_MT_FUNCq - 32) == hitkey))));
		fflush(stdout);
	}
}
static void app_main_checkResult(uint32_t prj_num)
{
	int hitkey;

	/* only need for project c3 as it sends out the result through CAN while others don't */
	if(PROJECT_ID_C3 == prj_num) {
		iPrintf("\r\nChecking result ... press '%c' to return!\r\n", PROJECT_MT_FUNCq);
		do {
			hitkey = (int)get_a_char_nb();
			/* receive CAN messages */
			msg_canfd_receive(prj_num);
			/* check the results */
			ndPrintf("Checking...");
			app_main_checkResult_c3();
		} while(!(((PROJECT_MT_FUNCq == hitkey) || ((PROJECT_MT_FUNCq - 32) == hitkey))));
		fflush(stdout);
	}
}

#if 0
static int app_config_mt_g3(uint32_t prj_num)
{
    int hitkey;

	iPrintf("\r\nPlease select command:");
	iPrintf("\r\n %c): Push defrost ON", PROJECT_MT_FUNC1);
	iPrintf("\r\n %c): Push defrost OFF", PROJECT_MT_FUNC2);
	iPrintf("\r\n %c): Set ambient temperature", PROJECT_MT_FUNC3);
	iPrintf("\r\n %c): Set vehicle speed", PROJECT_MT_FUNC4);
	iPrintf("\r\n %c): exit", PROJECT_MT_FUNCx);
	iPrintf("\r\n ->: ");

	do {
		//hitkey = (int)get_a_char_nb_wHandler(app_main_displayResult, prj_num);
		//hitkey = (int)get_a_char_nb_wHandler(app_main_checkResult, prj_num);
		hitkey = (int)get_a_char_nb();  app_main_checkResult(prj_num);
	} while(!((('a' <= hitkey) && ('z' >= hitkey))
			|| (('A' <= hitkey) && ('Z' >= hitkey))));			/* TODO: check function key which might contain these letters! */

	/* collect the enter key */
	//getc(stdin);
	//fflush(stdout);

    return hitkey;
}
#endif

static int app_config_mt_g3(uint32_t prj_num)
{
	iPrintf("\r\nPlease select command:");
	iPrintf("\r\n %c): Push defrost ON", PROJECT_MT_FUNC1);
	iPrintf("\r\n %c): Push defrost OFF", PROJECT_MT_FUNC2);
	iPrintf("\r\n %c): Set ambient temperature", PROJECT_MT_FUNC3);
	iPrintf("\r\n %c): Set vehicle speed", PROJECT_MT_FUNC4);
	iPrintf("\r\n %c): Check result", PROJECT_MT_FUNCr);
	iPrintf("\r\n %c): exit", PROJECT_MT_FUNCx);
	iPrintf("\r\n ->: ");

	int hitkey = 0;
	do {
		hitkey = (int)get_a_char();
	} while(!(((PROJECT_MT_FUNC1 <= hitkey) && (PROJECT_MT_FUNC4 >= hitkey))
			|| (((PROJECT_MT_FUNC1 - 32) <= hitkey) && ((PROJECT_MT_FUNC1 - 32) >= hitkey))
			|| ((PROJECT_MT_FUNCr == hitkey) && (PROJECT_MT_FUNCr == hitkey))
			|| ((PROJECT_MT_FUNCx == hitkey) && (PROJECT_MT_FUNCx == hitkey))));		/* TODO: check function key which might contain these letters! */

	/* collect the enter key */
	//getc(stdin);
	fflush(stdout);

    return hitkey;
}

static int (*app_config_mt_arr[PROJECT_ID_TOTAL])(uint32_t) = {
	NULL,
	app_config_mt_g3,
	NULL,
	app_config_mt_g3,			/* same as g3 for now */
	NULL,
};
static int app_config_mt(uint32_t prj_num)
{
	int ret;

    if(app_config_mt_arr[prj_num]){
    	ret = app_config_mt_arr[prj_num](prj_num);
    }
	else {
		iPrintf("Function app_config_mt for %s not available!\r\n", project_name[prj_num]);
		abort_program();
	}

    return ret;
}

#define MT_CAN_MSG_SEND_ONCE		/* sent once instead of periodically sent controlled by timer */
void app_main_manualTest(sysData_type *sdata)
{
	int command = 0;

	app_main_getMsgvalue(sdata);
	app_main_initMsg(sdata);
#ifdef MT_CAN_MSG_SEND_ONCE
	app_main_sendMsg(sdata);
#else
	app_main_startMsgtimer(sdata);
#endif

#if 0	/* for debug console input issue */
	while(1) {
		if(PROJECT_MT_FUNCx == getc(stdin)) {
			dPrintf("Exiting the program ...\r\n");
			break;
		}
	}
#endif
	while(1) {
#if 0		/* display results periodically */
		app_main_displayResult(sdata->project_id);
#else		/* display only when any value changes */
		app_main_checkResult(sdata->project_id);
#endif

		command = app_config_mt(sdata->project_id);
		if((PROJECT_MT_FUNCx == command) || ((PROJECT_MT_FUNCx - 32) == command)) {
			/* break the loop and exit */
			break;
		}
		else {
			app_main_resetMsginfo(sdata, command);
#ifdef MT_CAN_MSG_SEND_ONCE
			/* submit the message */
			app_main_sendMsg(sdata);
#endif
		}
	}
}

int app_main_autoTest(sysData_type *sdata)
{

	return 0;
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

void app_main_testApp(void)
{
#if 0	/* TODO: check why it doesn't work to received and send at the same time!
 	 	 	 	 only send at the beginning and then receiving works. */
	uint32_t msgId = 0x5B, msgdataNum = 8;
	uint8_t msgData[8] = {1, 2, 3, 4, 5, 6, 7, 8};

	canfd_messageSend(msgId, (uint8_t*)msgData, msgdataNum);

	while(1) {
		canfd_messageReceive(&msgId, (uint8_t*)msgData, &msgdataNum);
#if 0
		usleep(5000000);
		canfd_messageSend(msgId, (uint8_t*)msgData, msgdataNum);
#endif
	}
#endif
}


