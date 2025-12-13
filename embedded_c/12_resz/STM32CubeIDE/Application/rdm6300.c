/*
 * rdm6300.c
 *
 *  Created on: Dec 13, 2025
 *      Author: ivanovp
 */
#include "main.h"
#include "rdm6300.h"
#include "utils.h"

#define SOF   0x02u
#define EOF   0x03u

uint64_t RDM6300_read(void)
{
  uint8_t buf[10];
  uint64_t ret = RDM6300_ERROR;
  uint64_t code = 0u;
  uint8_t checksum = 0u;
  uint8_t checksumCalc = 0u;
  HAL_StatusTypeDef status;
  uint8_t i;
  bool error = false;

  do
  {
    status = HAL_UART_Receive(&huart3, buf, 1u, HAL_MAX_DELAY);
  } while (buf[0] != SOF && status == HAL_OK);

  if (status == HAL_OK)
  {
    status = HAL_UART_Receive(&huart3, buf, sizeof(buf), HAL_MAX_DELAY);
    if (status == HAL_OK)
    {
      code = 0u;
      for (i = 0u; i < sizeof(buf) && !error; i++)
      {
        code <<= 4;
        code |= hexChar2Bin(buf[i], &error);
        if (i & 1)
        {
          checksumCalc ^= code & 0xFFu;
        }
      }

      if (!error)
      {
        status = HAL_UART_Receive(&huart3, buf, 3u, HAL_MAX_DELAY);
        if (status == HAL_OK && buf[2] == EOF)
        {
          checksum = hexChar2Bin(buf[0], &error);
          if (!error)
          {
            checksum <<= 4;
            checksum |= hexChar2Bin(buf[1], &error);

            if (!error && checksum == checksumCalc)
            {
              ret = code;
            }
          }
        }
      }
    }
  }

  return ret;
}
