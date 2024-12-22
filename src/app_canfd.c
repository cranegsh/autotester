/*
 * canfdComm.c
 *
 *  Created on: Dec. 13, 2022
 *  modified on Aug 15, 2023 to be ported to Raspberry Pi
 *      Author: Crane Shao
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <PCANBasic.h>

#include "app_canfd.h"
#include "utility.h"
#include "sysconfig.h"
#include "app_config.h"
#include "app_main_c3.h"
#include "app_main_id4.h"
#include "app_main_navy.h"

uint32_t canfd_DlcToDataBytes(CAN_DLC dlc)
{
    uint32_t dataBytesInObject = 0;

    if (dlc < CAN_DLC_12) {
        dataBytesInObject = dlc;
    } else {
        switch (dlc) {
            case CAN_DLC_12:
                dataBytesInObject = 12;
                break;
            case CAN_DLC_16:
                dataBytesInObject = 16;
                break;
            case CAN_DLC_20:
                dataBytesInObject = 20;
                break;
            case CAN_DLC_24:
                dataBytesInObject = 24;
                break;
            case CAN_DLC_32:
                dataBytesInObject = 32;
                break;
            case CAN_DLC_48:
                dataBytesInObject = 48;
                break;
            case CAN_DLC_64:
                dataBytesInObject = 64;
                break;
            default:
                break;
        }
    }

    return dataBytesInObject;
}

CAN_DLC canfd_DataBytesToDlc(uint32_t n)
{
	CAN_DLC dlc = CAN_DLC_0;

    if (n <= 4) {
        dlc = CAN_DLC_4;
    } else if (n <= 8) {
        dlc = CAN_DLC_8;
    } else if (n <= 12) {
        dlc = CAN_DLC_12;
    } else if (n <= 16) {
        dlc = CAN_DLC_16;
    } else if (n <= 20) {
        dlc = CAN_DLC_20;
    } else if (n <= 24) {
        dlc = CAN_DLC_24;
    } else if (n <= 32) {
        dlc = CAN_DLC_32;
    } else if (n <= 48) {
        dlc = CAN_DLC_48;
    } else if (n <= 64) {
        dlc = CAN_DLC_64;
    }

    return dlc;
}

static int32_t canfd_messageReceive(uint32_t *mid, uint8_t *data, uint32_t *num)
{
    int32_t status = -100;
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    TPCANMsg Message;
    TPCANTimestamp ts;
    TPCANTimestamp ts_prev;
    TPCANTimestamp ts_diff;
#else
    TPCANMsgFD Message;
    TPCANTimestampFD ts;
    TPCANTimestampFD ts_prev;
    TPCANTimestampFD ts_diff;
#endif
    TPCANStatus Status;

#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    Status = CAN_Read(PCAN_DEVICE, &Message, &ts);
#else
    Status = CAN_ReadFD(PCAN_DEVICE, &Message, &ts);
#endif

    if(PCAN_ERROR_OK == Status)
    {   // Copy the message
    	status = 0;
		*mid = (int)Message.ID;
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
		*num = (uint16_t)(Message.LEN);
#else
		*num = canfd_DlcToDataBytes((CAN_DLC)Message.DLC);
#endif
		for(uint32_t i=0; i<*num; i++)
			data[i] = Message.DATA[i];
    }

    return status;
}

/* Function to send a message by calling another function to request send after loading message */
static int32_t canfd_messageSend(uint32_t mid, uint8_t *data, uint32_t num)
{
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    TPCANMsg Message;
#else
    TPCANMsgFD Message;
#endif
    TPCANStatus Status;

    // Initialize ID and control
    if(0 == (mid & CAN_IDMASK_EID)) {
    	Message.ID = (mid & CAN_IDMASK_SID) >> CAN_EID_BITS;   			// set Standard ID (first 11 bits)
    	Message.MSGTYPE = PCAN_MESSAGE_STANDARD;          				// set Standard type
    }
    else {
        Message.ID = mid;   								// set Extended ID (total 29 bits)
        Message.MSGTYPE = PCAN_MESSAGE_EXTENDED;					// set Extended type
    }
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    Message.LEN = (BYTE)(canfd_DataBytesToDlc((uint8_t)num));
    ndPrintf("\r\nmid %8X, DLC %02d", Message.ID, Message.LEN);
#else
    Message.DLC = (BYTE)(canfd_DataBytesToDlc(num));
    Message.MSGTYPE |= PCAN_MESSAGE_FD;
    Message.MSGTYPE |= PCAN_MESSAGE_BRS;
    ndPrintf("\r\nmid %8X, DLC %02d", Message.ID, Message.DLC);
#endif

    // Initialize transmit data
    uint32_t i;
    for(i=0; i<num; i++) {
        Message.DATA[i] = data[i];
    }

	// transmit message
	i = 0;
	do {
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
		Status = CAN_Write(PCAN_DEVICE, &Message);
#else
		Status = CAN_WriteFD(PCAN_DEVICE, &Message);
#endif
		if(PCAN_ERROR_OK != Status) {
			usleep(100);                  					// Check every 100us
			i++;
		}
	}
	while((PCAN_ERROR_OK != Status) && (200 > i));       			// Checking if message is sent for 20ms

	/* check if the data are submitted successfully */
	if (PCAN_ERROR_OK == Status) {
		ndPrintf("\t[Sent] %X | 0x%02X", Message.ID, Message.DATA[0]);
		return 0;
	}
	else {
		ndPrintf("\t[Failed to send] %X | 0x%02X", Message.ID, Status);
		return -1;
	}

    //return (int32_t)Status;		// The value is not used other than judging 0 and non-0 in caller
}

