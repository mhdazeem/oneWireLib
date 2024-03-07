/*
 * DS2482_800.h
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#include "genericHeader.h"

ds2482RegsStatus ds2482_800Regs;

uint8_t short_detected;
uint8_t status_REG[8];

unsigned char DS2482_search_triplet(int search_direction)
{
	unsigned char status;
	int poll_count = 0;
	int POLL_LIMIT = 1000;
	uint8_t command[2];
	command[0] = CMD_1WTS;
	command[1] = (search_direction ? 0x80 : 0x00);

	if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	{
			do
			{
				status = DS2482_1W_status_REG();
			}
				while ((status & ds2482_800Regs.STATUS_1WB) && (poll_count++ < POLL_LIMIT));

		if (!(poll_count >= POLL_LIMIT))
		{
			return status;
		}
	}
		return status;
}

int DS2482_1W_reset()
{
	unsigned char status;
	int poll_count = 0;
	int POLL_LIMIT = 10000;
	uint8_t command[1] = {CMD_1WRS};

	if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	{
		HAL_I2C_Init(&hi2c1);

		do
		{
			status = DS2482_1W_status_REG();
		}
		while ((status & ds2482_800Regs.STATUS_1WB) && (poll_count++ < POLL_LIMIT));

		if (!(poll_count >= POLL_LIMIT))
		{
			if (status & ds2482_800Regs.STATUS_SD)
			{
				short_detected = TRUE;
			}
			else
			{
				short_detected = FALSE;
			}

			// check for presence detect
			if (status & ds2482_800Regs.STATUS_PPD)
			{
				return TRUE;
			}
			else
			{
				DS2482_reset();
				return FALSE;
			}
		}
	}

	DS2482_reset();

	return FALSE;
}

int DS2482_select_channel(uint8_t channel)
{

	unsigned char read_ch, ch_read;
	uint8_t command[2] = {CMD_CHSL, 0};

	  switch (channel) {
	    case 0:
	      command[1] = 0xF0;
	      ch_read = 0xB8;
	      break;
	    case 1:
	      command[1] = 0xE1;
	      ch_read = 0xB1;
	      break;
	    case 2:
	      command[1] = 0xD2;
	      ch_read = 0xAA;
	      break;
	    case 3:
	      command[1] = 0xC3;
	      ch_read = 0xA3;
	      break;
	    case 4:
	      command[1] = 0xB4;
	      ch_read = 0x9C;
	      break;
	    case 5:
	      command[1] = 0xA5;
	      ch_read = 0x95;
	      break;
	    case 6:
	      command[1] = 0x96;
	      ch_read = 0x8E;
	      break;
	    case 7:
	      command[1] = 0x87;
	      ch_read = 0x87;
	      break;
	    default:
	      return FALSE;
	  }

	  if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	  {
		  if(HAL_I2C_Master_Receive(&hi2c1, DS2482_ADDRESS, (uint8_t *) &read_ch, ONE, HAL_MAX_DELAY) == HAL_OK)
		  {
			  if(read_ch == ch_read)
			  {
				  return TRUE;
			  }
		  }
	  }
	  DS2482_reset();
	  return TRUE;
}

int DS2482_reset()
{
	uint8_t command[1] = {CMD_DRST};
	if(HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, command, sizeof(command), HAL_MAX_DELAY) == HAL_OK)
	{
		return HAL_OK;
	}
	return HAL_ERROR;
}

int DS2482_1W_status_REG(void)
{
	uint8_t readreg;
	uint8_t data[2];
	data[0] = CMD_SRP;
	data[1] = CMD_DRST;

	if(HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, data, sizeof(data), HAL_MAX_DELAY) == HAL_OK)
	{
		if(HAL_I2C_Master_Receive(&hi2c1, DS2482_ADDRESS, (uint8_t *) &readreg, ONE, HAL_MAX_DELAY) == HAL_OK)
		{
			for (uint8_t i = 0; i < 8; ++i)
			{
				status_REG[i] = (readreg >> i) & 0x01;
			}

			ds2482_800Regs.STATUS_1WB = status_REG[0];
			ds2482_800Regs.STATUS_PPD = status_REG[1];
			ds2482_800Regs.STATUS_SD  = status_REG[2];
			ds2482_800Regs.STATUS_LL  = status_REG[3];
			ds2482_800Regs.STATUS_RST = status_REG[4];
			ds2482_800Regs.STATUS_SBR = status_REG[5];
			ds2482_800Regs.STATUS_TSB = status_REG[6];
			ds2482_800Regs.STATUS_DIR = status_REG[7];

			return TRUE;
		}
	}
	return FALSE;
}

HAL_StatusTypeDef DS2482_WriteConfig(uint8_t config)
{
	unsigned char read_config;
    uint8_t data[2];
    data[0] = CMD_WCFG;
    data[1] = config | (~config << 4);

    if(HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, data, sizeof(data), HAL_MAX_DELAY) == HAL_OK)
    {
    	if(HAL_I2C_Master_Receive(&hi2c1, DS2482_ADDRESS, (uint8_t *) &read_config, ONE, HAL_MAX_DELAY) == HAL_OK)
    	{
    		if(read_config == config)
    		{
    			return HAL_OK;
    		}
    	}
    }

    DS2482_reset();

    return HAL_ERROR;
}

HAL_StatusTypeDef DS2482_ReadConfig_MODE(uint8_t *config)
{
	uint8_t setReadPointerCommand[2] = {CMD_SRP, CMD_CHSL};

    if (HAL_I2C_Master_Transmit(&hi2c1, DS2482_ADDRESS, setReadPointerCommand, TWO, HAL_MAX_DELAY) == HAL_OK)
    {
    	if (HAL_I2C_Master_Receive(&hi2c1, DS2482_ADDRESS, config, ONE, HAL_MAX_DELAY) == HAL_OK)
    	{
    		return HAL_OK;
    	}
    }
    return HAL_ERROR;
}
