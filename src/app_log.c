/*
 * dataLog.c
 *
 *  Created on: Apr. 8, 2023
 *      Author: Crane Shao
 */
#include <stdint.h>

#include "app_log.h"
#include "utility.h"
#include "sysconfig.h"
#include "app_file.h"

/***************************************
 * FRAM map in bytes:
 * Page 1: (0x00000 ~ 0x0FFFF) (FRAM_ADDR_P1 ~ FRAM_ADDR_P1 + 0x0FFFF)
 *  logBuff
 * Page 2: (0x10000 ~ 0x1FFFF) (FRAM_ADDR_P2 ~ FRAM_ADDR_P1 + 0x0FFFF)
 *  config mark (0x5555)                2
 *  configs size                        2
 *  configs data                        LOG_CONFIGS_SIZE * 2
 *  error mark (0xAAAA)                 2
 *  error size                          2
 *  error count                         2
 *  error data #1                       LOG_ERROR_SIZE * 2
 *  error data #1                       LOG_ERROR_SIZE * 2
 *  .............                       LOG_ERROR_SIZE * 2
 *  error data #LOG_ERROR_NUMBER        LOG_ERROR_SIZE * 2
 *  boundary check: 4 + LOG_CONFIGS_SIZE*2 + 6 + LOG_ERROR_SIZE*2 * LOG_ERROR_NUMBER < 0x0FFFF
 */
/*****************************************
 * File map in bytes:
 * Block 1: configs
 * Block 2: errors
 * Block 3: data
 */
#define LOG_CONFIGS_SIZE                (sizeof(struct petdConfig) + sizeof(struct pwmConfig)) /* number of 16bit words for configuration */

/* Address on FRAM page 2 in bytes */
#define LOG_ADDR_CONFIGS                FRAM_ADDR_P2
#define LOG_ADDR_ERRORINFO              (FRAM_ADDR_P2 + LOG_CONFIGS_SIZE*2 + 4)        /* 4 bytes for mark and count */

/* Log marks */
#define LOG_MARK_CONFIGS                0x5555
#define LOG_MARK_ERRORINFO              0xAAAA

struct sysConfigs {
    struct petdConfig petdlogConfig;
    struct pwmConfig pwmlogConfig;
};

/* configurations to log */
union sysConfigsUnion {
    struct sysConfigs logsysConfigs;
    uint16_t word_16bit[LOG_CONFIGS_SIZE];
};

struct sysConfigsLog {
    union sysConfigsUnion configsLog;
    uint16_t logCount;
    BOOL_INT32 writeEnabled;
};

/* data to log for debugging */
struct debugLog {
	BOOL_INT32 writeEnabled;
	BOOL_INT32 buffUpdated;
    uint16_t logCount;
    uint16_t buff_value[LOG_BUFF_SIZE];
#ifdef DATA_LOG_TIME
    uint16_t buff_value1[LOG_BUFF_SIZE];
#endif
    uint16_t buff_value2[LOG_BUFF_SIZE];
    uint16_t buff_value3[LOG_BUFF_SIZE];
};

struct dataLogger {
#ifdef LOG_IN_FRAM
    struct devFram *dataRom;
#endif
#ifdef LOG_IN_FILE
    FILE *fp;
#endif
    BOOL_INT32 writeEnabled;
    BOOL_INT32 roomFilled;
};

/* For configs logging */
struct sysConfigsLog logBuf_configs = {
    .logCount = 0,
    .writeEnabled = BOOL_FALSE,
};

/* For error logging */
struct errInfoLog logBuf_error = {
    .errorCount = 0,
    .writeEnabled = BOOL_FALSE,
    .logPosi = 0,
};

/* For buff logging */
static struct debugLog logBuff = {
    .writeEnabled = BOOL_FALSE,
    .buffUpdated = BOOL_FALSE,
    .logCount = 0
};

/* For FRAM logging */
static struct dataLogger framdataLogger = {
    .writeEnabled = BOOL_FALSE,
    .roomFilled = BOOL_FALSE
};

