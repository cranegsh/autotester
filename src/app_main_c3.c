/*
 * canfdComm.c
 *
 *  Created on: Dec. 21, 2024
 *  modified on Dec. 21, 2024 from app_canfd.c
 *      Author: Crane Shao
 */

#include "app_main_c3.h"
#include "utility.h"

/* CAN Message IDs for C3 project */
#define BMS_20                      0x0CF			/* Voltage */
#define BMS_22                      0x12DD54D1		/* State of Charge */
#define LiSi_01                     0x16A954BB		/* SW on dashboard */
#define ESP_21                      0x0FD			/* Speed */
#define KLIMA_03                    0x66E			/* Inside Temp. */
#define KLIMA_16                    0x16A95493		/* FSH Status */
#define KLIMA_S_01                  0x6B0			/* Humidity */
#define TEMP_01                     0x1A5555A6		/* Outside Temp. */
#define SYSTEMINFO_01               0x585			/* Bus Identification */
#define CAN_CUR                     0x587U          /* temp for test */
#define CAN_OP_MODE                 0x588U          /* temp for test */

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


static struct canfdData_c3 canfdio = {
   .c3canDataInfo = {
		/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
		{ BMS_22, "S.Charge", 1000, 0, 88 },
		{ KLIMA_16, "FSH Sts", 1000, 0, 0 },
		{ TEMP_01, "Amb.Temp.", 2000, 0, -10 },
		{ BMS_20<<CAN_EID_BITS, "Voltage", 10, 0, 350 }, //1000, 0, 0 },//	/* TODO: check why short interval causes failure to exit program by hitting 'x'? */
		{ ESP_21<<CAN_EID_BITS, "V.Speed", 10, 0, 99 }, //1000, 0, 0 },//
		{ KLIMA_03<<CAN_EID_BITS, "Cab.Temp.", 2000, 0, 22 },
		{ KLIMA_S_01<<CAN_EID_BITS, "Humidity", 2000, 0, 37 },
		{ SYSTEMINFO_01<<CAN_EID_BITS, "Sys. ID", 1000, 0, 85 },
		{ CAN_CUR<<CAN_EID_BITS, "Current", 10, 0, 0 }, //1000, 0 },//
		{ CAN_OP_MODE<<CAN_EID_BITS, "Op.mode", 1000, 0, 0 },
   },
   .c3dataIn.data = { 0, 0, 0, 0, 0},
};
static dataIn_type_c3 data_prev = {
	.data = {0, 0, 0, 0, 0}
};

inline struct canfdData_c3 *msg_canfd_getData_c3(void)   { return &canfdio; }

uint32_t msg_canfd_getMid_c3(uint32_t number)
{
	return canfdio.c3canDataInfo[number].mid;
}

/* Function to prepare CANFD data for C3 vehicle messages */
int msg_canfd_prepare_c3Veh(msg_mode_t msgno, int32_t value, uint8_t *data)
{
	switch((int)msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[3] = 0x10;
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			data[2] = ((value + 50 ) * 2) & 0xFF;
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			value = value * 4;
			data[7] = (value >> 4) & 0xFF;			/* take the higher 8 bits of total 12 bits and right shift 4 bits */
			data[6] = (value & 0x0F) << 4;			/* take the lower 4 bits and left shift 4 bits */
			break;
		case APP_OPT_DEV_SEND_SOC:
			value = value * 20;
			data[3] = (value >> 7 ) & 0x0F;			/* take the higher 4 bits of total 11 bits and right shift 7 bits */
			data[2] = (value & 0x7F) << 1;			/* take the lower 7 bits and left shift one bit */
			break;
		case APP_OPT_DEV_SEND_SPEED:
			value = value * 100;
			data[5] = (value >> 8 ) & 0xFF;			/* take the higher 8 bits of total 16 bits and right shift 8 bits */
			data[4] = value & 8;					/* take the lower 8 bits */
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[4] = ((value + 50 ) * 2 ) & 0xFF;
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
			data[5] = (value * 2 + 1) & 0xFF;
			break;
		case APP_OPT_DEV_SEND_SYSID:
			data[4] = (uint8_t)value;
			break;
		case APP_OPT_DEV_SEND_CURRENT:
			data[7] = (uint8_t)value;
			break;
		case APP_OPT_DEV_SEND_OPMODE:
			data[0] = (uint8_t)value;
			break;
		default:
			return -1;
	}
	return 0;
}

void msg_canfd_clear_c3(void)
{
	for(uint32_t i=0; i<CAN_DATA_IN_LEN_C3; i++)
	{
		canfdio.c3dataIn.byte[i] = 0;
	}
}

