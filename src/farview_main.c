#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>

#include "sysconfig.h"
#include "utility.h"
#include "app_canfd.h"
#include "app_config.h"
#include "app_log.h"
#include "app_timer.h"
#include "app_main.h"

static sysData_type farview_data = {
	.project_id = PROJECT_ID_C3,
	.project_func = PROJECT_FUNC_DEF,				/* should be modified according to command */
	.msg_num = 0,
	.canfd_status = -1,
	.update_default = 0,
};

/**************************************************************************************
 * 						main program
 **************************************************************************************/
int main(int argc, char *argv[]) {
	int i, ret = 0, status = 0;
    int input_command = 0;

	app_opt_t opt;
	opt.val = 0;
	opt.num = 1;            /* default loop num when repeating messages if not set in command line */
	opt.interval = 0xFFFF;	/* default interval in ms when repeating messages; not useful any more */
	opt.period = 0;
	opt.option = 0xFF;
	opt.mode = APP_OPT_UNKNOWN;
	opt.function = 0;

	/* get project number from command argument */
	if(optind < argc) {
		for(i=0; i<PROJECT_ID_TOTAL; i++) {
			ndPrintf("Compare %s to %s\n", project_name[i], argv[optind]);
			if(0 == strcmp(project_name[i], argv[optind])) {
				/* get the valid argument */
				farview_data.project_id = (uint32_t)i;
				break;
			}
		}
		if((PROJECT_ID_TOTAL == i) && (strcmp("-H", argv[optind]))){
			/* no valid argument */
			iPrintf("Wrong argument %s in command! Please check help.\n", argv[optind]);
			return -1;
		}
	}
	else {
		iPrintf("Missing argument in command! Please check help.\n");
		return -1;
	}

	/* get the option and parameters if needed (followed with :) */
	//dPrintf("\nGetting %d arguments and the option is %d\n", argc, ret);
	while(-1 != (ret = getopt(argc, argv, "Hmzyl:i:p:f:a:v:c:s:t:h:w:d:r:o:"))) {
		ndPrintf("\nGet %d arguments and the option is %c\n", argc, ret);
		status = app_main_parseOption(&farview_data, ret, optarg, &opt);
		ndPrintf("Opt status: %c - %d\n", ret, status);
		if (-1 == status) {
			/* ask for help or input an out-of-range value */
			app_main_displayHelp(argv[0]);
			return -1;
		}
		else if (0 == status) {
			/* set parameters for CAN test: update the number of the CAN messages from command options */
		    if(PROJECT_FUNC_DEF == farview_data.project_func) {
		        /* get total message number from command options */
		        farview_data.msg_num++;
		    }
		    else {
		        /* update message value(s) from command line */
		        farview_data.update_default = 1;
		    }
		}
		else if (2 == status) {
			farview_data.project_func = opt.function;
			/* set parameters for manual test or automatic test or CAN test */
			farview_data.msg_num = (uint32_t)app_main_getMsgnum(&farview_data);
			/* adjust total message number */
			if((PROJECT_ID_ID4 == farview_data.project_id) || (PROJECT_ID_C3 == farview_data.project_id)) {
			    /* decrease one due to two values in one message */
			    farview_data.msg_num--;
			}
			else if(PROJECT_ID_VOLVO == farview_data.project_id) {
			    /* decrease one due to two values in one message */
                farview_data.msg_num--;
            }
		}
		else if (3 == status) {
	    	//farview_data.project_func = PROJECT_FUNC_RC;
		}
	}

	/* init the system */
	app_main_test();
	if(0 != msg_canfd_init(farview_data.project_id)) {
		iPrintf("CAN not available!\n\n");
		//return -1;
	}
	dataLog_init();
	if((PROJECT_FUNC_CAN == farview_data.project_func) && (0 < farview_data.update_default)) {
	    /* Set messages' value from command line for "CAN test" function */
	    app_main_initData(&farview_data);
	}
	iPrintf("Project %s - Function %c - opt.mode %d - msg_num %d\n",
			project_name[farview_data.project_id], farview_data.project_func, opt.mode, farview_data.msg_num);
	app_main_testApp();


	/* set up system: branch according to the command options */
	if(PROJECT_FUNC_MT == opt.function) {
		/* run manual test */
		ndPrintf("Start running manual test ...\n");
		app_main_manualTest(&farview_data);
		return 0;
	}
	else if(PROJECT_FUNC_AT == opt.function) {
		/* run automatic test */
		dPrintf("Start running auto test ...\n");
		input_command = PROJECT_FUNC_AT;
		app_main_autoTest(&farview_data);
		return 0;
	}
	else if((APP_OPT_UNKNOWN != opt.mode) || (PROJECT_FUNC_CAN == opt.function)) {
    	/* do CAN test by submitting vehicle CAN messages specified by opt.mode */
		status = app_main_canTest(&farview_data, &opt);
		if(0 == status) {
			return 0;
		}
    }


    /* run remote control */
	input_command = app_config_main(farview_data.project_id);
	if(-1 == input_command) {
		return 0;
	}
	/* send CAN messages according to the input from console */
	app_main_sendCommand(&farview_data, input_command);
    /* start the main loop to display the control status like UART console */
	ndPrintf("\nStart the main loop... input value is %d", input_command);
	for(;;) {
		/* in remote control mode: process remote control command from console */
		status = app_main_remoteControl(&farview_data, input_command);
		if(0 == status) {
			iPrintf("\n");
			return 0;
		}
#if 0	/* not working */
		/* exit the while loop after pressing 'x' and ENTER */
		if('x' == get_a_char_nb) {
			dPrintf("Exiting the program ...\n");
			break;
		}
#endif
		//debugPrintf("Cycle count: %d", i); if(70 == i++) for(;;) {;}
		if(60 == i++) dPrintf("\n"); // How does it affects the printing?
	}

	return ret;
}
