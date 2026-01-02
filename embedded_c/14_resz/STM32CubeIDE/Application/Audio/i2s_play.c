/*
 * i2s_play.c
 *
 *  Created on: Dec 19, 2025
 *      Author: ivanovp
 */
#include "i2s_play.h"
#include "main.h"

#define I2S_MAX_CHUNK_SIZE 16384

static volatile uint16_t * i2s_data_ptr = NULL;
static volatile uint32_t i2s_data_size = 0;
static volatile uint16_t i2s_chunk_size;

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

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == &hi2s3)
	{
		if (i2s_data_size > 0u)
		{
			if (i2s_data_size > I2S_MAX_CHUNK_SIZE)
			{
				i2s_chunk_size = I2S_MAX_CHUNK_SIZE;
			}
			else
			{
				i2s_chunk_size = i2s_data_size;
			}
			HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)i2s_data_ptr, i2s_chunk_size);
			i2s_data_ptr += i2s_chunk_size;
			i2s_data_size -= i2s_chunk_size;
		}
	}
}

bool i2s_play_dma(const int16_t * i2s_data, uint32_t i2s_data_samples)
{
	HAL_StatusTypeDef status;
	bool ret = false;
	i2s_data_ptr = (uint16_t *)i2s_data;
	i2s_data_size = i2s_data_samples;

	if (i2s_data_size > I2S_MAX_CHUNK_SIZE)
	{
		i2s_chunk_size = I2S_MAX_CHUNK_SIZE;
	}
	else
	{
		i2s_chunk_size = i2s_data_size;
	}
	status = HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)i2s_data_ptr, i2s_chunk_size);
	i2s_data_ptr += i2s_chunk_size;
	i2s_data_size -= i2s_chunk_size;
	if (status == HAL_OK)
	{
		ret = true;
	}

	return ret;
}

bool i2s_is_playing(void)
{
	return (HAL_DMA_GetState(hi2s3.hdmatx) == HAL_DMA_STATE_BUSY);
}
