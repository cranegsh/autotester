/*
 * canfdComm.c
 *
 *  Created on:    May 05, 2024
 *  Last modified: May 09, 2024
 *  Created from: app_main_c3.c
 */

#include "app_main_volvo.h"
#include "utility.h"

/* CAN Message IDs for Volvo project */
#ifndef VOLVO_ALASKA
#define AMB_IC						0x18FEF517
#define VP152						0x18FF98EF      /* HV_SOC */
#define VP155_T						0x0CFF9B03      /* HV_READY */
#else
#define AMB_IC						0x18FEF500
#define BAT_SOC                     0x18FCB631      /* Battery SoC */
#define BAT_VOL                     0x18FEF717      /* Battery Voltage */
#endif
#define CCVS_V						0x18FEF111
#define TD_IC						0x18FEE617
#define DEFROST_SIG                 0x381

/* CANFD data should be interpreted according to the below DBC information:
 * BO_ 207 BMS_20: 8
 * SG_ BMS_Spannung : 52|12@1+ (0.25,0) [0|1000] "Unit_Volt" Vector__XXX
 * BO_ 2463978705 BMS_22:
 * 8 SG_ BMS_Ladezustand : 17|11@1+ (0.05,0) [0|100] "Unit_PerCent" Vector__XXX
 * BO_ 253 ESP_21: 8
 * SG_ ESP_v_Signal : 32|16@1+ (0.01,0) [0|655.32] "Unit_KiloMeterPe" Vector__XXX
 * SG_ ESP_QBit_v_Signal : 55|1@1+ (1,0) [0|1] "" Vector__XXX
 * BO_ 1646 Klima_03: 8
 * SG_ KL_Innen_Temp : 32|8@1+ (0.5,-50) [-50|76] "Unit_DegreCelsi" Vector__XXX
 * BO_ 2527679635 Klima_16: 8
 * SG_ FSH_Status : 28|2@1+ (1,0) [0|3] "" Vector__XXX
 * BO_ 1712 Klima_Sensor_01: 8
 * SG_ FS_Luftfeuchte_rel : 40|8@1+ (0.5,-0.5) [0|100] "Unit_PerCent" Vector__XXX
 * BO_ 2589283750 Temperaturen_01: 8
 * SG_ KBI_QBit_Aussen_Temp_gef : 12|1@1+ (1,0) [0|1] "" Vector__XXX
 * SG_ KBI_Aussen_Temp_gef : 16|8@1+ (0.5,-50) [-50|75] "Unit_DegreCelsi" Vector__XXX
 */
//#define CAN_STANDARD_ONLY

/* ID of the messages sent from the controller */
#define PETD_RUN_TIME               0x396U
#define PETD_COMMAND                0x05CU
#define WS_RESISTANCE               0x19ABCDEFU
#define ERROR_CODE                  0x15793468U

const uint64_t TIMESTAMP = 			(SECONDS 			<< SECONDS_BIT_START)
								  + (MINUTES 			<< MINUTES_BIT_START)
								  + (HOURS				<< HOURS_BIT_START)
								  + (MONTHS				<< MONTHS_BIT_START)
								  +	((uint64_t)DAYS 	<< DAYS_BIT_START)
								  + ((uint64_t)YEARS	<< YEARS_BIT_START);

static struct canfdData_Volvo canfdio = {
   .volvocanDataInfo = {
		/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
#ifndef VOLVO_ALASKA
        { VP152, "S. of Charge", 1000, 0, 55 },
#else
        { BAT_SOC, "S. of Charge", 1000, 0, 55 },
#endif
        { DEFROST_SIG<<CAN_EID_BITS, "Defrost S.", 500, 0, 0 },
#ifndef VOLVO_ALASKA
        { AMB_IC, "Amb.Temp.", 1000, 0, -5 },
		{ VP152, "HighVoltage", 500, 0, 648 },
#else
        { AMB_IC, "Amb.&Cab.Temp.", 1000, 0, (18 << 16) + (uint16_t)(-5) },      /* with Cab. temp in the same message */
        { BAT_VOL, "BatVoltage", 500, 0, 24 },
#endif
		{ CCVS_V, "Veh.Speed", 100, 0, 250 },
		{ TD_IC, "Time", 1000, 0, TIMESTAMP },
   },
   .volvodataIn.data = { 0, 0, 0, 0, 0},
};
static dataIn_type_volvo data_prev = {
	.data = {0, 0, 0, 0, 0}
};

