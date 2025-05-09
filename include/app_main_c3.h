/*
 * canfdComm.h
 *
 *  Created on: Dec. 21, 2024
 *  Modified on Dec. 21, 2024 from app_canfd.h
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_C3_H_
#define APPLICATION_CANFDCOMM_C3_H_

#include "app_canfd.h"
#include "app_main.h"

/* These are data sent by tester to controller to conduct test
 * Actually defined in c3canDataInfo!
 * */
#define CAN_DATA_OUT_LEN_C3    				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct C3DataVeh_type)))
typedef union {
    struct C3DataVeh_type {
        uint16_t stateCharge;
        uint16_t fsh;
        int16_t tempOutside;
        uint16_t voltage;
        uint16_t speed;
        int16_t tempInside;
        uint16_t humidity;
        uint16_t systemId;
        uint16_t current;
        uint16_t opMode;
    } data __attribute__((aligned(MEM_ALIGNMENT)));
    uint16_t word[CAN_DATA_OUT_LEN_C3/2U];
    uint8_t byte[CAN_DATA_OUT_LEN_C3];
} dataOut_type_c3;

/* These are the data received by tester from controller to check */
#define CAN_DATA_IN_LEN_C3    				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct c3DataIn_type)))
typedef union {
    struct c3DataIn_type {
        float resistance;					/* Ohms */
        float petdRuntime;					/* s */
        uint16_t petdCommand;				/* 1: power engaged; 0: power not engaged */
        uint16_t errorCode;
        uint16_t errorValue;
    } data __attribute__((aligned(MEM_ALIGNMENT)));
    uint16_t word[CAN_DATA_IN_LEN_C3/2U];
    uint8_t byte[CAN_DATA_IN_LEN_C3];
} dataIn_type_c3;

struct canfdData_c3 {
	canDataInfo_type c3canDataInfo[CAN_VEH_MSG_NUM];
	dataIn_type_c3 c3dataIn;
    BOOL_INT32 updated;
};

/* Function to pass data pointer */
struct canfdData_c3 *msg_canfd_getData_c3(void);
uint32_t msg_canfd_getMid_c3(uint32_t number);
int msg_canfd_prepare_c3Veh(msg_mode_t msgno, int64_t value, uint8_t *data);
void msg_canfd_clear_c3(void);
void msg_canfd_interpret_c3(uint32_t mid, uint8_t *data, uint32_t num);

void app_main_c3_sendCommand(sysData_type *sdata, int cmd);
void app_main_c3_getMsgvalue(msg_opt_t *msgi);
void app_main_c3_getMsginfo(msg_opt_t *msgi);
void app_main_displayMsg_c3(msg_opt_t *msgi);
void app_main_c3_print(void);
void app_main_c3_print_canVeh(uint32_t func, int msgNum, int msgCount);
void app_main_displayResult_c3(void);
int app_main_checkResult_c3(void);

#endif /* APPLICATION_CANFDCOMM_C3_H_ */
