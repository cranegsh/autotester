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

#define CAN_DATA_IN_LEN_C3    				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct C3DataVeh_type)))
typedef union dataIn_type {
    struct C3DataVeh_type {
        uint16_t voltage;
        uint16_t stateCharge;
        uint16_t speed;
        int16_t tempInside;
        int16_t tempOutside;
        uint16_t humidity;
        uint16_t current;
        uint16_t fsh;
        uint16_t countMsg;
        uint16_t systemId;
        uint16_t opMode;
    } data;
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
int32_t msg_canfd_prepare_c3Veh(msg_mode_t msgno, int32_t value, uint8_t *data);
void msg_canfd_clear_c3(void);

void app_main_c3_sendCommand(sysData_type *sdata, int cmd);
void app_main_c3_getMsginfo(msg_opt_t *msgi);
void app_main_c3_print(void);
void app_main_c3_print_canVeh(uint32_t func, int msgNum, int msgCount);

#endif /* APPLICATION_CANFDCOMM_C3_H_ */
