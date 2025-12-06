/*
 * seven_seg.h
 *
 *  Created on: Nov 15, 2025
 *      Author: ivanovp
 */

#ifndef APPLICATION_USER_SEVEN_SEG_H_
#define APPLICATION_USER_SEVEN_SEG_H_

#define SEVEN_SEG_DIGIT_NUM	4

#define SEG_A	4
#define SEG_B	16
#define SEG_C	2
#define SEG_D	128
#define SEG_E	64
#define SEG_F	1
#define SEG_G	8
#define SEG_DP	32

extern volatile uint8_t SSD_sevenSegBuf[SEVEN_SEG_DIGIT_NUM];
extern volatile bool SSD_enableDisplay;
extern volatile bool SSD_triggerUpdate;

extern const uint8_t hexDigits[16];

extern void displayHandler(void);
extern void SSD_printHexUint16LZ(uint16_t number);
extern bool SSD_printDecUint16LZ(uint16_t number);
extern bool SSD_printDecInt16LZ(int16_t number);
extern void SSD_printDecUint16(uint16_t number);
extern void SSD_printDecInt16(int16_t number);
extern void SSD_printDecFloat(float number, uint8_t fractNum);

#endif /* APPLICATION_USER_SEVEN_SEG_H_ */