/* This is to interpret the messages from the controller */
void msg_canfd_interpret_c3(uint32_t mid, uint8_t *data, uint32_t num)
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
			canfdio.c3dataIn.data.petdRuntime = ((float)temp) / 100;			/* converted from 0.01s */
			break;
		case PETD_COMMAND:
			debugPrintf_canfd("PETD Command:\t");
			canfdio.c3dataIn.data.petdCommand = (uint16_t)data[0];
			break;
		case WS_RESISTANCE:
			dbgPrintf_canfd("Windshield resistance: %d | ", num);
			temp = data[1];
			temp = (temp << 8) + data[0];
			canfdio.c3dataIn.data.resistance = ((float)temp / 1000);		/* converted from 0.001Ohms */
			break;
		case ERROR_CODE:
			debugPrintf_canfd("Error code:\t");
			temp = data[3];
			temp = (temp << 8) + data[2];
			canfdio.c3dataIn.data.errorCode = temp;
			temp = data[5];
			temp = (temp << 8) + data[4];
			canfdio.c3dataIn.data.errorValue = temp;
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

void app_main_c3_sendCommand(sysData_type *sdata, int cmd)
{
	if(canfdio.updated) {
		ndPrintf("\n Sending data '%c' to CAN ...", cmd);
		msg_canfd_send_tester(sdata->project_id, (uint32_t)cmd);
		ndPrintf("\n data '%c' to CAN sent!", cmd);
		canfdio.updated = BOOL_FALSE;
	}
}

void app_main_c3_getMsgvalue(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->val = canfdio.c3canDataInfo[(int)msgi->mode].value;
}

void app_main_c3_getMsginfo(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->interval = canfdio.c3canDataInfo[(int)msgi->mode].interval;
	msgi->num = canfdio.c3canDataInfo[(int)msgi->mode].num;

	ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", canfdio.c3canDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, \
			msgi->interval, msgi->num);					/* when controlling loop number of every single message */
	iPrintf("%s:\t%d\t%d\t | %d\n", canfdio.c3canDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_displayMsg_c3(msg_opt_t *msgi)
{
	iPrintf("%s:\t%d\t%d\t | %d\n", canfdio.c3canDataInfo[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_c3_print_canVeh(uint32_t func, int msgNum, int msgCount)
{
	if(PROJECT_FUNC_CAN == func) {
		iPrintf("%s: %3d | ", canfdio.c3canDataInfo[msgNum].name, msgCount);
	}
}

void app_main_displayResult_c3(void)
{
	iPrintf("PETD command %d, runtime %6.3f s | WS Res %6.3f Ohms | Error code %8X, value %d\r\n",
			canfdio.c3dataIn.data.petdCommand, canfdio.c3dataIn.data.petdRuntime, canfdio.c3dataIn.data.resistance,
			canfdio.c3dataIn.data.errorCode, canfdio.c3dataIn.data.errorValue);
}

int app_main_checkResult_c3(void)
{
	int status = 0;
	//static dataIn_type_c3 data_prev;

	if(data_prev.data.petdCommand != canfdio.c3dataIn.data.petdCommand) {
		ndPrintf("petdCommand: %d != %d\t", data_prev.data.petdCommand, canfdio.c3dataIn.data.petdCommand);
		data_prev.data.petdCommand = canfdio.c3dataIn.data.petdCommand;
		app_main_displayResult_c3();
	}

	if((int)(data_prev.data.petdRuntime * 100) != (int)(canfdio.c3dataIn.data.petdRuntime * 100)) {
		ndPrintf("petdRuntime: %d != %d\t", (int)(data_prev.data.petdRuntime * 100), (int)(canfdio.c3dataIn.data.petdRuntime * 100));
		data_prev.data.petdRuntime = canfdio.c3dataIn.data.petdRuntime;
		app_main_displayResult_c3();
	}

	if((int)(data_prev.data.resistance * 1000) != (int)(canfdio.c3dataIn.data.resistance * 1000)) {
		ndPrintf("Resistance: %d != %d\t", (int)(data_prev.data.resistance * 1000), (int)(canfdio.c3dataIn.data.resistance * 1000));
		data_prev.data.resistance = canfdio.c3dataIn.data.resistance;
		app_main_displayResult_c3();
	}

	if(data_prev.data.errorCode != canfdio.c3dataIn.data.errorCode) {
		ndPrintf("errorCode: %d != %d\t", data_prev.data.errorCode, canfdio.c3dataIn.data.errorCode);
		data_prev.data.errorCode = canfdio.c3dataIn.data.errorCode;
		app_main_displayResult_c3();
	}

	return status;
}
