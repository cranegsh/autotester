/*
 * canfdComm.h
 *
 *  Created on: Dec. 21, 2024
 *  Modified on Dec. 21, 2024 from app_canfd.h
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_ID4_H_
#define APPLICATION_CANFDCOMM_ID4_H_

#include "app_config.h"
#include "app_main.h"

/* These are the data from the vehicle through CAN message */
struct id4DataVeh_type {
    float voltage;
    float stateCharge;
    float speed;
    float tempInside;
    float tempOutside;
    float humidity;
    uint32_t fsh;
    uint32_t countMsg;
    BOOL_INT32 speedQbit;
    BOOL_INT32 tempOutsideQbit;
};

/* These are the data sent from tester */
#define CAN_DATA_OUT_LEN_ID4    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataOut_type)))
union dataOut_type {
    struct id4DataOut_type {
        long int freq;
        uint16_t deadband_red1;         // dead band rising edge delay for PWM1, in 10ns
        uint16_t deadband_fed1;         // dead band falling edge delay for PWM1, in 10ns
        uint16_t deadband_red2;         // dead band rising edge delay for PWM2, in 10ns
        uint16_t deadband_fed2;         // dead band falling edge delay for PWM2, in 10ns
        int16_t compensation_up3;       // compensation of rising edge for PWM3, in 10ns
        int16_t compensation_up4;       // compensation of rising edge for PWM4, in 10ns
        uint16_t compensation_lik;      // parameter for calculating compensation falling edge in pH (1000uH)
        uint16_t compensation_n;        // parameter for calculating compensation falling edge
        uint16_t compensation_rload;    // parameter for calculating compensation falling edge in mOhms
        int16_t modeControl;            /* true: close loop control; false: open loop control */
        uint16_t VoutTarget;            /* Desired target voltage in volts */
        uint16_t runtime;               /* Forced run time in seconds */
        int16_t thTempTransfo;          /* Transformer temperature check threshold in degree */
        uint16_t thReslow;              /* Windshield resistance check threshold low in mOhms */
        uint16_t thReshigh;             /* Windshield resistance check threshold high in mOhms */
        uint16_t thCout;                /* Output current protection check threshold in A */
        uint16_t checkHV;               /* true: check high voltage input; false: force high voltage input */
        uint16_t psTarget;              /* final phase shift for open loop control in degree */
        uint16_t addRuntime;            /* Forced extra run time in seconds based on calculated run time */
    } data;
    uint16_t word[CAN_DATA_OUT_LEN_ID4/2U];
    uint8_t byte[CAN_DATA_OUT_LEN_ID4];
};

#define CAN_DATA_OUT_COMMAND_LEN     (uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataOut_command_type)))
union dataOut_command_type {
    struct id4DataOut_command_type {
        uint16_t debugValue;
        uint16_t addrStart;
        uint16_t addrEnd;
        uint16_t dataNum;
    } data;
    uint16_t word[CAN_DATA_OUT_COMMAND_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_COMMAND_LEN];
};

#define CAN_DATA_OUT_CFG_PETD_LEN     (uint32_t)(MEM_ALIGN_SIZE(sizeof(struct petdConfig)))
union dataOut_cfgPetd_type {
	struct petdConfig data;
    uint16_t word[CAN_DATA_OUT_CFG_PETD_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_CFG_PETD_LEN];
};

#define CAN_DATA_OUT_CFG_PWM_LEN     (uint32_t)(MEM_ALIGN_SIZE(sizeof(struct pwmConfig)))
union dataOut_cfgPwm_type {
	struct pwmConfig data;
    uint16_t word[CAN_DATA_OUT_CFG_PWM_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_CFG_PWM_LEN];
};

/* These are the data sent out to tester from the controller */
#define CAN_DATA_IN_LEN_ID4    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataIn_type)))
union dataIn_type_id4 {
    struct id4DataIn_type {
        BOOL_INT16 updated;
        uint16_t mode;
        uint16_t hitCount;
        uint16_t vin;
        uint16_t vout;
        uint16_t cout;
        uint16_t res;
        int16_t tempTro;
        int16_t temp1;
        int16_t temp2;
        int16_t temp3;
        int16_t temp4;
        uint16_t petdRuntime;
        uint16_t pwmCompen;
        uint16_t veh_fsh;
        uint16_t veh_voltage;
        uint16_t veh_stateCharge;
        uint16_t veh_speed;
        uint16_t veh_tempInside;
        uint16_t veh_tempOutside;
        uint16_t veh_humidity;
    } data;
    uint16_t word[CAN_DATA_IN_LEN_ID4/2U];
    uint8_t byte[CAN_DATA_IN_LEN_ID4];
};

#define CAN_DATA_IN_CFG_LEN    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Cfg_type)))
union dataIn_Cfg_type {
    struct id4DataIn_Cfg_type {
        BOOL_INT16 updated;
        struct petdConfig petdCanConfig;
        struct pwmConfig pwmCanConfig;
    } data;
    uint16_t word[CAN_DATA_IN_CFG_LEN/2U];
    uint8_t byte[CAN_DATA_IN_CFG_LEN];
};

