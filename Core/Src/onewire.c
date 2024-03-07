/*
 * onewire.c
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#include "genericHeader.h"

oneWireDevSearchStruc oneWireDevSearchDB[NUMBER_OF_CHANNELS];

int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;

unsigned char deviceSearchAddress[ROM_ADDRESS_LENGTH];
uint8_t deviceScratchPad[SCRATCH_PAD_LENGTH];


/* ------------------------------------------------------------------------------------------------- */
/* Search device/s on the one-wire bus */
/* ------------------------------------------------------------------------------------------------- */
uint8_t oneWireSearch(uint8_t * deviceAddr)
{

	int rom_id_bit_number;
	int last_zero, rom_byte_number, search_result;
	int id_bit, cmp_id_bit;
	unsigned char rom_byte_mask, search_direction, status;

	// initialize for search
	rom_id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = FALSE;

	if (!LastDeviceFlag)
	{
		if (!DS2482_1W_reset())
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;
		}

		OW_WriteByte(ONEWIRE_SEARCH_ROM_CMD);

		do
		{
			// if this discrepancy if before the Last Discrepancy
			// on a previous next then pick the same as last time
			if (rom_id_bit_number < LastDiscrepancy)
			{
				if ((deviceSearchAddress[rom_byte_number] & rom_byte_mask) > 0)
				{
					search_direction = 1;
				}
				else
				{
					search_direction = 0;
				}
			}
			else
			{
				// if equal to last pick 1, if not then pick 0
				if (rom_id_bit_number == LastDiscrepancy)
				{
					search_direction = 1;
				}
				else
				{
					search_direction = 0;
				}
			}

			// Perform a triple operation on the DS2482 which will perform
			// 2 read bits and 1 write bit
			status = DS2482_search_triplet(search_direction);

			// check bit results in status byte
			id_bit = (status & ds2482_800Regs.STATUS_SBR);
			cmp_id_bit = (status & ds2482_800Regs.STATUS_TSB);
			search_direction = (status & ds2482_800Regs.STATUS_DIR) ? (uint8_t)1 : (uint8_t)0;

			if ((id_bit) && (cmp_id_bit))
			{
				break;
			}
			else
			{
				if ((!id_bit) && (!cmp_id_bit) && (search_direction == 0))
				{
					last_zero = rom_id_bit_number;

					// check for Last discrepancy in family
					if (last_zero < 9)
					{
						LastFamilyDiscrepancy = last_zero;
					}
				}

	            // set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask
				if (search_direction == 1)
				{
					deviceSearchAddress[rom_byte_number] |= rom_byte_mask;
				}
	            else
	            {
	            	deviceSearchAddress[rom_byte_number] &= (uint8_t)~rom_byte_mask;
	            }

				// increment the byte counter id_bit_number and shift the mask rom_byte_mask

				rom_id_bit_number++;
				rom_byte_mask <<= 1;


				// if the mask is 0 then go to new SerialNum byte rom_byte_number
				// and reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}

		while(rom_byte_number < ROM_ADDRESS_LENGTH);  // loop until through all ROM bytes 0-7

		// if the search was successful then

		if (!(rom_id_bit_number <= ROM_DATA_LENGTH))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag
	        // search_result

			LastDiscrepancy = last_zero;

	         // check for last device
	         if (LastDiscrepancy == 0)
	         {
	        	 LastDeviceFlag = TRUE;
	         }

	         search_result = TRUE;
		}
	}

	// if no device found then reset counters so next
	// 'search' will be like a first

	if (!search_result || !deviceSearchAddress[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = FALSE;
		LastFamilyDiscrepancy = 0;
		search_result = FALSE;
	}
	else
	{
		for (int index = ZERO; index < ROM_ADDRESS_LENGTH; index++)
		{
			deviceAddr[index] = deviceSearchAddress[index];
		}
	}

	return search_result;
}

void oneWireResetSearch(void)
{
	// reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = FALSE;
	LastFamilyDiscrepancy = 0;

	for (int i = ROM_ADDRESS_LENGTH - DECREMENT; ; i--)
	{
		deviceSearchAddress[i] = 0;
		if (i == 0)
		{
			break;
		}
	}
}

uint8_t oneWireCRC8(uint8_t * deviceAddress, uint8_t dataLen)
{

	uint8_t crcValue = ZERO;

	for (uint8_t i = ZERO; i < dataLen; i++) {

		uint8_t byte = deviceAddress[i];

		for (uint8_t j = ZERO; j < ROM_ADDRESS_LENGTH; j++) {

			uint8_t mix = (crcValue ^ byte) & 0x01;

			crcValue >>= 1;

			if (mix) {

				crcValue ^= 0x8C;
			}

			byte >>= 1;
		}
	}

	return crcValue;
}

int8_t oneWireSetResolutionAll(uint8_t * deviceAddress, uint8_t newResoluion)
{

	int8_t doneResolutionProcess = FALSE, isDeviceConnected = FALSE;

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_CONFIGURATION] = newResoluion;

		oneWireWriteScratchpad(deviceAddress);

	} else {

		doneResolutionProcess = isDeviceConnected;
	}

	return doneResolutionProcess;
}

