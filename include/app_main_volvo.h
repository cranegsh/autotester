/*
 * canfdComm.h
 *
 *  Created on: Dec. 21, 2024
 *  Modified on Dec. 21, 2024 from app_canfd.h
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_VOLVO_H_
#define APPLICATION_CANFDCOMM_VOLVO_H_

#include "app_canfd.h"
#include "app_main.h"

#define MSGNO_OFFSET							10

#define SECONDS_BIT_START			    		0
#define MINUTES_BIT_START			    		8
#define HOURS_BIT_START			    			16
#define MONTHS_BIT_START						24
#define DAYS_BIT_START							32
#define YEARS_BIT_START							40

#define SECONDS									15
#define MINUTES									30
#define HOURS									12
#define MONTHS									5
#define DAYS									8
#define YEARS									2025

/* These are data sent by tester to controller to conduct test
 * Actually defined in c3canDataInfo!
 * */
#define CAN_DATA_OUT_LEN_VOLVO  				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct VolvoDataVeh_type)))
typedef union {
    struct VolvoDataVeh_type {
    	uint16_t ambientAirTemp;
    	uint16_t wheelBasedVehicleSpeed;
    	uint8_t powerTrainDrivelineStatus;
    	uint16_t highVoltage;
    	uint64_t time;
    } data __attribute__((aligned(MEM_ALIGNMENT)));
    uint16_t word[CAN_DATA_OUT_LEN_VOLVO/2U];
    uint8_t byte[CAN_DATA_OUT_LEN_VOLVO];
} dataOut_type_volvo;

/* These are the data received by tester from controller to check */
#define CAN_DATA_IN_LEN_VOLVO    				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct VolvoDataIn_type)))
typedef union {
    struct VolvoDataIn_type {
        float resistance;					/* Ohms */
        float petdRuntime;					/* s */
        uint16_t petdCommand;				/* 1: power engaged; 0: power not engaged */
        uint16_t errorCode;
        uint16_t errorValue;
    } data __attribute__((aligned(MEM_ALIGNMENT)));
    uint16_t word[CAN_DATA_IN_LEN_VOLVO/2U];
    uint8_t byte[CAN_DATA_IN_LEN_VOLVO];
} dataIn_type_volvo;

struct canfdData_Volvo {
	canDataInfo_type volvocanDataInfo[CAN_VEH_MSG_NUM];
	dataIn_type_volvo volvodataIn;
    BOOL_INT32 updated;
};

/* Function to pass data pointer */
struct canfdData_Volvo *msg_canfd_getData_Volvo(void);
uint32_t msg_canfd_getMid_Volvo(uint32_t number);
int msg_canfd_prepare_VolvoVeh(msg_mode_t msgno, int64_t value, uint8_t *data);
void msg_canfd_clear_Volvo(void);
void msg_canfd_interpret_Volvo(uint32_t mid, uint8_t *data, uint32_t num);

void app_main_Volvo_sendCommand(sysData_type *sdata, int cmd);
void app_main_Volvo_getMsgvalue(msg_opt_t *msgi);
void app_main_Volvo_getMsginfo(msg_opt_t *msgi);
void app_main_displayMsg_Volvo(msg_opt_t *msgi);
void app_main_Volvo_print(void);
void app_main_Volvo_print_canVeh(uint32_t func, int msgNum, int msgCount);
void app_main_displayResult_Volvo(void);
int app_main_checkResult_Volvo(void);

#endif /* APPLICATION_CANFDCOMM_C3_H_ */