/* Function to send data as from vehicle for test */
void msg_canfd_send_veh(msg_mode_t msgno, int32_t value)
{
    uint32_t messageID = 0;
    uint8_t messageData[CAN_VEH_MSG_LEN];
    uint32_t dataNumber = CAN_VEH_MSG_LEN;
    uint32_t i;
    int32_t status = 0;

    /* clear the data buffer */
    for(i=0; i<dataNumber; i++) { messageData[i] = 0;	}

    /* prepare the data frame */
#ifdef PROJECT_CAN_ID4
    status = msg_canfd_prepare_id4Veh(msgno, value, messageData);
#endif
#ifdef PROJECT_CAN_G3
    status = msg_canfd_prepare_g3Veh(msgno, value, messageData);
#endif
#ifdef PROJECT_CAN_BZ4X
    status = msg_canfd_prepare_bz4xVeh(msgno, value, messageData);
#endif
#ifdef PROJECT_C3
    status = msg_canfd_prepare_c3Veh(msgno, value, messageData);
#endif
    if(0 != status)
    {
    	ndPrintf("\nData not ready!");
    	return;
    }

#ifdef PROJECT_ID4
    messageID = msg_canfd_getData_id4()->id4_vehData[(int)msgno].mid;
#endif
#ifdef PROJECT_C3
    messageID = msg_canfd_getData_c3()->c3canDataInfo[(int)msgno].mid;
#endif
    status = canfd_messageSend(messageID, messageData, dataNumber);
    ndPrintf("\n Status %d, Sent message: 0x%X, %d | ", status, messageID >> EID_BITS, dataNumber);

    if(0 != status)
    {	/* submission failed. Resend the data! */
    	// TODO: Need to find out why sometimes there are messages with ID of 0x1FFFFFFF and all data are 0xFF
    	ndPrintf("\nSubmission failed: ID-0x%X | %d\t", messageID >> EID_BITS, dataNumber);
    	ndPrintf("F.");
    }
    else
    {	/* submission done. Display the message! */
    	ndPrintf(" / Sent message: 0x%X, %d | ", messageID >> EID_BITS, dataNumber);
		for(i=0; i<dataNumber; i++)
		{
			ndPrintf(" %02X", messageData[i]);
			if((0 == i%16) && (0 < i)) ndPrintf("\n\t");
		}
		ndPrintf("\n");
    }
}

/* Function to send data - Farview project */
void msg_canfd_send_tester(uint32_t cmd)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber = 0;
    uint32_t i;
    int32_t status = 0;

    /* clear the data buffer */
    for(i=0; i<MAX_DATA_BYTES; i++) { messageData[i] = 0;	}

    /* prepare the data frame */
#ifdef PROJECT_ID4
    status = msg_canfd_prepare_id4(cmd, &messageID, messageData, &dataNumber);
#endif
#ifdef PROJECT_NAVY
    status = msg_canfd_prepare_navy(cmd, &messageID, messageData, &dataNumber);
