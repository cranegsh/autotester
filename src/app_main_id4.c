/*
 * canfdComm.c
 *
 *  Created on: Dec. 21, 2024
 *  modified on Dec. 21, 2024 from app_canfd.c
 *      Author: Crane Shao
 */
#include <string.h>

#include "app_main_id4.h"
#include "utility.h"

//#define CAN_VEH_MSG_NUM				7								/* basic CAN messages for ID4 */
#define FILTER_TOTAL                (CAN_VEH_MSG_NUM + 1 + 2 + 7)	/* 1: system ID; 2:two general filters; 7: debugger control */
#define FILTER_NUMBER				FILTER_TOTAL

/* CAN Message IDs for ID4 project */
/* ID4 messages */
#define BMS_20                      0x0CF			/* Voltage */
#define BMS_22                      0x12DD54D1		/* State of Charge */
#define LiSi_01                     0x16A954BB		/* SW on dashboard */
#define ESP_21                      0x0FD			/* Speed */
#define KLIMA_03                    0x66E			/* Inside Temp. */
#define KLIMA_16                    0x16A95493		/* FSH Status */
#define KLIMA_S_01                  0x6B0			/* Humidity */
#define TEMP_01                     0x1A5555A6		/* Outside Temp. */
#define SYSTEMINFO_01               0x585			/* Bus Identification */

/* BZ4X/G3 shared messages */
#define TEMP_AMB_SIG				0x3B0
#define TEMP_AMB_RAW				0x380
#define TEMP_CABIN					0x407
#define SPEED_VEH					0x610
#define HUMIDITY_CABIN				0x480
#define DEFROST_SIG					0x381
/* BZ4X messages */
#define G4R_TEMP_AMB				TEMP_AMB_RAW//TEMP_AMB_SIG
#define G4R_HV_READY				0x3B6
#define G4R_HV_SOC					0x3B6
/* G3 messages */
#define G3_TEMP_AMB					TEMP_AMB_RAW
#define G3_HV_READY					0x51E
#define G3_HV_SOC					0x356

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

static struct canfdData_id4 canfdio = {
   .id4Dataveh = {0, },
   .id4DataIn = { {0, 0, }, },
   .id4DataIn_Cfg = { { 0, { }, { }, }, },
   .id4DataIn_Ctl = { {0, 0, }, },
   .id4DataIn_Env = { {0, 0, }, },
   .id4DataIn_Res = { {0, 0, }, },
   .id4DataLog = { {0, 0, 0, 0, 0, 0}, },
   .id4DataOut = { {0, 0, }, },
   .id4DataOut_command = { {0, 0 }, },
   .id4DataOut_cfgPetd = { {0, 0 }, },
   .id4DataOut_cfgPwm = { {0, 0 }, },
   .id4_vehData = {
	/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
//		{ BMS_22, "S. of Charge", 1000, 0 },
		{ LiSi_01, "SW on Dash", 100, 0 },
		{ KLIMA_16, "FSH status", 500, 0 },
		{ TEMP_01, "Amb. Temp.", 1000, 0 },
		{ BMS_20<<CAN_EID_BITS, "Voltage", 200, 0 },
		{ ESP_21<<CAN_EID_BITS, "Veh. Speed", 100, 0 },
		{ KLIMA_03<<CAN_EID_BITS, "Cab. Temp.", 1000, 0 },
		{ KLIMA_S_01<<CAN_EID_BITS, "Humidity", 1000, 0 }
   }
};

/* for ROJECT_CAN_G3 */
static struct canfdData_id4 canfdio_g3 = {
   .id4_vehData = {
	/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
		{ G3_HV_SOC<<CAN_EID_BITS, "S. of Charge", 1000, 0 },
		{ DEFROST_SIG<<CAN_EID_BITS, "Defrost S.", 500, 0 },
		{ G3_TEMP_AMB<<CAN_EID_BITS, "Amb. Temp. S", 1000, 0 },
		{ G3_HV_READY<<CAN_EID_BITS, "HV Ready", 500, 0 },
		{ SPEED_VEH<<CAN_EID_BITS, "Veh. Speed", 100, 0 },
		{ TEMP_CABIN<<CAN_EID_BITS, "Cab. Temp.", 1000, 0 },
		{ HUMIDITY_CABIN<<CAN_EID_BITS, "Humidity", 1000, 0 }
   }
};

