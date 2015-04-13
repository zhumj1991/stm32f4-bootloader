#ifndef  __INCLUDES_H
#define  __INCLUDES_H


#include "stm32f4xx.h"

/*
 * STANDARD LIBRARIES
 */
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include	<string.h>
#include  <math.h>



/*
 * BSP
 */
#include "bsp.h"


/*
 * BOOT
 */
#include "stm32.h"
#include "command.h"
#include "console.h"
#include "xyzmodem.h"
#include "date.h"
#include "spi_flash.h"

void udelay(uint32_t nTime);
int readline (const char *const prompt);


#endif