/* Function to log data to FRAM */
void dataLogfram_write(void)
{
#ifdef PROJECT_ID4
    /* Log the configs first */
    static BOOL_INT32 configLoged = BOOL_TRUE;
    if(logBuf_configs.writeEnabled)
    {
        configLoged = BOOL_FALSE;
    }
    if(!configLoged)
    {
        if(dataLogfram_configsWrite())
        {
            configLoged = BOOL_TRUE;
        }
        return;
    }
#endif

    /* Log the error info secondly */
    static BOOL_INT32 errorLoged = BOOL_TRUE;
    if(logBuf_error.writeEnabled)
    {
        errorLoged = BOOL_FALSE;
    }
    if(!errorLoged)
    {
        if(dataLogfram_errorWrite())
        {
            errorLoged = BOOL_TRUE;
        }
        return;
    }

    /* Log the buff data then */
    if(framdataLogger.writeEnabled)
    {
        static uint16_t number = 0;

        if(0 == number)
        {
            iPrintf("Data of %u logging to FRAM enabled...", logBuff.logCount);
        }
        iPrintf("%d..", number);

        /* log only one parameter */
//        dataLogfram_readfmbuff_word(0, number);
        /* log N parameters */
        dataLogfram_readfmbuff_wordn(0, number);
        number++;

        /* when complete all in the log buffer */
        if(logBuff.logCount < number)
        {
            iPrintf("Data %u logging to FRAM complete!", number-1);
            number = 0;
            framdataLogger.writeEnabled = BOOL_FALSE;
        }
    }
}
#ifdef LOG_IN_FRAM
uint16_t dataLog_getSlaveaddr(uint32_t addr)
{
    ASSERT(FRAM_SIZE > addr);
    return FRAM_I2C_ADDRESS + addr / (FRAM_SIZE/2);
}
#endif
/* Function to log data to buff */
void dataLog_buff(uint16_t *data)
{
    if(!logBuff.writeEnabled)
    {
        return;
    }

#ifndef DATA_LOG_BYTE
	logBuff.buff_value[logBuff.logCount] = *data;
#ifdef DATA_LOG_TIME
        logBuff.buff_value1[logBuff.logCount] = get_timeCount()->ticks_Control;
#endif
	logBuff.buff_value2[logBuff.logCount] = *(data+1);
	logBuff.buff_value3[logBuff.logCount] = *(data+2);
	logBuff.logCount++;
	if(LOG_BUFF_SIZE <= logBuff.logCount)
	{
		logBuff.writeEnabled = BOOL_FALSE;
		logBuff.buffUpdated = BOOL_TRUE;
	}
#else
	uint16_t value;

	value = *data;
	value = value & 0xFF;

	if(logBuff.logCount%2)
	{   /* even number of data -> high byte */
		logBuff.buff_value[logBuff.logCount/2] += value << 8;
	}
	else
	{   /* odd number of data -> low byte */
		logBuff.buff_value[logBuff.logCount/2] = value;
	}

	logBuff.logCount++;
	if(LOG_BUFF_SIZE * 2 <= logBuff.logCount)
	{
		logBuff.writeEnabled = BOOL_FALSE;
		logBuff.buffUpdated = BOOL_TRUE;
	}
#endif
}

void dataLogbuff_enable()
{
    logBuff.writeEnabled = BOOL_TRUE;
    logBuff.logCount = 0;
}

inline void dataLogbuff_disable()               { logBuff.writeEnabled = BOOL_FALSE; }
inline void dataLogbuff_clearfilled()           { logBuff.buffUpdated = BOOL_FALSE; }

/* Function to read RAM in word */
BOOL_INT32 dataLogbuff_print_word(uint16_t addr, uint16_t num)
{
    uint16_t i;

    for(i=0; i<num; i++)
    {
        iPrintf("\r\n%u,", logBuff.buff_value[addr+i]);
#ifdef DATA_LOG_TIME
        iPrintf("%u,", logBuff.buff_value1[addr+i]);
#endif
        iPrintf("%u,", logBuff.buff_value2[addr+i]);
        iPrintf("%u", logBuff.buff_value3[addr+i]);
    }

    return BOOL_TRUE;
}

/* Function to read RAM in byte */
BOOL_INT32 dataLogbuff_print_byte(uint16_t addr, uint16_t num)
{
    uint16_t i;
    uint16_t value = 0;

    for(i=0; i<num; i++)
    {
        if(i%2)
        {   /* even number of data -> high byte */
            value = (logBuff.buff_value[(addr+i)/2] >> 8) & 0xff;
        }
        else
        {   /* odd number of data -> low byte */
            value = (logBuff.buff_value[(addr+i)/2]) & 0xff;
        }

        //iPrintf("\r\n%u", value);           /* for PC */
        iPrintf("%u\t", value);               /* for human */
    }

    return BOOL_TRUE;
}