/* for PROJECT_CAN_BZ4X (G4R) */
static struct canfdData_id4 canfdio_g4r = {
   .id4_vehData = {
	/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
		{ G4R_HV_SOC<<CAN_EID_BITS, "S. of Charge", 1000, 0 },
		{ DEFROST_SIG<<CAN_EID_BITS, "Defrost S.", 500, 0 },
		{ G4R_TEMP_AMB<<CAN_EID_BITS, "Amb. Temp. S", 1000, 0 },
		{ G4R_HV_READY<<CAN_EID_BITS, "HV Ready", 500, 0 },
		{ SPEED_VEH<<CAN_EID_BITS, "Veh. Speed", 100, 0 },
		{ TEMP_CABIN<<CAN_EID_BITS, "Cab. Temp.", 1000, 0 },
		{ HUMIDITY_CABIN<<CAN_EID_BITS, "Humidity", 1000, 0 }
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

inline struct canfdData_id4 *msg_canfd_getData_id4(void)   { return &canfdio; }

inline uint32_t msg_canfd_getMid_id4(int number) 	{ return canfdio.id4_vehData[number].mid; }

inline uint32_t msg_canfd_getMid_g3(int number) 	{ return canfdio_g3.id4_vehData[number].mid; }

inline uint32_t msg_canfd_getMid_g4r(int number) 	{ return canfdio_g4r.id4_vehData[number].mid; }

/* Function to prepare CANFD data for ID4 vehicle messages */
int32_t msg_canfd_prepare_id4Veh(msg_mode_t msgno, int32_t value, uint8_t *data)
{
	switch(msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[3] = 0x10;
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			data[2] = ((value + 50 ) * 2 ) & 0xFF;
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			value = value * 4;
			data[7] = (value >> 4) & 0xFF;			/* take the higher 8 bits of total 12 bits and right shift 4 bits */
			data[6] = (value & 0x0F) << 4;			/* take the lower 4 bits and left shift 4 bits */
			break;
		case APP_OPT_DEV_SEND_SOC:
#if 0	// change SOC to LiSi_01
			value = value * 20;
			data[3] = (value >> 7 ) & 0x0F;			/* take the higher 4 bits of total 11 bits and right shift 7 bits */
			data[2] = (value & 0x7F) << 1;			/* take the lower 7 bits and left shift one bit */
#else
			if(0 != value) {
				data[6] = 0x40;
			}
#endif
			break;
		case APP_OPT_DEV_SEND_SPEED:
			value = value * 100;
			data[5] = (value >> 8 ) & 0xFF;			/* take the higher 8 bits of total 16 bits and right shift 8 bits */
			data[4] = value & 8;					/* take the lower 8 bits */
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[4] = ((value + 50 ) * 2) & 0xFF;
			// When setting Cabin temperature as 1, FSH_Auto is set at the same time!
			if(1 == value) {
				data[6] = 0x08;
			}
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
//			data[5] = value * 2 + 1;
			value = value * 10 + 396;
			data[2] = value & 0xFF;
			data[3] = (value >> 8) & 0x03;
			break;
		default:
			return -1;
	}
	return 0;
}

/* Function to prepare CANFD data for G3 and BZ4X vehicle messages */
int32_t msg_canfd_prepare_g3Veh(msg_mode_t msgno, int32_t value, uint8_t *data)
//int32_t msg_canfd_prepare_bz4xVeh(msg_mode_t msgno, int32_t value, uint8_t *data)
{
	float temp;
	switch((int)msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[1] |= 0x20;
			}
			else {
				data[1] &= (~0x20);
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			temp = (float)value / 160 * 256;
			value = (int32_t)temp;
			if(0 <= value) {
				data[6] = value & 0xFF;
			}
			else {
				data[6] = (0x100 + value) & 0xFF;				/* two's complement */
			}
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			if(0 != value) {
				data[0] |= 0x80;
			}
			else {
				data[0] &= (~0x80);
			}
			break;
		case APP_OPT_DEV_SEND_SOC:
			//not confirmed yet
			break;
		case APP_OPT_DEV_SEND_SPEED:
			data[2] = value & 0xFF;
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[2] = (value * 4 + 26) & 0xFF;				/* (value + 6.5 ) / 0.25 */
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
			// not confirmed yet
			break;
		default:
			return -1;
	}
	return 0;
}

/* Function to prepare CANFD data for ID4 project */
int32_t msg_canfd_prepare_id4(uint32_t option, uint32_t *mid, uint8_t *data, uint32_t *num)
{
	int32_t ret = 0;
	uint32_t i;

	switch(option) {
		case COMMAND_C:
		    *mid = ID_SEND_CFG_PETD << CAN_EID_BITS;
		    *num = CAN_DATA_OUT_CFG_PETD_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_cfgPetd.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send petd config!");
			break;
		case COMMAND_F:
		    *mid = ID_SEND_CFG_PWM << CAN_EID_BITS;
		    *num = CAN_DATA_OUT_CFG_PWM_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_cfgPwm.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send pwm config!");
			break;
		case COMMAND_P:
		case COMMAND_D:
		    *mid = ID_SEND_COMMAND << CAN_EID_BITS;
		    *num = CAN_DATA_OUT_COMMAND_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_command.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send command!");
			break;
		case COMMAND_S:
			*num = CAN_VEH_MSG_LEN;
	        if(1 == dData.display) {
			    *mid = KLIMA_16;
	        }
	        else if(2 == dData.display) {
			    *mid = KLIMA_16;
			    data[3] = 0x10;				/* FSH = ON */
	        }
	        else if(3 == dData.display) {
			    *mid = TEMP_01;
			    data[2] = (dData.number + 50 ) * 2;		/* convert temperature to CAN data */
	        }
		    ndPrintf("\n Send CAN message %d with %d!", dData.display, dData.number);
			break;
		default:
			ndPrintf("\n Send nothing!");
			ret = -1;
			break;
	}

	return ret;
}

/* This is to interpret the messages from the vehicle */
BOOL_INT32 msg_canfd_interpret_id4Veh(uint32_t mid, uint8_t *data, uint32_t num)
{
	BOOL_INT32 status = BOOL_TRUE;
    uint32_t i, temp;
    float value;
    union CANMSG_BMS20 canmsgBMS20;
    union CANMSG_BMS22 canmsgBMS22;
    union CANMSG_ESP21 canmsgESP21;

	dbgPrintf_canfd("\n");
    if(mid & CAN_IDMASK_EID)
    {   // extended frame
        switch(mid)
        {
            case BMS_22:
                dbgPrintf_canfd("BMS_22:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS22.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS22.hword[i] += (*data++) << 8;
                }
                temp = canmsgBMS22.bF.bms22_soc.high & 0x0F;
                temp <<= 7;
                temp += (canmsgBMS22.bF.bms22_soc.low >> 1);
                value = (float)temp/20;
                dPrintf_canfd("\tState of Charge: %4.2f %%", value);
                canfdio.id4DataIn.data.veh_stateCharge = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("BMS_22:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tState of Charge: %4.2f %%", value);
                }
                break;
            case KLIMA_16:
                dbgPrintf_canfd("KLIMA_16:\t");
                temp = data[3];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                temp = (temp >> 4) & 0x3;
                dPrintf_canfd("\tFSH status: %d", temp);
                canfdio.id4DataIn.data.veh_fsh = temp;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_16:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tFSH status: %u", temp);
                }
                break;
            case TEMP_01:
                dbgPrintf_canfd("TEMP_01:\t");
                temp = data[1];
                value = data[2];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value/2 - 50;
                dPrintf_canfd("\tOutside temperature: %5.2f �C", value);
                temp = (temp>>4) & 0x01;
                dPrintf_canfd(" | Qbit: %d", temp);
                canfdio.id4DataIn.data.veh_tempOutside = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("TEMP_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tOutside temperature: %3.2f �C", value);
                }
                break;
            default:
                dbgPrintf_canfd("Invalid data!\t");
                dbgPrintf_canfd("E:%X -", mid);
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                break;
        }
    }
    else
    {   // standard frame
        switch(mid >> CAN_EID_BITS)
        {
            case BMS_20:
                dbgPrintf_canfd("BMS_20:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS20.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS20.hword[i] += (*data++) << 8;
                }
                temp = canmsgBMS20.bF.bms20_vol.high;
                temp <<= 4;
                temp += (canmsgBMS20.bF.bms20_vol.low >> 4);
                value = (float)temp/4;
                dPrintf_canfd("\tBMS_Spannung: %4.2f V", value);
                canfdio.id4DataIn.data.veh_voltage = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("BMS_20:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tHigh Voltage: %3.2f V", value);
                }
                break;
            case ESP_21:
                dbgPrintf_canfd("ESP_21:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgESP21.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgESP21.hword[i] += (*data++) << 8;
                }
                temp = canmsgESP21.bitF.esp21_speed.high;
                value = (temp << 8) + canmsgESP21.bitF.esp21_speed.low;
                value = value / 100;
                dPrintf_canfd("\tSpeed: %4.2f KM/h", value);
                temp = canmsgESP21.bitF.esp21_speed.Qbit;
                dPrintf_canfd(" | Qbit: %d", temp);
                canfdio.id4DataIn.data.veh_speed = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("ESP_21:\n");
                    print_array_byte(data - num, num);
                    iPrintf("\tVehicle Speed: %3.2f KM/h", value);
                }
                break;
            case KLIMA_03:
                dbgPrintf_canfd("KLIMA_03:\t");
                value = data[4];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value/2 - 50;
                dPrintf_canfd("\tInside temperature: %4.2f �C", value);
                canfdio.id4DataIn.data.veh_tempInside = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_03:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tInside temperature: %3.2f �C", value);
                }
                break;
            case KLIMA_S_01:
                dbgPrintf_canfd("KLIMA_S_01:\t");
                value = data[5];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value /2 - 0.5;
                dPrintf_canfd("\tHumidity: %4.2f %%", value);
                canfdio.id4DataIn.data.veh_humidity = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_S_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tHumidity: %3.2f %%", value);
                }
                break;
            case SYSTEMINFO_01:
                dbgPrintf_canfd("SYSTEMINFO_01:\t");
                temp = data[4];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                dPrintf_canfd("\tSI_Bus_Identifikation: %d", temp&0xFF);
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("SYSTEMINFO_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tCAN bus identification: 0x%X", temp&0xFF);
                }

                break;
            default:
                dbgPrintf_canfd("Invalid data!\t");
                dbgPrintf_canfd("S:%X -", (int)(mid >> EID_BITS));
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                break;
        }
    }

    return status;
}

