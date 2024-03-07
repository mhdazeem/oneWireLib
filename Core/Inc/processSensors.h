/*
 * getTemperature.h
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#include "genericHeader.h"

#ifndef INC_PROCESSSENSORS_H_
#define INC_PROCESSSENSORS_H_

#define INCREMENT_NUM							1
#define	TEMP_CONV_DELAY							600 // delay for conversion to complete - milli-seconds

#define TEMPERATURE_DATA_LEN					4

typedef enum {

	INITIALIZE_STATE,
	SEARCH_DEVICES,
	SET_RESOLUTION,
	RESET_MASTER,
	CONVERT_DATA,
	WAIT_FOR_DELAY,
	SELECT_DEVICE,
	READ_DATA,
	PROCESS_DATA,
	CLEAR_DB,
	ERROR_HANDLING,
	RESET_SMACHINE

} ProcessDevices;

typedef struct
{
    uint16_t bInitialization : 1;
    uint16_t bSearchDevices : 1;
    uint16_t bDataConversion : 1;
    uint16_t bReadData : 1;
    uint16_t bProcessData : 1;
    uint16_t bClearDB : 1;
    uint16_t bErrorHandling : 1;
    uint16_t bResetSMachine : 1;
    uint16_t : 8;

}StateMachineStatus;

typedef struct _oneWireDevSearch {

	uint8_t devicesAddressROM[MAX_DEVICES][ROM_ADDRESS_LENGTH];			/*!< 8-bytes address of last search device */
	uint8_t devicesScratchpadRawData[MAX_DEVICES][SCRATCH_PAD_LENGTH];			/*!< 8-bytes address of last search device */
	float deviceTemperatureData[MAX_DEVICES];
	uint8_t deviceAlarm[MAX_DEVICES];

	StateMachineStatus StateMachine_Status;

} oneWireDevSearchStruc;

typedef enum {

	INITIALIZE_PTC,
	SKIP_ROM_PTC,
	START_CONVERSION_PTC,
	WAIT_CONVERSION_PTC,
	SELECT_DEVICE_PTC,
	READ_SCRATCHPAD_PTC,
	PROCESS_DATA_PTC,
	RESETSM

} Temperature_C_State;

extern oneWireDevSearchStruc oneWireDevSearchDB[NUMBER_OF_CHANNELS];

extern uint8_t WHILELOOP_STATUS;

void processDevices(uint8_t CHANNEL);

void findOneWireDevices(uint8_t CURRENT_CHANNEL);

void readScratchPadMemory(void);

extern uint8_t numberOfDevices;

#endif /* INC_PROCESSSENSORS_H_ */