/* Function to use log buff data to simulate the phase shift curve
 * This is using the buff which stores only phase shift every 200us
 * */
uint16_t dataLogbuff_read(void)
{
    static uint16_t count = 0;
    static uint16_t value;

    if(LOG_BUFF_SIZE * 2 >= count)
    {
        if(count%2)
        {   /* even number of data -> high byte */
            value = (logBuff.buff_value[count/2] >> 8 ) & 0xff;
        }
        else
        {   /* odd number of data -> low byte */
            value = (logBuff.buff_value[count/2]) & 0xff;
        }

        count++;
    }

    return value;
}

#ifdef PROJECT_ID4
/* Function to copy configs to buff */
BOOL_INT32 dataLogfram_configsCopy(void)
{
    struct petdConfig *temp_cfg;
    temp_cfg = petdConfig_get();
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.VoutTarget = temp_cfg->VoutTarget;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.runtime = temp_cfg->runtime;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.thReshigh = temp_cfg->thReshigh;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.thReslow = temp_cfg->thReslow;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.thTempTransfo = temp_cfg->thTempTransfo;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.checkHV = temp_cfg->checkHV;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.checkRes = temp_cfg->checkRes;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.checkTempransfo = temp_cfg->checkTempransfo;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.forceRuntime = temp_cfg->forceRuntime;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.configUpdated = temp_cfg->configUpdated;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.thCout = temp_cfg->thCout;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.checkCout = temp_cfg->checkCout;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.modeControl = temp_cfg->modeControl;
    logBuf_configs.configsLog.logsysConfigs.petdlogConfig.psTarget = temp_cfg->psTarget;

    struct pwmConfig *temp_pwm_cfg;
    temp_pwm_cfg = pwmConfig_get();
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.freq = temp_pwm_cfg->freq;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.deadband_fed1 = temp_pwm_cfg->deadband_fed1;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.deadband_fed2 = temp_pwm_cfg->deadband_fed2;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.deadband_red1 = temp_pwm_cfg->deadband_red1;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.deadband_red2 = temp_pwm_cfg->deadband_red2;
//    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.compensation_down3 = temp_pwm_cfg->compensation_down3;
//    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.compensation_down4 = temp_pwm_cfg->compensation_down4;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.compensation_up3 = temp_pwm_cfg->compensation_up3;
    logBuf_configs.configsLog.logsysConfigs.pwmlogConfig.compensation_up4 = temp_pwm_cfg->compensation_up4;

    return BOOL_TRUE;
}

/* Function to log configs to FRAM */
BOOL_INT32 dataLogfram_configsWrite(void)
{
    if(logBuf_configs.writeEnabled)
    {
    	static BOOL_INT32 copied = BOOL_FALSE;
        uint16_t value;
        static uint32_t addr_start;

        if(!copied)
        {
            /* copy the configs */
            dataLogfram_configsCopy();

            /* write a log mark */
            value = LOG_MARK_CONFIGS;
            addr_start = LOG_ADDR_CONFIGS;
            dataLogger_writeWord(addr_start, 1, &value);

            /* write the configs' size */
            value = LOG_CONFIGS_SIZE;
            addr_start += 2;
            dataLogger_writeWord(addr_start, 1, &value); /* int16_t is 2 bytes */

            /* set the flag */
            copied = BOOL_TRUE;

            /* set the start address for writing configs data */
            addr_start += 2;
        }
        else
        {   /* write the configs */
            static uint16_t number = 0;
            value = logBuf_configs.configsLog.word_16bit[number];
            dataLogger_writeWord(addr_start + number * 2, 1, &value);

            number++;
            if(LOG_CONFIGS_SIZE <= number)
            {   /* writing complete and reset the flags */
                iPrintf("\t===>Logging %u configs complete!", number);
                number = 0;
                copied = BOOL_FALSE;
                logBuf_configs.writeEnabled = BOOL_FALSE;

                return BOOL_TRUE;
            }
        }
    }

    return BOOL_FALSE;
}

