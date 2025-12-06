/*
 * seven_seg.c
 *
 *  Created on: Nov 15, 2025
 *      Author: ivanovp
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "main.h"
#include "seven_seg.h"

volatile uint8_t SSD_sevenSegBuf[SEVEN_SEG_DIGIT_NUM] = { 0 };
volatile bool SSD_enableDisplay = true;
volatile bool SSD_triggerUpdate = true;

const uint8_t hexDigits[16] =
{
		SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
		SEG_B | SEG_C,									// 1
		SEG_A | SEG_B | SEG_G | SEG_D | SEG_E,			// 2
		SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,			// 3
		SEG_B | SEG_C | SEG_F | SEG_G,					// 4
		SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,			// 5
		SEG_A | SEG_F | SEG_G | SEG_C | SEG_D | SEG_E,  // 6
		SEG_A | SEG_B | SEG_C,  						// 7
		SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,	// 8
		SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,			// 9
		SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, 			// A
		SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,					// B
		SEG_A | SEG_D | SEG_E | SEG_F,							// C
		SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,					// D
		SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,					// E
		SEG_A | SEG_E | SEG_F | SEG_G							// F
};


static void setSegments(uint8_t segments)
{
	segments ^= 0xFF; // invert all bits due to negative logic
	if (segments & 1) // bit 0
	{
		HAL_GPIO_WritePin(LED_GREEN_0_GPIO_Port, LED_GREEN_0_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_0_GPIO_Port, LED_GREEN_0_Pin, GPIO_PIN_RESET);
	}
	if (segments & 2) // bit 1
	{
		HAL_GPIO_WritePin(LED_GREEN_1_GPIO_Port, LED_GREEN_1_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_1_GPIO_Port, LED_GREEN_1_Pin, GPIO_PIN_RESET);
	}
	if (segments & 4) // bit 2
	{
		HAL_GPIO_WritePin(LED_GREEN_2_GPIO_Port, LED_GREEN_2_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_2_GPIO_Port, LED_GREEN_2_Pin, GPIO_PIN_RESET);
	}
	if (segments & 8) // bit 3
	{
		HAL_GPIO_WritePin(LED_GREEN_3_GPIO_Port, LED_GREEN_3_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_3_GPIO_Port, LED_GREEN_3_Pin, GPIO_PIN_RESET);
	}
	if (segments & 16) // bit 4
	{
		HAL_GPIO_WritePin(LED_GREEN_4_GPIO_Port, LED_GREEN_4_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_4_GPIO_Port, LED_GREEN_4_Pin, GPIO_PIN_RESET);
	}
	if (segments & 32) // bit 5
	{
		HAL_GPIO_WritePin(LED_GREEN_5_GPIO_Port, LED_GREEN_5_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_5_GPIO_Port, LED_GREEN_5_Pin, GPIO_PIN_RESET);
	}
	if (segments & 64) // bit 6
	{
		HAL_GPIO_WritePin(LED_GREEN_6_GPIO_Port, LED_GREEN_6_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_6_GPIO_Port, LED_GREEN_6_Pin, GPIO_PIN_RESET);
	}
	if (segments & 128) // bit 7
	{
		HAL_GPIO_WritePin(LED_GREEN_7_GPIO_Port, LED_GREEN_7_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_GREEN_7_GPIO_Port, LED_GREEN_7_Pin, GPIO_PIN_RESET);
	}
}

static void setDigit(uint8_t digit)
{
	switch (digit)
	{
	case 0:
		HAL_GPIO_WritePin(DIGIT0_GPIO_Port, DIGIT0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT1_GPIO_Port, DIGIT1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT2_GPIO_Port, DIGIT2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT3_GPIO_Port, DIGIT3_Pin, GPIO_PIN_SET);
		break;

	case 1:
		HAL_GPIO_WritePin(DIGIT0_GPIO_Port, DIGIT0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT1_GPIO_Port, DIGIT1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT2_GPIO_Port, DIGIT2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT3_GPIO_Port, DIGIT3_Pin, GPIO_PIN_SET);
		break;

	case 2:
		HAL_GPIO_WritePin(DIGIT0_GPIO_Port, DIGIT0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT1_GPIO_Port, DIGIT1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT2_GPIO_Port, DIGIT2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT3_GPIO_Port, DIGIT3_Pin, GPIO_PIN_SET);
		break;

	case 3:
		HAL_GPIO_WritePin(DIGIT0_GPIO_Port, DIGIT0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT1_GPIO_Port, DIGIT1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT2_GPIO_Port, DIGIT2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT3_GPIO_Port, DIGIT3_Pin, GPIO_PIN_RESET);
		break;

	default:
		HAL_GPIO_WritePin(DIGIT0_GPIO_Port, DIGIT0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT1_GPIO_Port, DIGIT1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT2_GPIO_Port, DIGIT2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT3_GPIO_Port, DIGIT3_Pin, GPIO_PIN_SET);
		break;
	}
}

static uint8_t getSegmentCount(uint8_t segments)
{
	uint8_t i = 8u;
	uint8_t segmentCntr = 0;
	while (i--)
	{
		if (segments & 1u)
		{
			segmentCntr++;
		}
		segments >>= 1u;
	}

	return segmentCntr;
}

void displayHandler(void)
{
	static uint8_t digitCntr = 0;
	static uint8_t sevenSegBufMirror[SEVEN_SEG_DIGIT_NUM] = { 0 };
	static uint8_t prescaler = 0;
	static uint8_t brightness = 0u;
	uint8_t i;
	if (SSD_enableDisplay)
	{
		if (prescaler == 0)
		{
			digitCntr++;
			if (digitCntr >= SEVEN_SEG_DIGIT_NUM)
			{
				if (SSD_triggerUpdate)
				{
					for (i = 0; i < SEVEN_SEG_DIGIT_NUM; i++)
					{
						sevenSegBufMirror[i] = SSD_sevenSegBuf[i];
					}
					SSD_triggerUpdate = false;
				}
				digitCntr = 0;
			}
			setDigit(0xFF);
			setSegments(sevenSegBufMirror[digitCntr]);
			setDigit(digitCntr);
			brightness = getSegmentCount(sevenSegBufMirror[digitCntr]);
		}
		if (brightness == prescaler)
		{
			setDigit(0xFF);
		}
		if (prescaler == 8)
		{
			prescaler = 0;
		}
		else
		{
			prescaler++;
		}
  }
  else
  {
	setDigit(0xFF);
  }
}

void SSD_printHexUint16LZ(uint16_t number)
{
	SSD_sevenSegBuf[0] = hexDigits[number & 0xF];
	SSD_sevenSegBuf[1] = hexDigits[(number >> 4) & 0xF];
	SSD_sevenSegBuf[2] = hexDigits[(number >> 8) & 0xF];
	SSD_sevenSegBuf[3] = hexDigits[(number >> 12) & 0xF];
}

bool SSD_printDecUint16LZ(uint16_t number)
{
	bool ret = true;
	if (number <= 9999)
	{
		SSD_sevenSegBuf[0] = hexDigits[number % 10];
		number /= 10;
		SSD_sevenSegBuf[1] = hexDigits[number % 10];
		number /= 10;
		SSD_sevenSegBuf[2] = hexDigits[number % 10];
		number /= 10;
		SSD_sevenSegBuf[3] = hexDigits[number % 10];
	}
	else
	{
		SSD_sevenSegBuf[3] = 0;
		SSD_sevenSegBuf[2] = hexDigits[0xE];
		SSD_sevenSegBuf[1] = SEG_E | SEG_G;
		SSD_sevenSegBuf[0] = SEG_E | SEG_G;
		ret = false;
	}

	return ret;
}

bool SSD_printDecInt16LZ(int16_t number)
{
	bool ret = true;
	if (number >= 0)
	{
		ret = SSD_printDecUint16LZ(number);
	}
	else if (number < 0 && number >= -999)
	{
		ret = SSD_printDecUint16LZ(-1 * number);
		SSD_sevenSegBuf[3] = SEG_G;
	}
	else
	{
		SSD_sevenSegBuf[3] = 0;
		SSD_sevenSegBuf[2] = hexDigits[0xE];
		SSD_sevenSegBuf[1] = SEG_E | SEG_G;
		SSD_sevenSegBuf[0] = SEG_E | SEG_G;
		ret = false;
	}
	return ret;
}

void SSD_printDecUint16(uint16_t number)
{
	int i;

	if (number <= 9999)
	{
		for (i = 0; i < SEVEN_SEG_DIGIT_NUM; i++)
		{
			if (i == 0 || number > 0)
			{
				SSD_sevenSegBuf[i] = hexDigits[number % 10];
			}
			else
			{
				SSD_sevenSegBuf[i] = 0;
			}
			number /= 10;
		}
	}
	else
	{
		SSD_sevenSegBuf[3] = 0;
		SSD_sevenSegBuf[2] = hexDigits[0xE];
		SSD_sevenSegBuf[1] = SEG_E | SEG_G;
		SSD_sevenSegBuf[0] = SEG_E | SEG_G;
	}
}

/**
 * Prints a signed 16-bit integer to the seven segment display.
 *
 * @param[in] number Number to display.
 */