/* This is to interpret the messages from the tester */
void msg_canfd_interpret_id4(uint32_t mid, uint8_t *data, uint32_t num)
{
    uint32_t i;

    dbgPrintf_canfd("\n");
	switch(mid >> CAN_EID_BITS)
	{
		case ID_RCV_DATA:
			dbgPrintf_canfd("Params: %d | ", num);
			msg_canfd_copyData(num, CAN_DATA_IN_LEN_ID4, data, (uint8_t *)&canfdio.id4DataIn.byte);
			break;
		case ID_RCV_DATA_CFG:
			debugPrintf_canfd("Data cfg:\t");
			//debugPrintf("Data cfg:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_CFG_LEN, data, (uint8_t *)&canfdio.id4DataIn_Cfg.byte);
			//dPrintf("\n");  for(i=0; i<CAN_DATA_IN_CFG_LEN; i++) dPrintf("%02X ", canfdio.id4DataIn_Cfg.byte[i]);
			break;
		case ID_RCV_DATA_CTL:
			debugPrintf_canfd("Data ctl:\t");
			//debugPrintf("Data ctl:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_CTL_LEN, data, (uint8_t *)&canfdio.id4DataIn_Ctl.byte);
			//dPrintf("\n");  for(i=0; i<CAN_DATA_IN_CFG_LEN; i++) dPrintf("%02X ", canfdio.id4DataIn_Ctl.byte[i]);
			break;
		case ID_RCV_DATA_ENV:
			debugPrintf_canfd("Data env:\t");
			//debugPrintf("Data env:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_ENV_LEN, data, (uint8_t *)&canfdio.id4DataIn_Env.byte);
			break;
		case ID_RCV_DATA_RES:
			debugPrintf_canfd("Data res:\t");
			//debugPrintf("Data res:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_RES_LEN, data, (uint8_t *)&canfdio.id4DataIn_Res.byte);
			break;
		case ID_RCV_LOG:
			debugPrintf_canfd("Log:\t");
			msg_canfd_copyData(num, CAN_DATA_LOG_LEN_ID4, data, (uint8_t *)&canfdio.id4DataLog.byte);
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

void msg_canfd_clear_id4(void)
{
	for(uint32_t i=0; i<CAN_DATA_IN_LEN_ID4; i++)
	{
		canfdio.id4DataIn.byte[i] = 0;
	}
}

static int32_t msg_canfd_receiveConfigs_id4(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber;
	int32_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_LOG != (messageID >> CAN_EID_BITS)));

	ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);

	msg_canfd_copyData(dataNumber, CAN_DATA_IN_CFG_LEN, (uint8_t*)messageData, (uint8_t *)&canfdio.id4DataIn_Cfg.byte);

	return status;
}

