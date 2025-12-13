/*
 * utils.c
 *
 *  Created on: Dec 13, 2025
 *      Author: ivanovp
 */

#include "utils.h"

uint8_t hexChar2Bin(uint8_t hexChar, bool * error)
{
  uint8_t ret = 0u;

  *error = false;
  if (hexChar >= '0' && hexChar <= '9')
  {
    ret = hexChar - '0';
  }
  else if (hexChar >= 'A' && hexChar <= 'F')
  {
    ret = hexChar - 'A' + 0xAu;
  }
  else if (hexChar >= 'a' && hexChar <= 'f')
  {
    ret = hexChar - 'a' + 0xAu;
  }
  else
  {
    *error = true;
  }

  return ret;
}