//#define TEST_PETDCONFIG_CHECK_FALSE
/* Function to read parameters from FRAM */
BOOL_INT32 dataLogfram_configDisplay(void)
{
	BOOL_INT32 status = BOOL_TRUE;
    uint16_t value;
    uint32_t addr_start;

    /* read and check the log mark */
    addr_start = LOG_ADDR_CONFIGS;
    status = dataLogger_readWord(addr_start, 1, &value);
    if(LOG_MARK_CONFIGS != value)
    {
        iPrintf("\r\n\nNo valid configs to read!");
        return BOOL_FALSE;
    }

    /* read the count */
    addr_start += 2;
    status = dataLogger_readWord(addr_start, 1, &value);
    if((!status) || (LOG_CONFIGS_SIZE != value))
    {
        iPrintf("\r\n\nError in reading logged config count!");
        return BOOL_FALSE;
    }

    /* read the config value */
    uint16_t i;
    addr_start += 2;
    for(i=0; i<LOG_CONFIGS_SIZE; i++)
    {
        status = dataLogger_readWord(addr_start + i * 2, 1, &value);
        if(status)
        {
            logBuf_configs.configsLog.word_16bit[i] = value;
        }
        else
        {
            iPrintf("\r\n\nError in reading logged config!");
            return BOOL_FALSE;
        }
    }

#ifdef TEST_PETDCONFIG_CHECK_FALSE
    struct petdConfig temp_cfg;
    temp_cfg.VoutTarget = 40;//10;
    temp_cfg.runtime = 50;//290;
    temp_cfg.forceRuntime = 0;//1;
    temp_cfg.thTempTransfo = 10;//45;
    temp_cfg.checkTempransfo = 0;//1;
    temp_cfg.checkHV = 0;//-1;
    temp_cfg.thReshigh = 2000;//40;
    temp_cfg.thReslow = 1200;//0;
    temp_cfg.checkRes = 0;//1;
    temp_cfg.thCout = 20;//0;
    temp_cfg.checkCout = 0;//1;
    temp_cfg.configUpdated = 1;
    petdConfig_check(&temp_cfg, petdConfig_get(), BOOL_FALSE);
#else
    /* copy and check the configs */
    pwmConfig_check(&logBuf_configs.configsLog.logsysConfigs.pwmlogConfig, pwmConfig_get(), BOOL_FALSE);
    petdConfig_check(&logBuf_configs.configsLog.logsysConfigs.petdlogConfig, petdConfig_get(), BOOL_FALSE);
#endif

    /* display the configs */
    iPrintf("\r\n\nThe stored configs are read successfully from FRAM!");
    pwmConfig_print();
    petdConfig_print();

    return BOOL_TRUE;
}
#endif	/* #ifdef PROJECT_ID4 */

inline void dataLogfram_writeEnable_configs(void)   {  logBuf_configs.writeEnabled = BOOL_TRUE;   }

inline struct errInfoLog *logBuf_error_get(void)  { return &logBuf_error; }

/* Function to log error info to FRAM */
BOOL_INT32 dataLogfram_errorWrite(void)
{
    if(logBuf_error.writeEnabled)
    {
        static BOOL_INT32 marked = BOOL_FALSE;
        uint16_t value;
        static uint32_t addr_start;

        if(!marked)
        {   /* write error log mark */
            value = LOG_MARK_ERRORINFO;
            addr_start = LOG_ADDR_ERRORINFO;
            dataLogger_writeWord(addr_start, 1, &value);

            /* write error info size */
            value = LOG_ERROR_SIZE;
            addr_start += 2;
            dataLogger_writeWord(addr_start, 1, &value);

            /* write error log count */
            logBuf_error.errorCount++;
            value = logBuf_error.errorCount;
            addr_start += 2;
            dataLogger_writeWord(addr_start, 1, &value);

            /* set the flag */
            marked = BOOL_TRUE;

            /* set the start address for writing error data */
            addr_start += 2;
        }
        else
        {   /* write error info */
            static uint16_t number = 0;

            value = logBuf_error.errorLog.word_16bits[number];
            dataLogger_writeWord(addr_start + logBuf_error.logPosi*LOG_ERROR_SIZE*2 + number*2, 1, &value);

            number++;
            if(LOG_ERROR_SIZE <= number)
            {   /* completed writing this error info */
                iPrintf("Logging error info with code #%u to FRAM complete!", logBuf_error.errorLog.logerrorInfo.errno);
                number = 0;
                marked = BOOL_FALSE;
                logBuf_error.writeEnabled = BOOL_FALSE;

                logBuf_error.logPosi++;
                if(LOG_ERROR_NUMBER <= logBuf_error.logPosi)
                {   /* filled the space for error info, so reset log Position */
                    logBuf_error.logPosi = 0;
                }

                return BOOL_TRUE;
            }
        }
    }

    return BOOL_FALSE;
}

