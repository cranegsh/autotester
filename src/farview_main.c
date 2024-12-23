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

/* For identifying the project */
uint32_t idProject;

sysData_type farview_data = {
	.project_id = 0,
	.canfd_status = -1,
};


/**************************************************************************************
 * 						main program
 **************************************************************************************/
int main(int argc, char *argv[]) {
	int i, ret = 0, status = 0;
    int input_command = 0;

	app_opt_t opt;
	opt.val = 0;
	opt.num = 0;
	opt.interval = 1000;	/* default interval in ms when repeating messages; not useful any more as each message has its own */
	opt.period = 0;
	opt.option = 0xFF;
	opt.mode = APP_OPT_UNKNOWN;

	/* get the option and parameters if needed (followed with :) */
	//dPrintf("\r\nGetting %d arguments and the option is %d\n", argc, ret);
	while(-1 != (ret = getopt(argc, argv, "mzl:i:p:f:a:v:c:s:t:h:d:r:o:H"))) {
		ndPrintf("\r\nGet %d arguments and the option is %c\n", argc, ret);
		status = app_main_processOption(ret, &opt, optarg);
		if (-1 == status) {
			app_main_displayHelp(argv[0]);
			return -1;
		}
	}

	/* get the argument */
	if(optind < argc) {
		if(0 == strcmp(PROJECT_ARGU_ID4, argv[optind])) {
			idProject = PROJECT_ID_ID4;
		}
		else if(0 == strcmp(PROJECT_ARGU_G3, argv[optind])) {
			idProject = PROJECT_ID_G3;
		}
		else if(0 == strcmp(PROJECT_ARGU_G4R, argv[optind])) {
			idProject = PROJECT_ID_G4R;
		}
		else if(0 == strcmp(PROJECT_ARGU_C3, argv[optind])) {
			idProject = PROJECT_ID_C3;
		}
		else if(0 == strcmp(PROJECT_ARGU_NAVY, argv[optind])) {
			idProject = PROJECT_ID_NAVY;
		}
		else {
			printf("\r\nError in command: wrong argument! Please check help.\r\n");
			return -1;
		}
		ndPrintf("\r\nNon-option argument:%s %d\r\n", argv[optind], idProject);
	}
	else {
		idProject = PROJECT_ID_DEFAULT;
		//printf("Error in command: missing argument! Please check help.\r\n");	return -1;
	}
	ndPrintf("Project IS is %d\r\n", idProject);

	/* init the system */
	app_main_test();
	farview_data.project_id = idProject;
	farview_data.canfd_status = msg_canfd_init();
	dataLog_init();
	app_main_initData(&farview_data);

	/* set up system: branch according to the command options */
    if(APP_OPT_UNKNOWN != opt.mode) {
    	/* do CAN test by submitting vehicle CAN messages specified by opt.mode */
		app_main_initMsg(idProject, &opt);
		status = app_main_canTest(&opt);
		if(0 == status) {
			printf("\r\n");
			return 0;
		}
    }
    else
    {  	/* get the function selection */
    	if('m' == opt.function) {
    		/* run manual test */
    		input_command = 'm';
    		dPrintf("Start running manual test ...\r\n");
    		return 0;
    	}
    	else if('z' == opt.function) {
    		/* run automatic test */
    		input_command = 'z';
    		dPrintf("Start running auto test ...\r\n");
    		return 0;
    	}
    }

    /* run remote control */
	input_command = app_config_main(idProject);
	/* send CAN messages according to the input from console */
	app_main_sendCommand(idProject, &farview_data, input_command);
    /* start the main loop */
	ndPrintf("\nStart the main loop... input value is %d", input_command);
	for(;;) {
		/* in remote control mode: process remote control command from console */
		status = app_main_remoteControl(idProject, input_command, &farview_data);
		if(0 == status) {
			printf("\r\n");
			return 0;
		}

		//debugPrintf("Cycle count: %d", i); if(70 == i++) for(;;) {;}
		if(60 == i++) dPrintf("\n"); // How does it affects the printing?
	}

	return ret;
}
