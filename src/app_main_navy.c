/*
 * canfdComm.c
 *
 *  Created on: Dec. 21, 2024
 *  modified on Dec. 21, 2024 from app_canfd.c
 *      Author: Crane Shao
 */

#include "app_main_navy.h"
#include "app_config.h"
#include "utility.h"

#define FILTER_NUMBER				3	/* make sure to change this number accordingly */

static struct canfdData_navy canfdio = {
   .navyOut = { {0, 0, 0, 0}, },
   .navyIn = { {0, 0, 0, 0, 0, 0, 0, 0}, },
   .updated = BOOL_FALSE,
};

inline struct canfdData_navy *msg_canfd_getData_navy(void)   { return &canfdio; }

/* Function to prepare data for CAN submission - Navy project */
int msg_canfd_prepare_navy(uint32_t option, uint32_t *mid, uint8_t *data, uint32_t *num)
{
	uint32_t temp = 0;
	int ret = 0;

	switch(option)
	{
		case 'c': //ID_SEND_CONFIG:
			temp = ID_SEND_CONFIG;
			*mid = temp << CAN_EID_BITS;
			*num = 8;
			//for(i=0; i<*num; i++)	data[i] = (uint8_t)canfdOutput.config + i;
		    data[0] = ( powerConfig_get()->thRes ) & 0xFF;
		    data[1] = ( powerConfig_get()->thRes ) >> 8;
		    data[2] = ( powerConfig_get()->thVoltagePeak ) & 0xFF;
		    data[3] = ( powerConfig_get()->thVoltagePeak ) >> 8;
		    data[4] = ( powerConfig_get()->thCurrentPeak ) & 0xFF;
		    data[5] = ( powerConfig_get()->thCurrentPeak ) >> 8;
			ndebugPrintf("0x%X - %d", temp, data[0]);
			break;
		case 'f': //ID_SEND_COMMAND:
			temp = ID_SEND_COMMAND;
			*mid =
			*num = 1;
			if(3 >= dData.number) {
				data[0] = dData.number;
			}
			ndebugPrintf("0x%X - %d", temp, data[0]);
			break;
		case 'd': // ID_SEND_DATA:
		    temp = ID_SEND_DATA;
		    *mid = temp << CAN_EID_BITS;
		    *num = CAN_DATA_OUT_LEN_NAVY;
		    ndPrintf("\n len %d | ", *num);
		    for(uint32_t i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.navyOut.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

/* Function to interpret the CANFD data for Farview project */
void msg_canfd_interpret_navy(uint32_t mid, uint8_t *data, uint32_t num)
{
    uint32_t i;

    dbgPrintf_canfd("\n");
	switch(mid >> CAN_EID_BITS)
	{
		case ID_RCV_DATA:
			dbgPrintf_canfd("Params: %d | ", num);
			for(i=0; i<num; i++)
			{
				dbgPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
			}
			if(CAN_DATA_IN_LEN_NAVY <= num) {
				for(i=0; i<CAN_DATA_IN_LEN_NAVY; i++)
				{
					canfdio.navyIn.byte[i] = *(data+i);
				}
			}
			ndPrintf(" | %d\n", canfdio.navyIn.data.resWindshield);
			break;
		case ID_RCV_LOG:
			debugPrintf_canfd("Log:\t");
			for(i=0; i<num; i++)
			{
				dPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dPrintf_canfd("\n\t\t");
			}
			if(CAN_DATA_LOG_LEN_NAVY <= num) {
				for(i=0; i<CAN_DATA_LOG_LEN_NAVY; i++)
				{
					canfdio.navyLog.byte[i] = *(data+i);
				}
			}
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

void msg_canfd_clear_navy(void)
{
	for(uint32_t i=0; i<CAN_DATA_IN_LEN_NAVY; i++)
	{
		canfdio.navyIn.byte[i] = 0;
	}
}

void app_main_navy_print(void)
{
    iPrintf("\nSetup: ");
    iPrintf("mode %u, ", canfdio.navyIn.data.mode);
    iPrintf("Vp %5.1fV, ", (float)canfdio.navyIn.data.voltagePeak / 10);
    iPrintf("Vr %5.1fV, ", (float)canfdio.navyIn.data.voltageRms / 100);
    iPrintf("Cp %4.1fA, ", (float)canfdio.navyIn.data.currentPeak / 100);
    iPrintf("Cr %4.1fA, ", (float)canfdio.navyIn.data.currentRms / 100);
    iPrintf("Rw %4.1fOhms, ", (float)canfdio.navyIn.data.resWindshield / 100);
    iPrintf("Tw %4.1f°C | ", (float)canfdio.navyIn.data.temp01 / 100);
    iPrintf("State %d, ", canfdio.navyIn.data.state);
}