void dataLogfram_errorPrint(void)
{
    iPrintf("\r\nError#%3u", logBuf_error.errorLog.logerrorInfo.errno);
    iPrintf(" | T % 6.2fs", (float)logBuf_error.errorLog.logerrorInfo.runningtime/100.0f);
    iPrintf(" | %s", logBuf_error.errorLog.logerrorInfo.logParameters.modeCtrl ? "CL":"OL");
    iPrintf(" | %s", logBuf_error.errorLog.logerrorInfo.logParameters.modeOp ? "Deice":"Defog");
    iPrintf(" | Vs %uV", logBuf_error.errorLog.logerrorInfo.logParameters.voutSet);
    iPrintf(" | tA % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempAmbient/100.0f);
    iPrintf(" | Tr % 5.1fs", (float)logBuf_error.errorLog.logerrorInfo.logParameters.runtime/100.0f);
    iPrintf(" | Vi % 5.1fV", (float)logBuf_error.errorLog.logerrorInfo.logParameters.vin/100.0f);
    iPrintf(" | Vo % 4.1fV", (float)logBuf_error.errorLog.logerrorInfo.logParameters.vout/100.0f);
    iPrintf(" | Io % 3.1fA", (float)logBuf_error.errorLog.logerrorInfo.logParameters.cout/100.0f);
    iPrintf(" | R %4dmOhms", logBuf_error.errorLog.logerrorInfo.logParameters.res);
    iPrintf(" | tT % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempTransfo/100.0f);
    iPrintf(" | tT1 % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempT1/100.0f);
    iPrintf(" | tT2 % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempT2/100.0f);
    iPrintf(" | tT3 % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempT3/100.0f);
    iPrintf(" | tT4 % 4.1f°C", (float)logBuf_error.errorLog.logerrorInfo.logParameters.tempT4/100.0f);
}

/* Function to read logged error info from FRAM */
BOOL_INT32 dataLogfram_errorDisplay(void)
{
	BOOL_INT32 status;
    uint16_t value;
    uint32_t addr_start;

    /* read and check error log mark */
    addr_start = LOG_ADDR_ERRORINFO;
    status = dataLogger_readWord(addr_start, 1, &value);
    if((LOG_MARK_ERRORINFO != value) || !status)
    {
        iPrintf("\r\n\nFailed to read error info from FRAM! status %d | addr %u | value %X", status, addr_start, value);
        return BOOL_FALSE;
    }

    /* read and check error info size */
    addr_start += 2;
    status = dataLogger_readWord(addr_start, 1, &value);
    if((LOG_ERROR_SIZE != value) || !status)
    {
        iPrintf("\r\n\nError info size not right!");
        return BOOL_FALSE;
    }

    /* force the error count if something wrong with it or for debugging to set the count */
//    value = 11;
//    addr_start += 2;
//    status = framdataLogger_writeWord(addr_start, 1, &value);

    /* read the error log count */
    addr_start += 2;
    status = dataLogger_readWord(addr_start, 1, &value);
    if(status)
    {
        logBuf_error.errorCount = value;
        logBuf_error.logPosi = value % LOG_ERROR_NUMBER;
        iPrintf("\r\n\nTotally %u errors caused!", value);
        if(0 == value)
        {   /* no error stored */
            return BOOL_TRUE;
        }
    }

     /* Ring buffer boundary control
     * if total error number is smaller than or equal to LOG_ERROR_NUMBER, read from 0 to errorCount
     * if total error number is larger than LOG_ERROR_NUMBER, read from logPosi+1 to logPosi
     * */
    uint16_t i, j;
    uint16_t pStart, num;
    if(logBuf_error.errorCount <= LOG_ERROR_NUMBER)
    {   /* it is still in the ring, so maximum is errorCount and start from 0 */
        pStart = 0;
        num = logBuf_error.errorCount;
    }
    else
    {   /* it already exceeds the ring, so maximum is LOG_ERROR_NUMBER and start from the middle */
        iPrintf("\r\nThe last %u errors are:", LOG_ERROR_NUMBER);
        pStart = logBuf_error.logPosi % LOG_ERROR_NUMBER;
        num = LOG_ERROR_NUMBER;
    }

    addr_start += 2;
    for(j=0; j<num; j++)
    {
        uint16_t posi;
        posi = (pStart+j) % LOG_ERROR_NUMBER;
        for(i=0; i<LOG_ERROR_SIZE; i++)
        {
            dataLogger_readWord(addr_start + posi*LOG_ERROR_SIZE*2 + i*2, 1, &value);
            logBuf_error.errorLog.word_16bits[i] = value;
        }

        dataLogfram_errorPrint();
    }

    return BOOL_TRUE;
}

inline void dataLogfram_writeEnable_error(void)     { logBuf_error.writeEnabled = BOOL_TRUE;  }

inline void dataLogfram_writeEnable_buff(void)     { framdataLogger.writeEnabled = BOOL_TRUE; }

/* Function to write words from FRAM to logBuff */
BOOL_INT32 dataLogfram_write2buff_word(void)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint16_t value = 0;
    uint32_t addr_temp = 0;

    uint32_t addr = 0;
    uint16_t num = LOG_BUFF_SIZE;

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i * 2;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value);
#endif
#ifdef LOG_IN_FILE
        status = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value);
#endif
        if(status)
        {
            logBuff.buff_value[i] = value;
        }
        else
        {
            i = num;
        }
    }

    return status;
}

/* Function to write word from logBuff to FRAM */
BOOL_INT32 dataLogfram_readfmbuff_word(uint32_t addr, uint16_t num_No)
{
    uint16_t i;
    uint16_t Value = 0;

    i = num_No;

    Value = logBuff.buff_value[addr+i];

    dataLogger_writeWord(addr+i*2, 1, &Value);

    return BOOL_TRUE;
}

/* Function to write N words (up to 4) from logBuff to FRAM */
BOOL_INT32 dataLogfram_readfmbuff_wordn(uint32_t addr, uint16_t num_No)
{
    uint16_t i;
    uint16_t value = 0, value1 = 0, value2 = 0, value3 = 0;

    i = num_No;
//    for(i=0; i<num; i++)
    {
        value = logBuff.buff_value[addr+i];
        dataLogger_writeWord(addr+i*8, 1, &value);
#ifdef DATA_LOG_TIME
        value1 = logBuff.buff_value1[addr+i];
        dataLogger_writeWord(addr+i*8+2, 1, &value1);
#endif
        value2 = logBuff.buff_value2[addr+i];
        dataLogger_writeWord(addr+i*8+4, 1, &value2);
        value3 = logBuff.buff_value3[addr+i];
        dataLogger_writeWord(addr+i*8+6, 1, &value3);

//        iPrintf("\r\n%d, %d, %d, %u", value1, value2, value3, value4);
    }

    return BOOL_TRUE;
}

BOOL_INT32 dataLogger_writeByte(uint32_t addr, uint16_t num, uint16_t *data)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint32_t addr_temp = 0;

    if(FRAM_SIZE/2 <= addr)
    {
        addr_temp = addr;
    }

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framWrite_Byte(framdataLogger.dataRom, (uint16_t)addr_temp, *(data + i));
#endif
#ifdef LOG_IN_FILE
        status = fileWrite_Byte(framdataLogger.fp, (uint16_t)addr_temp, *(data + i));
#endif

        if(!status)
        {   /* end writing */
            i = num;
        }
        else
        {
//            iPrintf("%X\t", *data + i);
        }
    }

    return status;
}

