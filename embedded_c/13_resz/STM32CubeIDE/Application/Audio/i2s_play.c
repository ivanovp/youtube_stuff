/*
 * i2s_play.c
 *
 *  Created on: Dec 19, 2025
 *      Author: ivanovp
 */
#include "i2s_play.h"
#include "main.h"

#define I2S_MAX_CHUNK_SIZE 16384

bool i2s_play(const int16_t * i2s_data, uint32_t i2s_data_samples)
{
	HAL_StatusTypeDef status;
	bool ret = false;
	uint16_t * data_ptr = (uint16_t *)i2s_data;
	uint32_t data_size = i2s_data_samples;
	uint16_t chunk_size;

	do
	{
		if (data_size > I2S_MAX_CHUNK_SIZE)
		{
			chunk_size = I2S_MAX_CHUNK_SIZE;
		}
		else
		{
			chunk_size = data_size;
		}
		status = HAL_I2S_Transmit(&hi2s3, data_ptr, chunk_size, HAL_MAX_DELAY);
		data_ptr += chunk_size;
		data_size -= chunk_size;
	} while (status == HAL_OK && data_size > 0);
	if (status == HAL_OK)
	{
		ret = true;
	}

	return ret;
}