int8_t oneWireSetResolution(uint8_t * deviceAddress, uint8_t newResoluion)
{

	int8_t doneResolutionProcess = FALSE, isDeviceConnected = FALSE;

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_CONFIGURATION] = newResoluion;

		oneWireWriteScratchpad(deviceAddress);

	} else {

		doneResolutionProcess = isDeviceConnected;
	}

	return doneResolutionProcess;
}

int8_t oneWireIsDeviceConnected(uint8_t * deviceAddress)
{

	int8_t isConnected = ERROR;

	isConnected = oneWireReadScratchpad(deviceAddress, deviceScratchPad);

	return isConnected &&
			!oneWireIsAllZeros(deviceScratchPad) &&
			(oneWireCRC8(deviceScratchPad, ROM_ADDRESS_LENGTH) == deviceScratchPad[SCPAD_CRC]);
}

int8_t oneWireReadScratchpad(uint8_t * deviceAddress, uint8_t * scratchpadData)
{
	DS2482_1W_reset();

	oneWireSelectDevice(deviceAddress);

	OW_WriteByte(ONEWIRE_READ_SCPD_CMD);

	for (int dataIndex = 0; dataIndex < SCRATCH_PAD_LENGTH; dataIndex++) {
		scratchpadData[dataIndex] = OW_ReadByte();
	}

	return DS2482_1W_reset();
}

void oneWireSelectDevice(uint8_t * deviceAddress)
{
	uint8_t romIndex;
	uint8_t devAdd;

	// Choose device
	OW_WriteByte(ONEWIRE_MATCH_ROM_CMD);

	for (romIndex = ZERO; romIndex < ROM_ADDRESS_LENGTH; romIndex++)
	{
		devAdd = deviceAddress[romIndex];

		OW_WriteByte(devAdd);
	}
}

uint8_t oneWireIsAllZeros(uint8_t * scratchPad)
{
	for (size_t i = 0; i < SCRATCH_PAD_LENGTH; i++) {

		if (scratchPad[i] != 0) {
			return FALSE;
		}
	}

	return TRUE;
}

int8_t oneWireGetHighTemperature(uint8_t * deviceAddress)
{

	uint8_t isDeviceConnected = FALSE;
	int8_t getDeviceTemperature = DEVICE_DISCONNECTED_C;

	isDeviceConnected = oneWireReadScratchpad(deviceAddress, deviceScratchPad);

	if (isDeviceConnected &&
			!oneWireIsAllZeros(deviceScratchPad) &&
			(oneWireCRC8(deviceScratchPad, ROM_ADDRESS_LENGTH) == deviceScratchPad[SCPAD_CRC])) {

		getDeviceTemperature = deviceScratchPad[SCPAD_HIGH_ALARM_TEMP];
	}
	else {

		return DEVICE_DISCONNECTED_C;

	}

	return getDeviceTemperature;
}

int8_t oneWireSetHighTemperature(uint8_t * deviceAddress, int8_t temperatureValueC)
{

	int8_t isDeviceConnected = FALSE, doneTemperatureProcess = DEVICE_DISCONNECTED_C;

	if (temperatureValueC < DS18B20_MIN_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MIN_TEMP_LIMIT_C;

	} else if (temperatureValueC > DS18B20_MAX_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MAX_TEMP_LIMIT_C;
	}

	if (oneWireGetHighTemperature(deviceAddress) == temperatureValueC) {
		return TRUE;
	}

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_HIGH_ALARM_TEMP] = (uint8_t) temperatureValueC;

		oneWireWriteScratchpad(deviceAddress);

		doneTemperatureProcess = TRUE;

	}

	return doneTemperatureProcess;
}

int8_t oneWireSetHighTemperatureAll(int8_t temperatureValueC, uint8_t * deviceAddress)
{

	int8_t isDeviceConnected = FALSE, doneTemperatureProcess = DEVICE_DISCONNECTED_C;

	if (temperatureValueC > DS18B20_MAX_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MAX_TEMP_LIMIT_C;
	}

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_HIGH_ALARM_TEMP] = (uint8_t) temperatureValueC;

		oneWireWriteScratchpad(deviceAddress);

	} else {

		doneTemperatureProcess = isDeviceConnected;
	}

	return doneTemperatureProcess;
}

int8_t oneWireGetLowTemperature(uint8_t * deviceAddress)
{
	uint8_t isDeviceConnected = FALSE;
	int8_t getDeviceTemperature = DEVICE_DISCONNECTED_C;

	isDeviceConnected = oneWireReadScratchpad(deviceAddress, deviceScratchPad);

	if (isDeviceConnected &&
			!oneWireIsAllZeros(deviceScratchPad) &&
			(oneWireCRC8(deviceScratchPad, ROM_ADDRESS_LENGTH) == deviceScratchPad[SCPAD_CRC])) {

		getDeviceTemperature = deviceScratchPad[SCPAD_LOW_ALARM_TEMP];
	}
	else {

		return DEVICE_DISCONNECTED_C;

	}

	return getDeviceTemperature;
}