BOOL_INT32 dataLogger_writeWord(uint32_t addr, uint16_t num, uint16_t *data)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint32_t addr_temp = 0;

    if(FRAM_SIZE/2 <= addr)
    {
        addr_temp = addr;
    }

    for(i=0; i<num; i++)
    {
    	addr_temp = addr + i * 2;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framWrite_Word(framdataLogger.dataRom, (uint16_t)addr_temp, *data + i);
#endif
#ifdef LOG_IN_FILE
        status = fileWrite_Word(framdataLogger.fp, (uint16_t)addr_temp, *data + i);
#endif

        if(!status)
        {   /* end writing */
            i = num;
        }
        else
        {
//            iPrintf("%X\t", *data + i);
        }
    }

    return status;
}

/* Function to read word from FRAM */
BOOL_INT32 dataLogger_readWord(uint32_t addr, uint16_t num, uint16_t *data)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint16_t value = 0;
    uint32_t addr_temp = 0;

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i * 2;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value);
#endif
#ifdef LOG_IN_FILE
        status = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value);
#endif

        if(status)
        {   /* valid value */
            *(data+i) = value;
        }
        else
        {   /* end reading */
            i = num;
        }
    }

    return status;
}

/* Function to print N words (up to 4) from FRAM */
BOOL_INT32 dataLogger_print_wordn(uint32_t addr, uint16_t num)
{
    uint16_t i;
    BOOL_INT32 status1 = BOOL_TRUE, status2 = BOOL_TRUE, status3 = BOOL_TRUE, status4 = BOOL_TRUE;
    uint16_t value1 = 0, value2 = 0, value3 = 0, value4 = 0;
    uint32_t addr_temp = 0;

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i * 8;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status1 = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value1);
#endif
#ifdef LOG_IN_FILE
        status1 = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value1);
