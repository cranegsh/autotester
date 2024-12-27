/*
 * canfdComm.h
 *
 *  Created on: Dec. 13, 2022
 *  Modified on Aug 15, 2023 to be ported to Raspberry Pi
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_H_
#define APPLICATION_CANFDCOMM_H_

#include <stdint.h>

#include "sysconfig.h"

#define PCAN_DEVICE                 PCAN_USBBUS2//PCAN_USBBUS1//

#define CAN_BUS_TYPE_CAN            0
#define CAN_BUS_TYPE_CANFD          1
#define CAN_BUS_TYPE                1//CAN_BUS_TYPE_CAN//FD
#define CANFD_BIT_RATE              "f_clock_mhz=80, nom_brp=2, nom_tseg1=63, nom_tseg2=16, nom_sjw=16, data_brp=2, data_tseg1=15, data_tseg2=4, data_sjw=4" //500K-2M, copied from PCAN Explorer config

#define CAN_IDMASK_SID				0x1FFC0000
#define CAN_IDMASK_EID				0x3FFFF
#define CAN_EID_BITS				18
#define MAX_DATA_BYTES 				64

#define CAN_VEH_MSG_LEN				8//64//		/* take the max number of all projects */
#define CAN_VEH_MSG_NUM				10			/* take the max number of all projects */

/* receive message ID for debugger control */
#define ID_RCV_DATA		       		0x201
#define ID_RCV_DATA_CFG	       		0x211
#define ID_RCV_DATA_CTL	       		0x221
#define ID_RCV_DATA_ENV	       		0x231
#define ID_RCV_DATA_RES	       		0x241
#define ID_RCV_ERROR				0x202
#define ID_RCV_LOG      			0x203

/* transmit message ID for debugger control*/
#define ID_SEND_DATA      			0x301	/* This message combines ID_SEND_CONFIG and IC_SEND_COMMAND
												and use a structure to prepare the data */
#define ID_SEND_COMMAND	    		0x311
#define ID_SEND_CONFIG				0x320
#define ID_SEND_CFG_PETD			0x321
#define ID_SEND_CFG_PWM				0x331

typedef enum {
    CAN_DLC_0,
    CAN_DLC_1,
    CAN_DLC_2,
    CAN_DLC_3,
    CAN_DLC_4,
    CAN_DLC_5,
    CAN_DLC_6,
    CAN_DLC_7,
    CAN_DLC_8,
    CAN_DLC_12,
    CAN_DLC_16,
    CAN_DLC_20,
    CAN_DLC_24,
    CAN_DLC_32,
    CAN_DLC_48,
    CAN_DLC_64
} CAN_DLC;

/* The sequence of below enumeration should be the same as the sequence of data in id4_vehData and ic3canDataInfo!!! */
typedef enum {
	APP_OPT_DEV_SEND_SOC,
	APP_OPT_DEV_SEND_FSH,
	APP_OPT_DEV_SEND_ATEMP,
	APP_OPT_DEV_SEND_VOLTAGE,
	APP_OPT_DEV_SEND_SPEED,
	APP_OPT_DEV_SEND_CTEMP,
	APP_OPT_DEV_SEND_HUMIDITY,
	APP_OPT_DEV_SEND_SYSID,
	APP_OPT_DEV_SEND_CURRENT,
	APP_OPT_DEV_SEND_OPMODE,
	APP_OPT_UNKNOWN
} msg_mode_t;

/*
 * struct canfdFilter {
    int16_t sid;
    int16_t sid_mask;
    int16_t sid11;
    int16_t sid11_mask;
    int32_t eid;
    int32_t eid_mask;
    BOOL_INT32 ide;
    BOOL_INT32 ide_check;
    CAN_FIFO_CHANNEL fifo_chan;
    CAN_FILTER filter_num;
};
struct canfdnode {
    char* spi_device;              // SPI device channel
    struct canfdFilter vwmsgFilter[FILTER_TOTAL];
};
*/
/* This is the data structure for vehicle data submission */
typedef struct {
	uint32_t mid;						/* message ID */
	char *name;							/* message name */
	uint32_t interval;					/* interval time in ms of continuous submission of this message, 0 is infinite */
	uint32_t num;						/* number of continuous submission of this message */
	int32_t value;						/* value of this message */
	uint8_t data[CAN_VEH_MSG_LEN];		/* data in frame of this message */
} canDataInfo_type;

//#define DEBUG_DISPLAY_CAN_MSG                       // defined to display CAN frames
#ifdef DEBUG_DISPLAY_CAN_MSG
#define debugPrintf_canfd(...)  debugPrintf(__VA_ARGS__);
#define dbgPrintf_canfd(...)  dPrintf(__VA_ARGS__);
#define dPrintf_canfd(...)  dPrintf(__VA_ARGS__);
#else
#define debugPrintf_canfd(...)  ndebugPrintf(__VA_ARGS__);
#define dbgPrintf_canfd(...)  ndPrintf(__VA_ARGS__);
#define dPrintf_canfd(...)  ndPrintf(__VA_ARGS__);
#endif

/* Functions to implement CANFD tasks */
void msg_canfd_send_veh(uint32_t prj_num, msg_mode_t msgno, int32_t value);
void msg_canfd_send_tester(uint32_t prj_num, uint32_t cmd);

int canfd_messageReceive(uint32_t *mid, uint8_t *data, uint32_t *num);
int canfd_messageSend(uint32_t mid, uint8_t *data, uint32_t num);
void msg_canfd_cleanData(uint32_t prj_num);
int msg_canfd_receive(uint32_t prj_num);
void msg_canfd_copyData(uint32_t number, uint32_t length, uint8_t *source, uint8_t *dest);

int msg_canfd_rcvCanConfigs(uint32_t prj_num);
int msg_canfd_rcvCanLog(uint32_t prj_num);

/* Function to initialize CAN FD device */
int msg_canfd_init(void);
void msg_canfd_deinit(void);

#endif /* APPLICATION_CANFDCOMM_H_ */
