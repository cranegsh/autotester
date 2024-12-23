/*
 * canfdComm.h
 *
 *  Created on: Dec. 21, 2024
 *  Modified on Dec. 21, 2024 from app_canfd.h
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_NAVY_H_
#define APPLICATION_CANFDCOMM_NAVY_H_

#include "app_canfd.h"
#include "app_main.h"

#define CAN_DATA_OUT_LEN_NAVY     			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct navyDataOut))) //8//

union dataOut {
    struct navyDataOut {
        uint16_t mode;
        int16_t tempOff;
        int16_t tempOn;
        uint16_t thRes;
        uint16_t thVoltagePeak;
        uint16_t thVoltageRms;
        uint16_t thCurrentPeak;
        uint16_t thCurrentRms;
    } data;
    uint16_t word[CAN_DATA_OUT_LEN_NAVY/2];
    uint8_t byte[CAN_DATA_OUT_LEN_NAVY];
};

#define CAN_DATA_IN_LEN_NAVY    				(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct navyDataIn))) //16
union dataIn {
    struct navyDataIn {
        uint16_t mode;
        int16_t state;
        int16_t tempOff;
        uint16_t thRes;
        uint16_t thVoltage;
        uint16_t thCurrent;
        int16_t temp01;
        int16_t voltagePeak;
        uint16_t voltageRms;
        int16_t currentPeak;
        uint16_t currentRms;
        uint16_t resWindshield; 
    } data;
    uint16_t word[CAN_DATA_IN_LEN_NAVY/2];
    uint8_t byte[CAN_DATA_IN_LEN_NAVY];
};

#define CAN_DATA_LOG_LEN_NAVY    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct navyDataLog)))
union dataLog {
    struct navyDataLog {
        BOOL_INT16 updated;
        uint16_t count;
        int16_t voltagePeak;
        uint16_t voltageRms;
        int16_t currentPeak;
        uint16_t currentRms;
        uint16_t resWindshield;
        int16_t temp01;
        int16_t temp02;
        int16_t temp03;
    } data;
    uint16_t word[(CAN_DATA_LOG_LEN_NAVY)/2];
    uint8_t byte[CAN_DATA_LOG_LEN_NAVY];
};

struct canfdData_navy {
    union dataIn navyIn;
    union dataLog navyLog;
    union dataOut navyOut;
    BOOL_INT32 updated;
};

/* Function to pass data pointer */
struct canfdData_navy *msg_canfd_getData_navy(void);
int32_t msg_canfd_prepare_navy(uint32_t option, uint32_t *mid, uint8_t *data, uint32_t *num);
void msg_canfd_interpret_navy(uint32_t mid, uint8_t *data, uint32_t num);
void msg_canfd_clear_navy(void);

void app_main_navy_print(void);

#endif /* APPLICATION_CANFDCOMM_NAVY_H_ */
