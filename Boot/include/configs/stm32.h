#ifndef __CONFIG_H
#define __CONFIG_H


#define APP_ADDR_START		0x68000000	/* extern sram base address */
#define BLOCK_SIZE				4096				/* spi flash */
#define NUM_BYTES_ADDR		0x001F0000	/* number of bytes download from ymodem */


typedef unsigned long ulong;

#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
	#define debug(fmt,args...)	printf (fmt ,##args) 
#else
	#define debug(fmt,args...)
#endif


#define	CFG_CBSIZE						256			/* Console I/O Buffer Size */
#define	CFG_MAXARGS						16			/* Max number of command args	*/
#define CFG_LONGHELP
#define CFG_PROMPT						"#"
#define CFG_LOAD_ADDR					0x68000000
#define CFG_MEMTEST_START			0x68000000
#define CFG_MEMTEST_END				0x68001000


#define CFG_COMMANDS					1
#define CFG_CMD_DATE					1
#define	CFG_CMD_LOAD					1
#define CFG_CMD_LOADB					1
#define	CFG_CMD_MMC						0
#define CFG_CMD_MEMORY				1
//#define CFG_NO_FLASH					1


#define CONFIG_BOOTDELAY			10
#define CONFIG_DIAG						/* should  comment when on board*/
#define CONFIG_BOOT_MOVINAND

#define CONFIG_COMMANDS				CFG_COMMANDS


#define disable_interrupts() __disable_irq()

#endif
