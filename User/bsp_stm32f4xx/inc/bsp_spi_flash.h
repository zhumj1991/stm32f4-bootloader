#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H

#include "stm32f4xx.h"

#define SF_MAX_PAGE_SIZE	(4 * 1024)


#define CMD_AAI       	0xAD  	/* AAI 连续编程指令(FOR SST25VF016B) */



#define CMD_WREN      	0x06		/* 写使能命令 */
#define CMD_EWRSR	  		0x50		/* 允许写状态寄存器的命令 */
#define CMD_DISWR	  		0x04		/* 禁止写, 退出AAI状态 */
#define CMD_RDSR      	0x05		/* 读状态寄存器命令 */
#define CMD_WRSR      	0x01  	/* 写状态寄存器命令 */
#define CMD_READ      	0x03  	/* 读数据区命令 */
#define CMD_RDID      	0x9F		/* 读器件ID命令 */
#define CMD_SE        	0x20		/* 擦除扇区命令 */
#define CMD_BE        	0xC7		/* 批量擦除命令 */
#define CMD_POWER_DOWN	0xB9
#define	CMD_POWER_UP		0xAB
#define DUMMY_BYTE			0xA5		/* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG				0x01		/* 状态寄存器中的正在编程标志（WIP) */

/* 定义串行Flash ID */
enum
{
	SST25VF016B_ID	= 0xBF2541,
	MX25L1606E_ID		= 0xC22015,
	W25Q64BV_ID			= 0xEF4017,
	W25Q128FV_ID		= 0xEF4018
};

typedef struct
{
	uint32_t chipID;			/* 芯片ID */
	char chip_name[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t total_size;	/* 总容量 */
	uint16_t sector_size;
}spi_flash_info;

uint8_t bsp_spi_flash_init(void);
uint32_t spi_flash_read_chipID(void);
void spi_flash_erase_chip(void);

void spi_flash_erase_sector(uint32_t sector_addr);
void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size);

uint8_t spi_flash_write_buffer(uint8_t *buf, uint32_t write_addr, uint16_t size);
void spi_flash_read_buffer(uint8_t *buf, uint32_t read_addr, uint32_t size);

#endif
