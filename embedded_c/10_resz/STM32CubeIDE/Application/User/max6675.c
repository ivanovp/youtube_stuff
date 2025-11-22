/*
 * max6675.c
 *
 *  Created on: Nov 22, 2025
 *      Author: ivanovp
 */
#include <stdbool.h>

#include "main.h"
#include "max6675.h"

static void spi_delay(void)
{
	HAL_Delay(1);
}

static void set_spi_cs_low(void)
{
	HAL_GPIO_WritePin(SPI_SCK_GPIO_Port, SPI_SCK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
}

static void set_spi_cs_high(void)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

static void set_spi_sck_low(void)
{
	HAL_GPIO_WritePin(SPI_SCK_GPIO_Port, SPI_SCK_Pin, GPIO_PIN_RESET);
}

static void set_spi_sck_high(void)
{
	HAL_GPIO_WritePin(SPI_SCK_GPIO_Port, SPI_SCK_Pin, GPIO_PIN_SET);
}

static bool is_spi_miso_high(void)
{
	return HAL_GPIO_ReadPin(SPI_MISO_GPIO_Port, SPI_MISO_Pin) == GPIO_PIN_SET;
}

uint16_t spi_read(void)
{
	uint8_t i;
	uint16_t spiData = 0u;

	set_spi_cs_low();
	spi_delay();

	for (i = 0u; i < 13u; i++)
	{
		spiData <<= 1u;
		set_spi_sck_high();
		spi_delay();
		if (is_spi_miso_high())
		{
			spiData |= 1u;
		}
		set_spi_sck_low();
		spi_delay();
	}

	set_spi_cs_high();
	return spiData;
}

float TEMP_read(void)
{
	uint16_t spiData = spi_read();
	float temp_C = (float)(spiData >> 2u) + ((spiData >> 1u) & 1u) * 0.5f + (spiData & 1u) * 0.25f;

	return temp_C;
}
