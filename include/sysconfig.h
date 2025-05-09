/*
 *
 *
 */
//#define DEVELOP_VERSION

/* project definition */
/* select only one from PROJECT_NAVY, PROJECT_C3 and PROJECT_ID4
 * if PROJECT_ID4 is selected, select only one from PROJECT_CAN_ID4, PROJECT_CAN_G4R and PROJECT_CAN_G3 */
//#define PROJECT_NAVY
//#define PROJECT_C3
//#define PROJECT_ID4
//#ifdef PROJECT_ID4				   /* ID4, G4R and G3 all are based on PROJECT_ID4, the differences are CAN messages */
//#define PROJECT_CAN_ID4
//#define PROJECT_CAN_G4R
//#define PROJECT_CAN_G3
//#endif

/* select project by a variable from command line argument */
#define PROJECT_ID_TOTAL		6
#define PROJECT_ID_ID4			0			/* project id4 */
#define PROJECT_ID_G3			1			/* project g3 */
#define PROJECT_ID_G4R			2			/* project g4r */
#define PROJECT_ID_C3			3			/* project c3 */
#define PROJECT_ID_NAVY			4			/* project navy */
#define PROJECT_ID_VOLVO		5

#define PROJECT_ARGU_ID4		"id4"		/* argument: id4 */
#define PROJECT_ARGU_G3			"g3"		/* argument: g3 */
#define PROJECT_ARGU_G4R		"g4r"		/* argument: g4r */
#define PROJECT_ARGU_C3			"c3"		/* argument: c3 */
#define PROJECT_ARGU_NAVY		"navy"		/* argument: navy */
#define PROJECT_ARGU_VOLVO		"volvo"

#define PROJECT_CANOPT_ID4		"cfavsth"	/* CAN message options */
#define PROJECT_CANOPT_C3		"cfavsthdro"/* CAN message options */
#define PROJECT_CANOPT_NAVY		""			/* CAN message options */
#define PROJECT_CANOPT_VOLVO	"bwngu"

#define PROJECT_FUNC_MT			'm'			/* manual test */
#define PROJECT_FUNC_AT			'z'			/* automatic test */
#define PROJECT_FUNC_CAN		'y'			/* CAN test */
#define PROJECT_FUNC_RC			'x'			/* remote control */

#define PROJECT_MT_FUNC1		'a'			/* Turn ON defrost */
#define PROJECT_MT_FUNC2		'b'			/* Turn OFF defrost */
#define PROJECT_MT_FUNC3		'c'			/* Set ambient temperature */
#define PROJECT_MT_FUNC4		'd'			/* Set vehicle speed */
#define PROJECT_MT_FUNC5		'e'			/* Set cabin temperature */
#define PROJECT_MT_FUNCr		'r'			/* Read the results from controller */
#define PROJECT_MT_FUNCq		'q'			/* Quit from current action */
#define PROJECT_MT_FUNCx		'x'			/* Exiting the program */

/* debug configurations */
#define MAIN_LOOP_DISPLAY_DOT           0           /* display only . */
#define MAIN_LOOP_DISPLAY_DEVICE        1           /* display device running status */
#define MAIN_LOOP_DISPLAY_INPUT         2           /* display all input values */
#define MAIN_LOOP_DISPLAY_STATUS        3           /* display system running status */
#define MAIN_LOOP_DISPLAY_CAN           11          /* display CAN incoming messages */

/* for CANFD communication */
#define COMMAND_P						'p'
#define COMMAND_C						'c'
#define COMMAND_F						'f'
#define COMMAND_W						'w'
#define COMMAND_D						'd'
#define COMMAND_D444					444
#define COMMAND_D333					333
#define COMMAND_D222					222
#define COMMAND_S						's'
#define COMMAND_X						'x'

/* For SPI-CAN devices */
#if 0
#define SPICAN_DEVICE_NODE				"/dev/spidev0.0"
#endif
#if 1
#define SPICAN_DEVICE_NODE				"/dev/spican"
#endif
#define SPI_BIT_RATE                    6000000     // 6M Hz, it affects how fast CAN msg gets processed
#define SPI_DEFAULT_BUFFER_LENGTH       96          // SPI buffer sizes
#define CANFD_MSG_LEN_ID4               8           // all messages are actually 8 bytes (save processing time)
//#define CAN_RX_MODE_INT                             // defined to use interrupt mode; otherwise polling mode

/* For data log */
//#define LOG_IN_FRAM
#ifndef LOG_IN_FRAM									// log data to FRAM for controller
#define LOG_IN_FILE									// log data to file for farview
#endif
#define FRAM_SIZE                   	0x20000     // one page 0x10000 bytes, totally two pages (0x10000 words)
#define FRAM_ADDR_P1                    0x00000     // page #1 start address
#define FRAM_ADDR_P2                    0x10000     // page #2 start address
#define LOG_BUFF_SIZE                   2700

/* for architecture */
#if __SIZEOF_POINTER__ == 8  			// 64-bit system
#define MEM_ALIGNMENT 8
#else  									// 32-bit system
#define MEM_ALIGNMENT 4
#endif
#define MEM_ALIGN_SIZE(size)            (((size) + MEM_ALIGNMENT - 1U) & (~(MEM_ALIGNMENT - 1U)))
#define NULL							((void *)0)
#define BOOL_INT32						int32_t
#define BOOL_INT16						int16_t
#define BOOL_INT8						int8_t
#define BOOL_TRUE						1
#define BOOL_FALSE						0
#define INVALID_INPUT               	(-10000)