static int32_t msg_canfd_receiveLog_id4(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber;
	int32_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_LOG != (messageID >> CAN_EID_BITS)));

	msg_canfd_copyData(dataNumber, CAN_DATA_LOG_LEN_ID4, (uint8_t*)messageData, (uint8_t *)&canfdio.id4DataLog.byte);

	return status;
}

void app_main_id4_sendCommand(sysData_type *sdata, int cmd)
{
	if((0 == sdata->canfd_status) && (canfdio.updated)) {
		ndPrintf("\n Sending data '%c' to CAN ...", cmd);
		msg_canfd_send_tester(sdata->project_id, (uint32_t)cmd);
		ndPrintf("\n data '%c' to CAN sent!", cmd);
		canfdio.updated = BOOL_FALSE;
	}
}

void app_main_id4_getMsginfo(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->interval = canfdio.id4_vehData[(int)msgi->mode].interval;
	msgi->num = canfdio.id4_vehData[(int)msgi->mode].num;

	ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", canfdio.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, \
			msgi->interval, msgi->num);					/* when controlling loop number of every single message */
	iPrintf("%s:\t%d\t%d\t | %d\n", canfdio.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_g3_getMsginfo(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->interval = canfdio_g3.id4_vehData[(int)msgi->mode].interval;
	msgi->num = canfdio_g3.id4_vehData[(int)msgi->mode].num;

	ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", canfdio_g3.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, \
			msgi->interval, msgi->num);					/* when controlling loop number of every single message */
	iPrintf("%s:\t%d\t%d\t | %d\n", canfdio_g3.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}

void app_main_g4r_getMsginfo(msg_opt_t *msgi)
{
	/* use the default interval and total number */
	msgi->interval = canfdio_g4r.id4_vehData[(int)msgi->mode].interval;
	msgi->num = canfdio_g4r.id4_vehData[(int)msgi->mode].num;

	ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", canfdio_g4r.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, \
			msgi->interval, msgi->num);					/* when controlling loop number of every single message */
	iPrintf("%s:\t%d\t%d\t | %d\n", canfdio_g4r.id4_vehData[(int)msgi->mode].name, \
			msgi->mode + 1, msgi->val, msgi->interval);
}
void app_main_id4_print(void)
{
    iPrintf("\n%s", canfdio.id4DataIn_Ctl.data.mode ? "deIce" : "deFog");
    iPrintf(" | %d", canfdio.id4DataIn_Ctl.data.hitCount);
    iPrintf(" | %d", canfdio.id4DataIn_Ctl.data.veh_fsh);
    iPrintf(" | %3.1fKM/h", (float)canfdio.id4DataIn_Env.data.veh_speed / 10);
    iPrintf(" | %3.1fV", (float)canfdio.id4DataIn_Env.data.veh_voltage / 10);
    iPrintf(" | %3.1f%%", (float)canfdio.id4DataIn_Env.data.veh_stateCharge / 10);
    iPrintf(" | tI%3.1fC", (float)canfdio.id4DataIn_Env.data.veh_tempInside / 100);
    iPrintf(" | tO%3.1fC", (float)canfdio.id4DataIn_Env.data.veh_tempOutside / 100);
    iPrintf(" | H%3.1f%%", (float)canfdio.id4DataIn_Env.data.veh_humidity / 100);
    iPrintf(" | vI%3.1fV", (float)canfdio.id4DataIn_Env.data.vin / 10);
    iPrintf(" | vO%3.1fV", (float)canfdio.id4DataIn_Res.data.vout / 10);
    iPrintf(" | cO%3.1fA", (float)canfdio.id4DataIn_Res.data.cout / 100);
    iPrintf(" | R%5.3fOhms", (float)canfdio.id4DataIn_Res.data.res / 1000);
    iPrintf(" | tT%3.1f", (float)canfdio.id4DataIn_Res.data.tempTro / 100);
    iPrintf(" | Tru%ds", canfdio.id4DataIn_Cfg.data.petdCanConfig.runtime);
    iPrintf(" | %dns", canfdio.id4DataIn_Cfg.data.pwmCanConfig.compensation_up3);
}

void app_main_id4_print_canVeh(int msgNum, int msgCount)
{
	iPrintf("%s: %3d | ", canfdio.id4_vehData[msgNum].name, msgCount);
}

void app_main_g3_print_canVeh(int msgNum, int msgCount)
{
	iPrintf("%s: %3d | ", canfdio_g3.id4_vehData[msgNum].name, msgCount);
}

void app_main_g4r_print_canVeh(int msgNum, int msgCount)
{
	iPrintf("%s: %3d | ", canfdio_g4r.id4_vehData[msgNum].name, msgCount);
}

int app_main_id4_commandP(void)
{
	ndPrintf("\r\n print command sent out");
	int32_t status = msg_canfd_receiveConfigs_id4();
	if(0 == status) {
			ndPrintf("\r\n Config received");
		pwmConfig_check(&canfdio.id4DataIn_Cfg.data.pwmCanConfig, pwmConfig_get(), BOOL_FALSE);
		pwmConfig_get()->configUpdated = BOOL_TRUE;
		petdConfig_check(&canfdio.id4DataIn_Cfg.data.petdCanConfig, petdConfig_get(), BOOL_FALSE);
		petdConfig_get()->configUpdated = BOOL_TRUE;
		pwmConfig_print();
		petdConfig_print();
	}
		getc(stdin);

		return 0;
}

void app_main_id4_displayLog(struct canfdData_id4 *candata)
{
	iPrintf("\n %d\t%d\t%d\t%d\t%d\t%d", \
			candata->id4DataLog.data.address, \
			candata->id4DataLog.data.index, \
			candata->id4DataLog.data.value1, \
			candata->id4DataLog.data.value2, \
			candata->id4DataLog.data.value3, \
			candata->id4DataLog.data.value4 \
			);
}

void app_main_id4_commandD_log(void)
{
	ndPrintf("\r\n request log command sent out");
	iPrintf("\n log data (position, No., values ...):");
	uint16_t position, number;
	int index = -1;
    position = canfdio.id4DataOut_command.data.addrStart;
    number = canfdio.id4DataOut_command.data.dataNum;

	for(;;) {
		memset((void*)&canfdio.id4DataLog.data, 0, CAN_DATA_LOG_LEN_ID4);
		//iPrintf("\n Start receiving log from %d of %d - %d", index, number, canfdio.id4DataLog.data.index);
		int32_t status = msg_canfd_receiveLog_id4();
		if(0 == status) {
			/* check if it is a new data: temporary use,
			 * not good enough for the second round
			 * as there are residual data from CAN bus */
			if(index < canfdio.id4DataLog.data.index) {
				/* new data */
				app_main_id4_displayLog(&canfdio);
				index = canfdio.id4DataLog.data.index;
				iPrintf("\t%d of %d", index, number);

				/* check if all data are received */
    			if(number-1 <= index) {
    				/* all data are received */
    				iPrintf("\n");
    				//iPrintf("\t break check: %d of %d", index, number);
    				return;
    			}
			}
		}
	}
}

int app_main_id4_commandD_error(void)
{
	ndPrintf("\r\n request error command sent out");
	iPrintf("\n error data (Error No., parameters ...):");

	return 0;
}

