/*
 * getTemperature.c
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#include "genericHeader.h"

uint8_t WHILELOOP_STATUS = ZERO;

// Local function to reset the parameters to search the devices again
void resetparameters(uint8_t CURRENT_CHANNEL);
int isDataReady(void);

// Global variable to keep track of the number of devices on the bus
uint8_t numberOfDevices = 0;

uint8_t processDataCount = 0;
uint8_t setResolutionCount = 0;
uint16_t conversionTimer = 0;

// Assuming numberOfDevices is the total number of devices
int isProcessDataDone(void)
{
    return (processDataCount >= numberOfDevices);
}

// Assuming numberOfDevices is the total number of devices
int isResolutionDone(void)
{
    return (setResolutionCount >= numberOfDevices);
}

void processDevices(uint8_t CURRENT_CHANNEL)
{
    static ProcessDevices state = INITIALIZE_STATE;
    static uint32_t convDelayTimer = 0;

    uint8_t temperatureDataLSB = 0, temperatureDataMSB = 0;
    int16_t temperatureData = 0;

    switch (state) {

    case INITIALIZE_STATE:

    	oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bInitialization = TRUE;

        //oneWireDevSearchDB.StateMachine_Status.bInitialization = TRUE;

        state = SEARCH_DEVICES;
        break;

    case SEARCH_DEVICES:

    	oneWireResetSearch();

    	findOneWireDevices(CURRENT_CHANNEL);

    	if (numberOfDevices > 0)
    	{
    		oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bSearchDevices = TRUE;

    		if (processDataCount == 0)
    		{
    			conversionTimer = TEMP_CONV_DELAY * 3;
    		}

    		state = SET_RESOLUTION;
    	}
    	else
    	{
    		state = RESET_SMACHINE;
    	}

    	break;

    case SET_RESOLUTION:

		// Send conversion command to the next device
		oneWireSetResolution(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[processDataCount], TEMP_11_BIT_RESOLUTION);  // select the device

		state = CONVERT_DATA;
		break;

    case CONVERT_DATA:

    	DS2482_1W_reset(); // reset master

		R_status[3] = DS2482_WriteConfig(CFG_STRONG_PU);

		// Start temperature conversion
		oneWireSelectDevice(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[processDataCount]); // select the device

		OW_WriteByte(ONEWIRE_TEMP_CONV_CMD);

		state = WAIT_FOR_DELAY;
		break;


	case WAIT_FOR_DELAY:

		// Wait for the non-blocking delay (e.g., 750 ms)
		if (HAL_GetTick() - convDelayTimer >= conversionTimer) {

			convDelayTimer = HAL_GetTick();

			R_status[3] = DS2482_WriteConfig(CFG_STANDER_PU);

			state = SELECT_DEVICE;
		}

		break;

	case SELECT_DEVICE:

		// Start temperature conversion

		DS2482_1W_reset(); // reset master
		oneWireSelectDevice(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[processDataCount]); // select the device

		state = READ_DATA;
		break;

	case READ_DATA:

		oneWireReadScratchpad(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[processDataCount], oneWireDevSearchDB[CURRENT_CHANNEL].devicesScratchpadRawData[processDataCount]); // Read scratchpad

		// Stay in CONVERT_DATA state until all devices have received the command
		oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bReadData = TRUE;

		state = PROCESS_DATA;
		break;

    case PROCESS_DATA:

    	// Send conversion command to the next device
		if (processDataCount <= numberOfDevices) {

			// Start temperature conversion

			temperatureDataLSB = oneWireDevSearchDB[CURRENT_CHANNEL].devicesScratchpadRawData[processDataCount][SCPAD_TEMP_LSB];
			temperatureDataMSB = oneWireDevSearchDB[CURRENT_CHANNEL].devicesScratchpadRawData[processDataCount][SCPAD_TEMP_MSB];

			temperatureData = ((int16_t)temperatureDataMSB << 8) | temperatureDataLSB;

			oneWireDevSearchDB[CURRENT_CHANNEL].deviceTemperatureData[processDataCount] = (float) temperatureData / SENSOR_DIVIDER;

			conversionTimer = TEMP_CONV_DELAY;

			if (oneWireDevSearchDB[CURRENT_CHANNEL].deviceTemperatureData[processDataCount] == DEVICE_WAITING_CONVERSION)
			{
				state = CONVERT_DATA;
			}else
			{
				state = SET_RESOLUTION;
				processDataCount++; // Increment conversion count
			}
		}

		// Stay in CONVERT_DATA state until all devices have received the command
		if (isProcessDataDone()) {

			oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bProcessData = TRUE;
			state = CLEAR_DB;
		}

        break;

    case CLEAR_DB:

    	oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bClearDB = TRUE;

        state = RESET_SMACHINE;
        break;

    case ERROR_HANDLING:

    	oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bErrorHandling = TRUE;

        state = RESET_SMACHINE;
        break;

    case RESET_SMACHINE:

    	oneWireDevSearchDB[CURRENT_CHANNEL].StateMachine_Status.bResetSMachine = TRUE;

        resetparameters(CURRENT_CHANNEL);

        WHILELOOP_STATUS = ONE;
        state = INITIALIZE_STATE;
        break;

    default:
        break;
    }
}

void resetparameters(uint8_t CURRENT_CHANNEL)
{
	numberOfDevices = 0;
	processDataCount = 0;

	memset(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM, 0, sizeof(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM));
}

void findOneWireDevices(uint8_t CURRENT_CHANNEL)
{
    int deviceIndex = 0;

    uint8_t discoveredDevices[MAX_DEVICES][ROM_ADDRESS_LENGTH];

    int discoveredDevicesCount = 0;

    for (deviceIndex = 0; deviceIndex < MAX_DEVICES; deviceIndex++)
    {
        if (oneWireSearch(discoveredDevices[deviceIndex]))
        {
            if (oneWireCRC8(discoveredDevices[deviceIndex], ROM_ADDRESS_LENGTH - 1) !=
            		discoveredDevices[deviceIndex][ROM_ADDRESS_LENGTH - 1])
            {
                continue; // Skip devices with CRC mismatch
            }

            if (discoveredDevices[deviceIndex][0] != DS18B20_FAMILY_CODE)
            {
                continue; // Skip devices that are not DS18B20 temperature sensors
            }

            // Check if the device is already in the discoveredDevices list
            int isDuplicate = 0;

            for (int i = 0; i < discoveredDevicesCount; i++)
            {
                if (memcmp(discoveredDevices[deviceIndex], oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[i], ROM_ADDRESS_LENGTH) == 0)
                {
                    isDuplicate = 1;
                    break; // Exit the loop if a duplicate is found
                }
            }

            if (!isDuplicate)
            {
                // Add the device to the discoveredDevices list
                memcpy(oneWireDevSearchDB[CURRENT_CHANNEL].devicesAddressROM[discoveredDevicesCount], discoveredDevices[deviceIndex], ROM_ADDRESS_LENGTH);
                discoveredDevicesCount++;

                // Increment the numberOfDevices only for non-duplicate devices
                numberOfDevices += 1;
            }
        }
    }
}