#endif

        addr_temp = addr + i * 8 + 2;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status2 = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value2);
#endif
#ifdef LOG_IN_FILE
        status2 = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value2);
#endif

        addr_temp = addr + i * 8 + 4;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status3 = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value3);
#endif
#ifdef LOG_IN_FILE
        status3 = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value3);
#endif

        addr_temp = addr + i * 8 + 6;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status4 = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value4);
#endif
#ifdef LOG_IN_FILE
        status4 = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value4);
#endif

        if(status1 & status2 & status3 & status4)
        {
#ifdef LOG_TEMPERATURES
            iPrintf("\r\n%d, ", (int16_t)value1);
            iPrintf("%d, ", (int16_t)value2);
            iPrintf("%d, ", (int16_t)value3);
            iPrintf("%d", (int16_t)value4);
#else
            iPrintf("\r\n%u, ", (int16_t)value1);
            iPrintf("%u, ", (int16_t)value3);
            iPrintf("%u", (int16_t)value4);
#endif
        }
        else
        {   /* end reading */
            i = num;
        }
    }

    return status1 & status2 & status3 & status4;
}

/* Function to print word from FRAM */
BOOL_INT32 dataLogger_print_word(uint32_t addr, uint16_t num)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint16_t value = 0;
    uint32_t addr_temp = 0;

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i * 2;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framRead_Word(framdataLogger.dataRom, (uint16_t)addr_temp, &value);
#endif
#ifdef LOG_IN_FILE
        status = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value);
#endif

        if(status)
        {   /* valid value */
            iPrintf("\r\n%d", value);
        }
        else
        {   /* end reading */
            i = num;
        }
    }

    return status;
}

/* Function to print byte from FRAM */
BOOL_INT32 dataLogger_print_byte(uint32_t addr, uint16_t num)
{
    uint16_t i;
    BOOL_INT32 status = BOOL_TRUE;
    uint16_t value = 0;
    uint32_t addr_temp = 0;

    for(i=0; i<num; i++)
    {
        addr_temp = addr + i;
#ifdef LOG_IN_FRAM
        framdataLogger.dataRom->frami2cDesc.slave_address = dataLog_getSlaveaddr(addr_temp);
        addr_temp = addr_temp & (FRAM_SIZE/2-1);
        status = framRead_Byte(framdataLogger.dataRom, (uint16_t)addr_temp, &value);
#endif
#ifdef LOG_IN_FILE
        status = fileRead_Word(framdataLogger.fp, (uint16_t)addr_temp, &value);
#endif

        if(status)
        {   /* valid value */
            iPrintf("\r\n%d", value);
        }
        else
        {   /* end reading */
            i = num;
        }
    }

    return status;
}

void dataLog_init(void)
{
#ifdef LOG_IN_FRAM
    framdataLogger.dataRom = framGet();
#endif
#ifdef LOG_IN_FILE
    //TODO: open files
    ndebugPrintf("Open files to store data!");
#endif
}