int8_t oneWireSetLowTemperature(uint8_t * deviceAddress, int8_t temperatureValueC)
{

	int8_t isDeviceConnected = FALSE, doneTemperatureProcess = DEVICE_DISCONNECTED_C;

	if (temperatureValueC < DS18B20_MIN_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MIN_TEMP_LIMIT_C;

	} else if (temperatureValueC > DS18B20_MAX_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MAX_TEMP_LIMIT_C;
	}

	if (oneWireGetLowTemperature(deviceAddress) == temperatureValueC) {
		return TRUE;
	}

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_LOW_ALARM_TEMP] = (uint8_t) temperatureValueC;
		oneWireWriteScratchpad(deviceAddress);
		doneTemperatureProcess = TRUE;

	}

	return doneTemperatureProcess;
}

int8_t oneWireSetLowTemperatureAll(int8_t temperatureValueC, uint8_t * deviceAddress)
{

	int8_t isDeviceConnected = FALSE, doneTemperatureProcess = DEVICE_DISCONNECTED_C;

	if (temperatureValueC < DS18B20_MIN_TEMP_LIMIT_C) {

		temperatureValueC = DS18B20_MIN_TEMP_LIMIT_C;

	}

	isDeviceConnected = oneWireIsDeviceConnected(deviceAddress);

	if (isDeviceConnected) {

		deviceScratchPad[SCPAD_LOW_ALARM_TEMP] = (uint8_t) temperatureValueC;

		oneWireWriteScratchpad(deviceAddress);

	} else {

		doneTemperatureProcess = isDeviceConnected;
	}

	return doneTemperatureProcess;
}

unsigned char OWReadBit(void)
{
   return OWTouchBit_W(0x01);
}

unsigned char OWTouchBit_W(unsigned char sendbit)
{
	unsigned char status;
	int poll_count = 0;
	int POLL_LIMIT = 1000;
	uint8_t command[2];
	command[0] = CMD_1WSB;
	command[1] = (sendbit ? 0x80: 0x00);

	if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	  {
		do
		{
			status = DS2482_1W_status_REG();
		}
		while ((status & ds2482_800Regs.STATUS_1WB) && (poll_count++ < POLL_LIMIT));

		if (!(poll_count >= POLL_LIMIT))
		{
			if (status & ds2482_800Regs.STATUS_SBR)
			{
				return TRUE;
			}
		}
	  }
	return FALSE;
}

HAL_StatusTypeDef OW_WriteByte(unsigned char sendbyte)
{
	unsigned char status;
	int poll_count = 0;
	int POLL_LIMIT = 1000;
	uint8_t command[2];
	command[0] = CMD_1WWB;
	command[1] = sendbyte;

	if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	{
		do
		{
			status = DS2482_1W_status_REG();
		}
		while ((status & ds2482_800Regs.STATUS_1WB) && (poll_count++ < POLL_LIMIT));

		if (!(poll_count >= POLL_LIMIT))
		{
			return TRUE;
		}
	}

	return FALSE;
}

unsigned char OW_ReadByte(void)
{
	uint8_t i = ROM_ADDRESS_LENGTH, byte = ZERO;

	while (i--) {
			byte >>= 1;
			byte |= (OWTouchBit_W(0x01) << 7);
		}

	return byte;
}

uint8_t oneWireHasAlarm(uint8_t * deviceAddress)
{
	uint8_t alarmStatus = 0;

	oneWireWriteScratchpad(deviceAddress);

	// Read the temperature from the scratchpad memory (MSB and LSB)
	int16_t temperatureRaw = ((int16_t)deviceScratchPad[SCPAD_TEMP_MSB] << 8) | deviceScratchPad[SCPAD_TEMP_LSB];

	// Convert the raw temperature to degrees Celsius
	float temperatureC = (float)temperatureRaw / SENSOR_DIVIDER;

	// Check low alarm
	if (temperatureC <= (int8_t)deviceScratchPad[SCPAD_LOW_ALARM_TEMP])
		alarmStatus = LOW_ALARM_ON;

	// Check high alarm
	if (temperatureC >= (int8_t)deviceScratchPad[SCPAD_HIGH_ALARM_TEMP])
		alarmStatus = HIGH_ALARM_ON;

	// No alarm
	return alarmStatus;
}


void oneWireWriteScratchpad(uint8_t * deviceAddress)
{

	DS2482_1W_reset();

	oneWireSelectDevice(deviceAddress);
	OW_WriteByte(ONEWIRE_WRITE_SCPD_CMD);

	OW_WriteByte(deviceScratchPad[SCPAD_HIGH_ALARM_TEMP]);
	OW_WriteByte(deviceScratchPad[SCPAD_LOW_ALARM_TEMP]);

	OW_WriteByte(deviceScratchPad[SCPAD_CONFIGURATION]);

	DS2482_1W_reset();
}
