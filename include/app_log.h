/*
 * dataLog.h
 *
 *  Created on: Apr. 8, 2023
 *      Author: Crane Shao
 */

#ifndef APPLICATION_DATALOG_H_1_
#define APPLICATION_DATALOG_H_1_

#include <stdint.h>

#include "sysconfig.h"
#include "app_config.h"

#define LOG_ERROR_SIZE                  (uint32_t)(MEM_ALIGN_SIZE(sizeof(struct errorInfo))) /* number of 16bit words for one error info */
#define LOG_ERROR_NUMBER                10//0               /* number of error info to be saved */
//#define DATA_LOG_BYTE
//#define DATA_LOG_TIME

/* parameters to log when error occurs */
struct errorParameters {
	BOOL_INT8 modeCtrl;
	BOOL_INT8 modeOp;
    uint16_t voutSet;
    uint16_t tempAmbient;
    uint16_t runtime;           /* in 10ms */
    uint16_t vin;
    uint16_t vout;
    uint16_t cout;
    uint16_t res;
    int16_t tempTransfo;
    int16_t tempT1;
    int16_t tempT2;
    int16_t tempT3;
    int16_t tempT4;
};

/* error information to log when error occurs */
struct errorInfo {
    uint16_t errno;
    uint16_t runningtime;       /* in 10ms */
    struct errorParameters logParameters;
} __attribute__ ((aligned (MEM_ALIGNMENT)));

union errInfoUnion {
    struct errorInfo logerrorInfo;
    uint16_t word_16bits[LOG_ERROR_SIZE];
};

struct errInfoLog {
    union errInfoUnion errorLog;
    uint32_t errorCount;            /* total error number (0 - ...) */
    BOOL_INT32 writeEnabled;
    uint32_t logPosi;               /* log position (0-9) in error log space of LOG_ERROR_NUMBER */
};

/* Function to write data to FRAM */
void dataLogfram_write(void);

/* Functions to operate log buff: write and read/print */
void dataLog_buff(uint16_t *data);
void dataLogbuff_enable();
void dataLogbuff_disable();
void dataLogbuff_clearfilled();
BOOL_INT32 dataLogbuff_print_word(uint32_t addr, uint32_t num);
BOOL_INT32 dataLogbuff_print_byte(uint32_t addr, uint32_t num);
uint16_t dataLogbuff_read(void);

/* Functions to log configs and error info to FRAM
 * and read log data from FRAM*/
BOOL_INT32 dataLogfram_configsWrite(void);
BOOL_INT32 dataLogfram_configDisplay_id4(void);
void dataLogfram_writeEnable_configs(void);
struct errInfoLog *logBuf_error_get(void);
BOOL_INT32 dataLogfram_errorWrite(void);
BOOL_INT32 dataLogfram_errorDisplay(void);
void dataLogfram_writeEnable_error(void);

/* Functions to read log buff and write to FRAM or vice versa */
void dataLogfram_writeEnable_buff(void);
BOOL_INT32 dataLogfram_write2buff_word(void);
BOOL_INT32 dataLogfram_readfmbuff_word(uint32_t addr, uint32_t num_No);
BOOL_INT32 dataLogfram_readfmbuff_wordn(uint32_t addr, uint32_t num_No);

/* Functions to operate FRAM: write and read/print */
BOOL_INT32 dataLogger_writeByte(uint32_t addr, uint32_t num, uint16_t *data);
BOOL_INT32 dataLogger_writeWord(uint32_t addr, uint32_t num, uint16_t *data);
BOOL_INT32 dataLogger_readWord(uint32_t addr, uint32_t num, uint16_t *data);
BOOL_INT32 dataLogger_print_wordn(uint32_t addr, uint32_t num);
BOOL_INT32 dataLogger_print_word(uint32_t addr, uint32_t nums);
BOOL_INT32 dataLogger_print_byte(uint32_t addr, uint32_t num);

void dataLog_init(void);

#endif /* APPLICATION_DATALOG_H_1_ */