#define CAN_DATA_IN_CTL_LEN    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Ctl_type)))
union dataIn_Ctl_type {
    struct id4DataIn_Ctl_type {
        BOOL_INT16 updated;
        uint16_t mode;
        uint16_t hitCount;
        uint16_t veh_fsh;
    } data;
    uint16_t word[CAN_DATA_IN_CTL_LEN/2U];
    uint8_t byte[CAN_DATA_IN_CTL_LEN];
};

#define CAN_DATA_IN_ENV_LEN    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Env_type)))
union dataIn_Env_type {
    struct id4DataIn_Env_type {
        BOOL_INT16 updated;
        uint16_t vin;
        uint16_t veh_voltage;
        uint16_t veh_stateCharge;
        uint16_t veh_speed;
        uint16_t veh_tempInside;
        uint16_t veh_tempOutside;
        uint16_t veh_humidity;
    } data;
    uint16_t word[CAN_DATA_IN_ENV_LEN/2U];
    uint8_t byte[CAN_DATA_IN_ENV_LEN];
};

#define CAN_DATA_IN_RES_LEN    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Res_type)))
union dataIn_Res_type {
    struct id4DataIn_Res_type {
        BOOL_INT16 updated;
        int16_t tempTro;
        uint16_t vout;
        uint16_t cout;
        uint16_t res;
    } data;
    uint16_t word[CAN_DATA_IN_RES_LEN/2U];
    uint8_t byte[CAN_DATA_IN_RES_LEN];
};

/* These are the logged data sent out to tester */
#define CAN_DATA_LOG_LEN_ID4    			(uint32_t)(MEM_ALIGN_SIZE(sizeof(struct id4DataLog_type)))
union dataLog_type {
    struct id4DataLog_type {
        uint16_t value1;
        uint16_t value2;
        uint16_t value3;
        uint16_t value4;
        uint16_t address;
        uint16_t index;
    } data;
    uint16_t word[CAN_DATA_LOG_LEN_ID4/2U];
    uint8_t byte[CAN_DATA_LOG_LEN_ID4];
};

struct canfdData_id4 {
	struct id4DataVeh_type id4Dataveh;
	canDataInfo_type id4_vehData[CAN_VEH_MSG_NUM];
	union dataIn_type_id4 id4DataIn;
	union dataIn_Cfg_type id4DataIn_Cfg;
	union dataIn_Ctl_type id4DataIn_Ctl;
	union dataIn_Env_type id4DataIn_Env;
	union dataIn_Res_type id4DataIn_Res;
	union dataLog_type id4DataLog;
	union dataOut_type id4DataOut;
	union dataOut_command_type id4DataOut_command;
	union dataOut_cfgPetd_type id4DataOut_cfgPetd;
	union dataOut_cfgPwm_type id4DataOut_cfgPwm;
    BOOL_INT32 updated;
};

/* Function to pass data pointer */
struct canfdData_id4 *msg_canfd_getData_id4(void);
uint32_t msg_canfd_getMid_id4(uint32_t number);
uint32_t msg_canfd_getMid_g3(uint32_t number);
uint32_t msg_canfd_getMid_g4r(uint32_t number);
int msg_canfd_prepare_id4Veh(msg_mode_t msgno, int32_t value, uint8_t *data);
int msg_canfd_prepare_g3Veh(msg_mode_t msgno, int32_t value, uint8_t *data);
//int msg_canfd_prepare_g4rVeh(msg_mode_t msgno, int32_t value, uint8_t *data);		/* same as g3 now */
int32_t msg_canfd_prepare_id4(uint32_t option, uint32_t *mid, uint8_t *data, uint32_t *num);
BOOL_INT32 msg_canfd_interpret_id4Veh(uint32_t mid, uint8_t *data, uint32_t num);
void msg_canfd_interpret_id4(uint32_t mid, uint8_t *data, uint32_t num);
void msg_canfd_clear_id4(void);

void app_main_id4_sendCommand(sysData_type *sdata, int cmd);
void app_main_id4_getMsgvalue(msg_opt_t *msgi);
void app_main_g3_getMsgvalue(msg_opt_t *msgi);
void app_main_g4r_getMsgvalue(msg_opt_t *msgi);
void app_main_id4_getMsginfo(msg_opt_t *msgi);
void app_main_g3_getMsginfo(msg_opt_t *msgi);
void app_main_g4r_getMsginfo(msg_opt_t *msgi);
void app_main_displayMsg_g3(msg_opt_t *msgi);

void app_main_id4_print(void);
void app_main_id4_print_canVeh(uint32_t func, int msgNum, int msgCount);
void app_main_g3_print_canVeh(uint32_t func, int msgNum, int msgCount);
void app_main_g4r_print_canVeh(uint32_t func, int msgNum, int msgCount);
int app_main_id4_commandP(void);
void app_main_id4_commandD_log(void);
int app_main_id4_commandD_error(void);

#endif /* APPLICATION_CANFDCOMM_ID4_H_ */
