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

//#define CAN_VEH_MSG_NUM				10								/* basic CAN messages for C3, ID4 + 2: current and op mode for now. */
#define FILTER_TOTAL                (CAN_VEH_MSG_NUM + 2 + 7)		/* 1: system ID; 2:two general filters; 7: debugger control */
#define FILTER_NUMBER				(FILTER_TOTAL)	/* same as ID4 */

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

static struct canfdData_c3 canfdio = {
   .c3canDataInfo = {
		/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
		{ BMS_22, "S.Charge", 1000, 0 },
		{ KLIMA_16, "FSH Sts", 1000, 0 },
		{ TEMP_01, "Amb.Temp.", 2000, 0 },
		{ BMS_20<<CAN_EID_BITS, "Voltage", 10, 0 },
		{ ESP_21<<CAN_EID_BITS, "V.Speed", 10, 0 },
		{ KLIMA_03<<CAN_EID_BITS, "Cab.Temp.", 2000, 0 },
		{ KLIMA_S_01<<CAN_EID_BITS, "Humidity", 2000, 0 },
		{ SYSTEMINFO_01<<CAN_EID_BITS, "Sys. ID", 100, 0 },
		{ CAN_CUR<<CAN_EID_BITS, "Current", 10, 0 },
		{ CAN_OP_MODE<<CAN_EID_BITS, "Op.mode", 1000, 0 },
   }
};

union CANMSG_BMS20 {
    struct {
        uint32_t void_word;
        struct {
            uint32_t void_bits:16;
            uint32_t low:8;
            uint32_t high:8;
        } bms20_vol;
    } bF;
    struct {
        uint32_t void_word;
        struct {
            uint32_t void_bits:20;
            uint32_t low:4;
            uint32_t high:8;
        } bms20_vol;
    } bitsF;
    uint16_t hword[4];
    uint8_t byte[8];
};

union CANMSG_BMS22 {
    struct {
        struct{
            uint32_t void_bits:16;
            uint32_t low:8;
            uint32_t high:8;
        } bms22_soc;
        uint32_t void_word;
    } bF;
    struct {
        struct{
            uint32_t void_bits:17;
            uint32_t low:7;
            uint32_t high:4;
            uint32_t unimplmented;
        } bms22_soc;
        uint32_t void_word;
    } bitsF;
    uint16_t hword[4];
    uint8_t byte[8];
};

union CANMSG_ESP21 {
    struct {
        uint32_t void_word;
        struct {
            uint32_t low: 8;
            uint32_t high: 8;
            uint32_t unplemented1: 7;
            uint32_t Qbit: 1;
            uint32_t unplemented2: 8;
        } esp21_speed;
    } bitF;
    uint16_t hword[4];
    uint8_t byte[8];
};

inline struct canfdData_c3 *msg_canfd_getData_c3(void)   { return &canfdio; }

inline uint32_t msg_canfd_getMid_c3(int number) { return canfdio.c3canDataInfo[number].mid; }

/* Function to prepare CANFD data for C3 vehicle messages */
int32_t msg_canfd_prepare_c3Veh(msg_mode_t msgno, int32_t value, uint8_t *data)
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

void app_main_c3_sendCommand(sysData_type *sdata, int cmd)
{
	if((0 == sdata->canfd_status) && (canfdio.updated)) {
		ndPrintf("\n Sending data '%c' to CAN ...", cmd);
		msg_canfd_send_tester(sdata->project_id, (uint32_t)cmd);
		ndPrintf("\n data '%c' to CAN sent!", cmd);
		canfdio.updated = BOOL_FALSE;
	}
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

void app_main_c3_print_canVeh(int msgNum, int msgCount)
{
	iPrintf("%s: %3d | ", canfdio.c3canDataInfo[msgNum].name, msgCount);
}
