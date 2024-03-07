/*
 * DS2482_800.c
 *
 *  Created on: Jan 23, 2024
 *      Author: azeemmuhammad
 */

#include "genericHeader.h"

/* USER CODE BEGIN Includes */

#define DS2482_ADDRESS 0x18 << 1
#define MAX_DS2482_CHANNELS 1
#define DS18B20_FAMILY_CODE 0x28

#define CFG_1WS 0x00 		//   1-Wire speed (c1WS) = standard (0)
#define CFG_SPU 0x00 		//   Strong pullup (cSPU) = off (0)
#define CFG_PPM 0x00 		//	 Presence pulse masking (cPPM) = off (0)
#define CFG_APU 0x00 		//   Active pullup (cAPU) = on (CONFIG_APU = 0x01)

// DS2482-800 1-Wire Status Register Commands

typedef struct _ds2482RegsStatus {

    uint8_t STATUS_1WB;
    uint8_t STATUS_PPD;
    uint8_t STATUS_SD;
    uint8_t STATUS_LL;
    uint8_t STATUS_RST;
    uint8_t STATUS_SBR;
    uint8_t STATUS_TSB;
    uint8_t STATUS_DIR;

} ds2482RegsStatus;

extern ds2482RegsStatus ds2482_800Regs;

#define CFG_MODE_STANDARD   0x00
#define CFG_MODE_OVERDRIVE  0x01

#define CFG_STANDER_PU 	0x00 //   (c1WS | cSPU | cPPM | cAPU)
#define CFG_STRONG_PU 	0x04 //   (c1WS | cSPU | cPPM | cAPU)

#define NUMBER_OF_CHANNELS	8		// Total Number of Channels

/* USER CODE BEGIN Private defines */
int DS2482_select_channel(uint8_t channel);
HAL_StatusTypeDef DS2482_WriteConfig(uint8_t config);
HAL_StatusTypeDef DS2482_ReadConfig_MODE();
int DS2482_reset();
int DS2482_1W_reset();
int DS2482_1W_status_REG(void);
unsigned char DS2482_search_triplet(int search_direction);