void SSD_printDecInt16(int16_t number)
{
	if (number >= 0)
	{
		/* Number is larger or equal to zero */
		SSD_printDecUint16(number);
	}
	else if (number >= -9)
	{
		/* Number is between -9 .. -1. */
		SSD_printDecUint16(-1 * number);
		SSD_sevenSegBuf[1] = SEG_G;
	}
	else if (number >= -99)
	{
		/* Number is between -99 .. -10. */
		SSD_printDecUint16(-1 * number);
		SSD_sevenSegBuf[2] = SEG_G;
	}
	else if (number >= -999)
	{
		/* Number is between -999 .. -100. */
		SSD_printDecUint16(-1 * number);
		SSD_sevenSegBuf[3] = SEG_G;
	}
	else
	{
		/* Number is less than -999, it cannot be displayed */
		/* Display ERR */
		SSD_sevenSegBuf[3] = 0;
		SSD_sevenSegBuf[2] = hexDigits[0xE];
		SSD_sevenSegBuf[1] = SEG_E | SEG_G;
		SSD_sevenSegBuf[0] = SEG_E | SEG_G;
	}
}

void SSD_printDecFloat(float number, uint8_t fractNum)
{
	int16_t intNumber = (int16_t)(number * pow10f(fractNum));
	if (SSD_printDecInt16LZ(intNumber))
	{
		SSD_sevenSegBuf[fractNum] |= SEG_DP;
	}
}
