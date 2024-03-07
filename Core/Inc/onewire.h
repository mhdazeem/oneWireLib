/*
 * onewire.h
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#ifndef INC_ONEWIRE_H_
#define INC_ONEWIRE_H_

#include "genericHeader.h"

#define MAX_DEVICES						25 		// Maximum number of devices
#define ROM_ADDRESS_LENGTH				8 		// ROM address length
#define SCRATCH_PAD_LENGTH				9
#define ROM_DATA_LENGTH					64		// ROM data length
#define SENSOR_DIVIDER					16.0	// Sensor Temperature Conversion

// DS2482-800 commands
#define CMD_WCFG 0xD2  	// Write Configuration command code
#define CMD_DRST 0xF0  	// Device Reset
#define CMD_SRP  0xE1	// Set Read Pointer
#define CMD_CHSL 0xC3	// Channel Select
#define CMD_1WRS 0xB4	// 1-Wire Reset command
#define CMD_1WRB 0x96	// 1-Wire Read Byte
#define CMD_1WSB 0x87	// 1-Wire Single Bit
#define CMD_1WTS 0x78   // 1-Wire Triplet command
#define CMD_1WWB 0xA5   // 1-Wire Write Byte cmd


// DS18B20 commands
#define ONEWIRE_MATCH_ROM_CMD			0x55	// Select the device
#define ONEWIRE_READ_SCPD_CMD			0xBE	// Read scratchpad
#define ONEWIRE_WRITE_SCPD_CMD			0x4E	// write scratchpad
#define ONEWIRE_SKIP_ROM_CMD			0xCC	// Act like Broadcast
#define ONEWIRE_TEMP_CONV_CMD			0x44 	// temperature converison cmd
#define ONEWIRE_SEARCH_ROM_CMD 			0xF0	// search ROM Address cmd
#define ONEWIRE_READ_PSM_CMD			0xB4	// Read Power Supply Mode

// Scratchpad locations
#define SCPAD_TEMP_LSB					0
#define SCPAD_TEMP_MSB					1
#define SCPAD_HIGH_ALARM_TEMP			2
#define SCPAD_LOW_ALARM_TEMP 			3
#define SCPAD_CONFIGURATION				4
#define SCPAD_INTERNAL_BYTE				5
#define SCPAD_COUNT_REMAIN				6
#define SCPAD_COUNT_PER_C				7
#define SCPAD_CRC						8

#define SET_LOW_TEMP		20
#define SET_HIGH_TEMP		50

#define NOT_CONNECTED  -0.0625f

/* DS18B20 Sensor Resolution																		 */
#define TEMP_09_BIT_RESOLUTION			0x1F	//  9 bit
#define TEMP_10_BIT_RESOLUTION			0x3F	// 10 bit
#define TEMP_11_BIT_RESOLUTION			0x5F	// 11 bit
#define TEMP_12_BIT_RESOLUTION			0x7F	// 12 bit

/* ------------------------------------------------------------------------------------------------- */
/* DS18B20 Sensor Error Codes																		 */
/* ------------------------------------------------------------------------------------------------- */
#define DEVICE_WAITING_CONVERSION		85
#define DEVICE_DISCONNECTED_C			-127
#define DEVICE_DISCONNECTED_F			-196.6
#define DEVICE_DISCONNECTED_RAW			-7040

#define DS18B20_MIN_TEMP_LIMIT_C		-55 	// DS18B20 device identifier
#define DS18B20_MAX_TEMP_LIMIT_C		125 	// DS18B20 device identifier

#define LOW_ALARM_ON					1
#define HIGH_ALARM_ON					2

unsigned char OWTouchBit_W(unsigned char sendbit);

HAL_StatusTypeDef OW_WriteByte(unsigned char sendbyte);

unsigned char OW_ReadByte(void);

void oneWireResetSearch(void);

uint8_t oneWireSearch(uint8_t * deviceAddr);

uint8_t oneWireCRC8(uint8_t * deviceAddress, uint8_t dataLen);

void oneWireSelectDevice(uint8_t * deviceAddress);

int8_t oneWireSetResolutionAll(uint8_t * deviceAddress, uint8_t newResoluion);

int8_t oneWireSetResolution(uint8_t * deviceAddress, uint8_t newResoluion);

int8_t oneWireIsDeviceConnected(uint8_t * deviceAddress);

int8_t oneWireReadScratchpad(uint8_t * deviceAddress, uint8_t * scratchpadData);

uint8_t oneWireIsAllZeros(uint8_t * scratchPad);

void oneWireWriteScratchpad(uint8_t * deviceAddress);

int8_t oneWireSetLowTemperature(uint8_t * deviceAddress, int8_t temperatureValueC);

int8_t oneWireSetLowTemperatureAll(int8_t temperatureValueC, uint8_t * deviceAddress);

int8_t oneWireGetLowTemperature(uint8_t * deviceAddress);

int8_t oneWireSetHighTemperatureAll(int8_t temperatureValueC, uint8_t * deviceAddress);

int8_t oneWireSetHighTemperature(uint8_t * deviceAddress, int8_t temperatureValueC);

int8_t oneWireGetHighTemperature(uint8_t * deviceAddress);

uint8_t oneWireHasAlarm(uint8_t * deviceAddress);

unsigned char OWReadBit(void);

#endif /* INC_ONEWIRE_H_ */
