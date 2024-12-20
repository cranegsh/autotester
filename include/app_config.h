/*
 * configSet.h
 *
 *  Created on: Aug. 23, 2022
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CONFIGSET_H_
#define APPLICATION_CONFIGSET_H_

#include <stdint.h>
#include "sysconfig.h"

/* Power configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
struct powerConfig {
	uint16_t mode;
	int16_t tempOff;                /* Windshield temperature to turn off AC switch */
	int16_t tempOn;                 /* Windshield temperature to turn on AC switch */
	uint16_t thVoltagePeak;         /* Voltage input threshold for error */
	uint16_t thVoltageRms;          /* Voltage input threshold for error */
	uint16_t thCurrentPeak;         /* Current input threshold for error */
	uint16_t thCurrentRms;          /* Current input threshold for error */
	uint16_t thRes;                 /* Windshield resistance threshold for error */
    BOOL_INT16 checkRes;
    BOOL_INT16 checkVoltage;
    BOOL_INT16 checkCurrent;
    BOOL_INT16 configUpdated;
};

/* PWM configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
#define PWM_CONFIG_LEN				MEM_ALIGN_SIZE(sizeof(struct pwmConfig))
struct pwmConfig {
	BOOL_INT16 configUpdated;		/* C2000 bool size is 2 bytes. So using 16-bit is a must to match */
	uint32_t freq;
    uint16_t deadband_red1;         // dead band rising edge delay for PWM1, in 10ns
    uint16_t deadband_fed1;         // dead band falling edge delay for PWM1, in 10ns
    uint16_t deadband_red2;         // dead band rising edge delay for PWM2, in 10ns
    uint16_t deadband_fed2;         // dead band falling edge delay for PWM2, in 10ns
    int16_t compensation_up3;       // compensation of rising edge for PWM3, in 10ns
//    int16_t compensation_down3;     // compensation of falling edge for PWM3, in 10ns
    int16_t compensation_up4;       // compensation of rising edge for PWM4, in 10ns
//    int16_t compensation_down4;     // compensation of falling edge for PWM4, in 10ns
    uint16_t compensation_lik;      // parameter for calculating compensation falling edge in pH (1000uH)
    uint16_t compensation_n;        // parameter for calculating compensation falling edge
    uint16_t compensation_rload;    // parameter for calculating compensation falling edge in mOhms
};

/* PETD configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
#define PETD_CONFIG_LEN				MEM_ALIGN_SIZE(sizeof(struct petdConfig))
struct petdConfig {
	BOOL_INT16 configUpdated;		/* add 16-bit to be aligned with the sender's structure */
    int16_t VoutTarget;             /* Desired target voltage in volts */
    int16_t runtime;                /* Forced run time in seconds */
    int16_t thTempTransfo;          /* Transformer temperature check threshold in degree */
    int16_t thReslow;               /* Windshield resistance check threshold low in mOhms */
    int16_t thReshigh;              /* Windshield resistance check threshold high in mOhms */
    int16_t thCout;                 /* Output current protection check threshold in A */
    BOOL_INT16 forceRuntime;        /* true: force run time; false: compute run time */
    BOOL_INT16 checkTempransfo;     /* true: check transformer temperature; false: force transformer temperature */
    int16_t checkHV;                /* true: check high voltage input; false: force high voltage input */
    BOOL_INT16 checkRes;            /* true: check windshield resistance; false: force windshield resistance */
    BOOL_INT16 checkCout;           /* true: check output current; false: force output current */
    int16_t modeControl;            /* true: close loop control; false: open loop control */
    int16_t psTarget;               /* final phase shift for open loop control in degree */
    uint16_t addRuntime;            /* Forced extra run time in seconds based on calculated run time */
} __attribute__ ((aligned (8)));

int app_config();

void writeLogging(void);
void printLogging(void);

/* Functions to config power control - Navy */
struct powerConfig *powerConfig_get(void);
void powerConfig_updateCan(void);
void powerConfig_print(void);
BOOL_INT32 powerConfig_check(struct powerConfig *configIn, struct powerConfig *configOut, BOOL_INT32 ignore);

/* Functions to config PETD */
void petdConfig_print(void);
struct petdConfig *petdConfig_get(void);
BOOL_INT32 petdConfig_check(struct petdConfig *configIn, struct petdConfig *configOut, BOOL_INT32 ignore);

/* Functions to config PWM */
void pwmConfig_print(void);
struct pwmConfig* pwmConfig_get(void);
BOOL_INT32 pwmConfig_check(struct pwmConfig *configIn, struct pwmConfig *configOut, BOOL_INT32 ignore);

#endif /* APPLICATION_CONFIGSET_H_ */