inline struct canfdData_Volvo *msg_canfd_getData_Volvo(void)   { return &canfdio; }

uint32_t msg_canfd_getMid_Volvo(uint32_t number)
{
	return canfdio.volvocanDataInfo[number].mid;
}

/* Function to prepare CANFD data for Volvo vehicle messages */
int msg_canfd_prepare_VolvoVeh(msg_mode_t msgno, int64_t value, uint8_t *data)
{
    int32_t temp_value = (int32_t)value;

    for(uint32_t i=0; i<CAN_MSG_LEN; i++) {
        data[i] = 0;
    }

	switch((int)msgno + MSGNO_OFFSET) {
	    case APP_OPT_DEV_SEND_SOC:
#ifndef VOLVO_ALASKA
#else
            value = value * 400;
            data[1] = (value >> 8 ) & 0xFF;
            data[0] = (value & 0xFF) << 1;
#endif
	        break;
	    case APP_OPT_DEV_SEND_FSH:
            if(0 != value) {
                data[3] = 0x10;
            }
            break;
		case APP_OPT_DEV_SEND_ATEMP:
		    /* get amb. temp from lower 16-bit */
		    value = (int64_t)(temp_value & 0xFFFF);
		    value = ((value + 273) * 32);
			data[4] = (value >> 8) & 0xFF;
			data[3] = value & 0xFF;
			/* get cab. temp from higher 16-bit */
			value = (int64_t)((temp_value >> 16) & 0xFFFF);
            value = ((value + 273) * 32);
            data[2] = (value >> 8) & 0xFF;
            data[1] = value & 0xFF;
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
#ifndef VOLVO_ALASKA
			value *= 0.25;
			data[0] = value & 0xFF;
#else
            value *= 20;
            data[4] = value & 0xFF;
            data[5] = (value >> 8) & 0xFF;
#endif
			break;
		case APP_OPT_DEV_SEND_SPEED:
			value *= 256;
			data[2] = (value >> 8) & 0xFF;
			data[1] = value & 0xFF;
			break;
//		case APP_OPT_DEV_SEND_PTDRVLNSTATUS:
//			data[0] = (value & 0x7) << 3;
//			break;
		case (APP_OPT_DEV_SEND_TIME - 5):   /* relevant to the position in enum msg_mode_t */
			data[5] = ((value >> 40) & 0xFF) - 1985;
			data[4] = ((value >> 32) & 0xFF) * 4;
			data[3] = (value >> 24) & 0xFF;
			data[2] = (value >> 16) & 0xFF;
			data[1] = (value >> 8) & 0xFF;
			data[0] = (value & 0xFF) * 4;
			break;
		default:
			return -1;
	}
	return 0;
}

void msg_canfd_clear_Volvo(void)
{
	for(uint32_t i=0; i<CAN_DATA_IN_LEN_VOLVO; i++)
	{
		canfdio.volvodataIn.byte[i] = 0;
	}
}

/* This is to interpret the messages from the controller */
void msg_canfd_interpret_Volvo(uint32_t mid, uint8_t *data, uint32_t num)
{
    uint32_t i;
    uint16_t temp;

    dbgPrintf_canfd("\n");
    ndebugPrintf("Received 0x%X | %d\t", mid, num);
    //print_array_byte(data, num); dPrintf("\n");

	switch(mid)
	{
		case PETD_RUN_TIME:
			dbgPrintf_canfd("PETD Runtime: %d | ", num);
			temp = data[1];
			temp = (temp << 8) + data[0];
			canfdio.volvodataIn.data.petdRuntime = ((float)temp) / 100;			/* converted from 0.01s */
			break;
		case PETD_COMMAND:
			debugPrintf_canfd("PETD Command:\t");
			canfdio.volvodataIn.data.petdCommand = (uint16_t)data[0];
			break;
		case WS_RESISTANCE:
			dbgPrintf_canfd("Windshield resistance: %d | ", num);
			temp = data[1];
			temp = (temp << 8) + data[0];
			canfdio.volvodataIn.data.resistance = ((float)temp / 1000);		/* converted from 0.001Ohms */
			break;
		case ERROR_CODE:
			debugPrintf_canfd("Error code:\t");
			temp = data[3];
			temp = (temp << 8) + data[2];
			canfdio.volvodataIn.data.errorCode = temp;
			temp = data[5];
			temp = (temp << 8) + data[4];
			canfdio.volvodataIn.data.errorValue = temp;
			break;
		default:
			dbgPrintf_canfd("Invalid data! - ID%X\t", mid);
			for(i=0; i<num; i++)
			{
				dbgPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
			}
			break;
	}
}

