/*
 * i2s_play.h
 *
 *  Created on: Dec 19, 2025
 *      Author: ivanovp
 */

#ifndef APPLICATION_AUDIO_I2S_PLAY_H_
#define APPLICATION_AUDIO_I2S_PLAY_H_

#include <stdint.h>
#include <stdbool.h>

extern bool i2s_play(const int16_t * i2s_data, uint32_t i2s_data_samples);
extern bool i2s_play_dma(const int16_t * i2s_data, uint32_t i2s_data_samples);
extern bool i2s_is_playing(void);

#endif /* APPLICATION_AUDIO_I2S_PLAY_H_ */
