#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include "sysconfig.h"
#include "utility.h"
#include "app_canfd.h"
#include "app_config.h"
#include "app_log.h"
#include "app_timer.h"
#include "app_main.h"
#include "app_main_c3.h"
#include "app_main_id4.h"
#include "app_main_navy.h"

#define DEFAULT_MSG_INTERVAL		1000	/* default interval between same messages in ms; not useful any more as each message has its own */

timer_t timer_display;
int timer_display_ori = 0;

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
int timer_num = 0;

struct sysData farview_data;
int canfd_status = -1;


/**************************************************************************************
 * 						main program
 **************************************************************************************/
int main(int argc, char *argv[]) {
	int i, ret = 0, status = 0;
    int input_value = 0;

    time_t time_ori;
	app_opt_t opt;
	opt.val = 0;
	opt.num = 0;
	opt.interval = DEFAULT_MSG_INTERVAL;
	opt.period = 0;
	opt.option = 0xFF;
	opt.mode = APP_OPT_UNKNOWN;

	/* get the option and parameters if needed (followed with :) */
//	dPrintf("\r\nGetting %d arguments and the option is %d\n", argc, ret);
	while(-1 != (ret = getopt(argc, argv, "l:i:p:f:a:v:c:s:t:h:d:r:o:H"))) {
		ndPrintf("\r\nGet %d arguments and the option is %c\n", argc, ret);
		status = app_main_processOption(ret, &opt, optarg, timer_num);
		if((timer_num + 1) == status) {
			/* get an option with parameter */
			opt.val = atoi(optarg);
			msg[timer_num].mode = opt.mode;
			msg[timer_num].val = opt.val;
			ndPrintf("\t Value %d", opt.val);
			timer_num += 1;
		}
		else if (-1 == status) {
			app_main_displayHelp(argv[0]);
			return -1;
		}
	}

	/* get the argument */
	if(optind < argc) {
		if(0 == strcmp(PROJECT_ARGU_ID4, argv[optind])) {
			idProject = 1;
		}
		else if(0 == strcmp(PROJECT_ARGU_G3, argv[optind])) {
			idProject = 2;
		}
		else if(0 == strcmp(PROJECT_ARGU_G4R, argv[optind])) {
			idProject = 3;
		}
		else if(0 == strcmp(PROJECT_ARGU_C3, argv[optind])) {
			idProject = 4;
		}
		else if(0 == strcmp(PROJECT_ARGU_NAVY, argv[optind])) {
			idProject = 5;
		}
		else {
			idProject = 0;
			printf("\r\nError in command: wrong argument! Please check help.\r\n");
			return -1;
		}
		printf("\r\nNon-option argument:%s %d\r\n", argv[optind], idProject);
	}
	else {
		idProject = PROJECT_ID_DEFAULT;
		//printf("Error in command: missing argument! Please check help.\r\n");	return -1;
	}

	app_main_test();
	canfd_status = msg_canfd_init();
	dataLog_init();
	app_main_initData(&farview_data);
	/* init timer for display */
	timer_display_ori = time(NULL);

	/* no CAN message is specified in the command, get the function selection */
    if(APP_OPT_UNKNOWN == opt.mode) {
		input_value = app_config();

		/* send CAN messages */
		if((0 == canfd_status) && (farview_data.dataCan->updated)) {
			ndPrintf("\n Sending data '%c' to CAN ...", input_value);
			msg_canfd_send_tester((uint32_t)input_value);
			ndPrintf("\n data '%c' to CAN sent!", input_value);
			farview_data.dataCan->updated = BOOL_FALSE;
		}
    }
    else
    /* start the timer to control the submission of vehicle CAN messages */
    {
		dPrintf("\nTotal msg #: %d", timer_num);
		ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
		iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

    	for(int i=0; i<timer_num; i++) {
			/* get the default interval and total number and display msg information */
#ifdef PROJECT_ID4
    		app_main_id4_getMsginfo(&msg[i]);
#endif
#ifdef PROJECT_C3
    		app_main_c3_getMsginfo(&msg[i]);
#endif
			/* use the command interval and total number */
			//msg[i].interval = opt.interval;
			msg[i].num = opt.num;
    	}

    	for(int i=0; i<timer_num; i++) {
    		start_timer(&msg, i, timer_num);
    	}

    	time_ori = time(NULL);
    }

	ndPrintf("\nStart the main loop... input value is %d", input_value);
	for(;;) {
		if((0 != opt.period) && (opt.period < (time(NULL) - time_ori))) {
			printf("\n");
			return 0;
		}

		if((COMMAND_C == input_value) || (COMMAND_F == input_value)) {
			/* just return */
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

#ifdef PROJECT_ID4
		if(COMMAND_P == input_value) {
			app_main_id4_commandP();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

		if((COMMAND_D == input_value) && (444 == dData.display)) {
			/* receive log data */
			app_main_id4_commandD_log();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}
#endif

		if((COMMAND_D == input_value) && (333 == dData.display)) {
			/* receive error data */
			app_main_id4_commandD_error();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

//		if(COMMAND_S == input_value) {
		if(APP_OPT_UNKNOWN != opt.mode) {
			static int timer_stopped = 0;
    		for(int i=0; i<timer_num; i++) {
    			/* search all the timer to check if any one completed the submission */
        		if( (msg[i].timer_count >= msg[i].num) && (0 != msg[i].num) ) {
        			if(BOOL_TRUE == msg[i].timer_mark) {
        				/* if this timer is not stopped, stop it */
        				stop_timer(&msg[i].timer_id);

        				/* mark it after it is stopped */
        				msg[i].timer_mark = BOOL_FALSE;

            			/* update the total number of the timer left */
            			timer_stopped++;
                		ndPrintf(" | stopped %d\n", timer_stopped);
        			}
        		}
    		}

    		if(timer_num == timer_stopped) {
    			/* all the submission is complete, return */
    			printf("\n");
    			return ret;
    		}
		}

		if(2 == input_value) {
			/* receive CAN messages */
			if(0 == canfd_status) {
				msg_canfd_receive();
			}

			if(1 < (time(NULL) - timer_display_ori)) {
#ifdef PROJECT_ID4
				app_main_id4_print();
#endif	/* #ifdef PROJECT_ID4 */
#ifdef PROJECT_NAVY
				msg_canfd_navy_print();
#endif	/* #ifdef PROJECT_NAVY */
				timer_display_ori = time(NULL);
			}
		}

		//debugPrintf("Cycle count: %d", i); if(70 == i++) for(;;) {;}
		if(60 == i++) dPrintf("\n"); // How does it affects the printing?
	}

	return ret;
}

#if 0
		switch(ret) {
			case 'l':
				opt.num = atoi(optarg);
				break;
			case 'i':
				opt.interval = atoi(optarg);
				break;
			case 'p':
				opt.period = atoi(optarg);
				break;
			case 'f':
				opt.mode = APP_OPT_DEV_SEND_FSH;
				opt.val = atoi(optarg);
				if(1 >= opt.val) {
					dData.display = opt.val + 1;
					dData.number = opt.val;
				}
				else {
					app_main_displayHelp(argv[0]);
				}
				ndPrintf("\r\nFSH Sts:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'a':
				opt.mode = APP_OPT_DEV_SEND_ATEMP;
				opt.val = atoi(optarg);
				dData.display = 3;
				dData.number = opt.val;
				ndPrintf("\r\nAmb.Temp.:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'v':
				opt.mode = APP_OPT_DEV_SEND_VOLTAGE;
				opt.val = atoi(optarg);
				ndPrintf("\r\nVoltage:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'c':
				opt.mode = APP_OPT_DEV_SEND_SOC;
				opt.val = atoi(optarg);
				ndPrintf("\r\nS.Charge:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 's':
				opt.mode = APP_OPT_DEV_SEND_SPEED;
				opt.val = atoi(optarg);
				ndPrintf("\r\nV. Speed:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 't':
				opt.mode = APP_OPT_DEV_SEND_CTEMP;
				opt.val = atoi(optarg);
				ndPrintf("\r\nCab Temp.:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'h':
				opt.mode = APP_OPT_DEV_SEND_HUMIDITY;
				opt.val = atoi(optarg);
				ndPrintf("\r\nHumidity:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
#ifdef PROJECT_C3
			case 'd':
				opt.mode = APP_OPT_DEV_SEND_SYSID;
				opt.val = atoi(optarg);
				ndPrintf("\r\nSystem ID:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'r':
				opt.mode = APP_OPT_DEV_SEND_CURRENT;
				opt.val = atoi(optarg);
				ndPrintf("\r\nCurrent:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'o':
				opt.mode = APP_OPT_DEV_SEND_OPMODE;
				opt.val = atoi(optarg);
				ndPrintf("\r\nOp.mode:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
#endif
			case 'H':
				app_main_displayHelp(argv[0]);
				return 0;
				//break;
			case '?':
				// unknown option or missing argument
				iPrintf("Unknown option or missing argument!\n");
				return 0;
				//break;
			default:							// it won't come here if there is no argument at all, why?
				opt.mode = APP_OPT_UNKNOWN;
				//return -1;
				break;
		}
#endif