void app_main_Volvo_sendCommand(sysData_type *sdata, int cmd)
{
	if(canfdio.updated) {
		ndPrintf("\n Sending data '%c' to CAN ...", cmd);
		msg_canfd_send_tester(sdata->project_id, (uint32_t)cmd);
		ndPrintf("\n data '%c' to CAN sent!", cmd);
		canfdio.updated = BOOL_FALSE;
	}
}

void app_main_Volvo_getMsgvalue(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->val = canfdio.volvocanDataInfo[(int)msgi->mode].value;
}

void app_main_Volvo_getMsginfo(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->interval = canfdio.volvocanDataInfo[(int)msgi->mode].interval;
	msgi->num = canfdio.volvocanDataInfo[(int)msgi->mode].num;

	ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", canfdio.volvocanDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, \
			msgi->interval, msgi->num);					/* when controlling loop number of every single message */
	iPrintf("%s:\t%d\t%ld\t | %d\n", canfdio.volvocanDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_displayMsg_Volvo(msg_opt_t *msgi)
{
	iPrintf("%s:\t%d\t%ld\t | %d\n", canfdio.volvocanDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_Volvo_print_canVeh(uint32_t func, int msgNum, int msgCount)
{
	if(PROJECT_FUNC_CAN == func) {
		iPrintf("%s: %3d | ", canfdio.volvocanDataInfo[msgNum].name, msgCount);
	}
}

void app_main_displayResult_Volvo(void)
{
	iPrintf("PETD command %d, runtime %6.3f s | WS Res %6.3f Ohms | Error code %8X, value %d\n",
			canfdio.volvodataIn.data.petdCommand, canfdio.volvodataIn.data.petdRuntime, canfdio.volvodataIn.data.resistance,
			canfdio.volvodataIn.data.errorCode, canfdio.volvodataIn.data.errorValue);
}

int app_main_checkResult_Volvo(void)
{
	int status = 0;
	//static dataIn_type_c3 data_prev;

	if(data_prev.data.petdCommand != canfdio.volvodataIn.data.petdCommand) {
		ndPrintf("petdCommand: %d != %d\t", data_prev.data.petdCommand, canfdio.volvodataIn.data.petdCommand);
		data_prev.data.petdCommand = canfdio.volvodataIn.data.petdCommand;
		app_main_displayResult_Volvo();
	}

	if((int)(data_prev.data.petdRuntime * 100) != (int)(canfdio.volvodataIn.data.petdRuntime * 100)) {
		ndPrintf("petdRuntime: %d != %d\t", (int)(data_prev.data.petdRuntime * 100), (int)(canfdio.volvodataIn.data.petdRuntime * 100));
		data_prev.data.petdRuntime = canfdio.volvodataIn.data.petdRuntime;
		app_main_displayResult_Volvo();
	}

	if((int)(data_prev.data.resistance * 1000) != (int)(canfdio.volvodataIn.data.resistance * 1000)) {
		ndPrintf("Resistance: %d != %d\t", (int)(data_prev.data.resistance * 1000), (int)(canfdio.volvodataIn.data.resistance * 1000));
		data_prev.data.resistance = canfdio.volvodataIn.data.resistance;
		app_main_displayResult_Volvo();
	}

	if(data_prev.data.errorCode != canfdio.volvodataIn.data.errorCode) {
		ndPrintf("errorCode: %d != %d\t", data_prev.data.errorCode, canfdio.volvodataIn.data.errorCode);
		data_prev.data.errorCode = canfdio.volvodataIn.data.errorCode;
		app_main_displayResult_Volvo();
	}

	return status;
}