#endif
    if(0 != status)
    {
    	ndPrintf("\nData not ready!");
    	return;
    }

    status = canfd_messageSend(messageID, messageData, dataNumber);
    ndPrintf("\n Status %d, Sent message: 0x%X, %d | ", status, messageID >> EID_BITS, dataNumber);

    if(0 != status)
    {	/* submission failed. Resend the data! */
    	// TODO: Need to find out why sometimes there are messages with ID of 0x1FFFFFFF and all data are 0xFF
    	ndPrintf("\nSubmission failed: ID-0x%X | %d\t", messageID >> EID_BITS, dataNumber);
    	ndPrintf("F.");
    }
    else
    {	/* submission done. Display the message! */
    	ndPrintf(" / Sent message: 0x%X, %d | ", messageID >> EID_BITS, dataNumber);
		for(i=0; i<dataNumber; i++)
		{
			ndPrintf(" %02X", messageData[i]);
			if((0 == i%16) && (0 < i)) ndPrintf("\n\t");
		}
		ndPrintf("\n");
    }
}

void msg_canfd_receive(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber;
    int32_t status;
    static uint32_t timer_sec = 0;
    time_t now;
    struct tm *systime;
    uint32_t i;

    time( &now );
    systime = localtime( &now );

    status = canfd_messageReceive(&messageID, messageData, &dataNumber);
    /* TODO: investigate Why there is garbage messages with message ID of 0? */

    if((0 == status) && ( 0 != messageID))
    {
		ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
		//print_array_byte(messageData, dataNumber); dPrintf("\n");

		/* check data: wrong data if the number of data is odd or larger than max CANFD data length */
		if((0 == (dataNumber % 2 )) && (MAX_DATA_BYTES >= dataNumber))
		{	 /* valid, so process the message */
#ifdef PROJECT_ID4
			 msg_canfd_interpret_id4(messageID, messageData, dataNumber);
#endif
#ifdef PROJECT_NAVY
			 msg_canfd_interpret_navy(messageID, messageData, dataNumber);
#endif
		}

		timer_sec = systime->tm_sec;
    }
    else if (1 < systime->tm_sec - timer_sec) {
    	/* didn't receive any message after 1 second, set all data 0 */
#ifdef PROJECT_ID4
			msg_canfd_clear_id4();
#endif
#ifdef PROJECT_C3
			msg_canfd_clear_c3();
#endif
#ifdef PROJECT_NAVY
			msg_canfd_clear_navy();
#endif
		timer_sec = systime->tm_sec;
    }

    return;
}

int32_t msg_canfd_init(void)
{
	 int32_t retVal = -1;
     TPCANStatus Status;

#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
     Status = CAN_Initialize(PCAN_DEVICE, PCAN_BAUD_500K, 0, 0, 0);
     printf("CAN_Initialize(%xh): Status=0x%x\n", PCAN_DEVICE, (int)Status);
#else
     Status = CAN_InitializeFD(PCAN_DEVICE, CANFD_BIT_RATE);
     printf("CANFD_Initialize(%xh): Status=0x%x\n", PCAN_DEVICE, (int)Status);
#endif

	 if(PCAN_ERROR_OK == Status)
	 {
		 //canfd_configCheck();
#ifdef DEVELOP_VERSION
		 infoPrintf("CAN bus is initialized successfully!\r\n");
#endif
		 iPrintf("\r\nCAN functions are available!\n");
	 }
	 else
	 {
		 infoPrintf("CAN bus initialization failed!\n");
		 iPrintf("\r\nCAN functions are NOT available!\n");
		 retVal = 0;
	 }

	 return retVal;
}

void msg_canfd_copyData(uint32_t number, uint32_t length, uint8_t *source, uint8_t *dest)
{
	uint32_t i;
	for(i=0; i<number; i++)
	{
		dbgPrintf_canfd(" %02X", *(source+i));
		ndPrintf(" %02X", *(source+i));
		if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
	}
	ndPrintf("\nReceived %d for %d", number, length);
	if(length <= number)
	{
		ndPrintf("\n Copying ...")
		for(i=0; i<length; i++)
		{
			dest[i] = *(source+i);
			ndPrintf(" %02X", dest[i]);
		}
	}
}

/* TODO: avoid blocking */
int32_t msg_canfd_rcvCanConfigs(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber;
	int32_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_DATA_CFG != (messageID >> CAN_EID_BITS)));

	ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
#ifdef PROJECT_ID4
	msg_canfd_copyConfigs_id4(dataNumber, (uint8_t*)messageData);
#endif

	return status;
}

/* TODO: avoid blocking */
int32_t msg_canfd_rcvCanLog(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint32_t dataNumber;
	int32_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_LOG != (messageID >> CAN_EID_BITS)));

	ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
#ifdef PROJECT_ID4
	msg_canfd_copyConfigs_id4(dataNumber, (uint8_t*)messageData);
#endif	/* #ifdef PROJECT_ID4 */

	return status;
}
