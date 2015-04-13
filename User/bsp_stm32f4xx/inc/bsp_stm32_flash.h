#ifndef __BSP_STM32_FLASH_H
#define __BSP_STM32_FLASH_H

#include "stm32f4xx.h"

int stm32_flash_write(uint32_t offset, uint8_t *buf, uint16_t len);
int stm32_flash_read(uint32_t offset, uint8_t *buf, uint16_t len);
int stm32_flash_erase(uint32_t offset, uint16_t len);

#endif
