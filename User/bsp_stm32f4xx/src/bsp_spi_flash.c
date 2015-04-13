/*
*********************************************************************************************************
*
*	模块名称 : SPI接口串行FLASH 读写模块
*	文件名称 : bsp_spi_flash.c
*	版    本 : V1.0
*	说    明 : 支持 SST25VF016B、MX25L1606E 和 W25Q64BVSSIG
*			   		 SST25VF016B 的写操作采用AAI指令，可以提高写入效率。
*********************************************************************************************************
*/
#include "includes.h"

/*
	STM32F4XX 时钟计算.
		HCLK = 168M
		PCLK1 = HCLK / 4 = 42M
		PCLK2 = HCLK / 2 = 84M

		SPI2、SPI3 在 PCLK1, 时钟42M
		SPI1       在 PCLK2, 时钟84M

		STM32F4 支持的最大SPI时钟为 37.5 Mbits/S, 因此需要分频。
*/



#define SPI_FLASH			SPI1
#define SPI_BAUD			SPI_BaudRatePrescaler_4
#define SF_CS_GPIO		GPIOC
#define SF_CS_PIN			GPIO_Pin_5


#define SF_CS_LOW()       SF_CS_GPIO->BSRRH = SF_CS_PIN
#define SF_CS_HIGH()      SF_CS_GPIO->BSRRL = SF_CS_PIN


spi_flash_info flash_info;


void spi_flash_read_info(void);
static uint8_t spi_flash_send_byte(uint8_t value);
static void spi_flash_write_enable(void);
static void spi_flash_write_status(uint8_t value);
static uint8_t spi_flash_wait_ready(void);
static uint8_t spi_flash_need_erase(uint8_t *old_buf, uint8_t *new_buf, uint16_t len);
static uint8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
static uint8_t spi_flash_cmp_data(uint32_t src_addr, uint8_t *tar, uint32_t size);


static uint8_t spi_buf[4 * 1024];	/* 用于写函数，先读出整个sector，修改缓冲区后，再整个page回写 */


/*
*********************************************************************************************************
*	函 数 名: bsp_InitSpiFlash
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t bsp_spi_flash_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	uint8_t ret;

	/* COnfigure GPIOs */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = SF_CS_PIN;
	GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);

	SF_CS_HIGH();											/* 片选置高，不选中 */

	/* 配置SPI硬件参数用于访问串行Flash */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* 禁止SPI  */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;												/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;										/* 数据位长度 ： 8位 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;												/* 时钟的第1个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;														/* 片选控制方式：软件控制 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;									/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;									/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;														/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);
	SPI_Cmd(SPI_FLASH, ENABLE);			/* 使能SPI  */

	spi_flash_read_info();					/* 自动识别芯片型号 */

	SF_CS_LOW();										/* 软件方式，使能串行Flash片选 */
	spi_flash_send_byte(CMD_DISWR);	/* 发送禁止写入的命令,即使能软件写保护 */
	SF_CS_HIGH();										/* 软件方式，禁能串行Flash片选 */

	ret = spi_flash_wait_ready();					/* 等待串行Flash内部操作完成 */
	spi_flash_write_status(0);			/* 解除所有BLOCK的写保护 */
	
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_read_chipID
*	功能说明: 读取器件ID
*	形    参: 无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/
uint32_t spi_flash_read_chipID(void)
{
	uint32_t ID;
	uint8_t id1, id2, id3;

	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_RDID);									/* 发送读ID命令 */
	id1 = spi_flash_send_byte(DUMMY_BYTE);					/* 读ID的第1个字节 */
	id2 = spi_flash_send_byte(DUMMY_BYTE);					/* 读ID的第2个字节 */
	id3 = spi_flash_send_byte(DUMMY_BYTE);					/* 读ID的第3个字节 */
	SF_CS_HIGH();									/* 禁能片选 */

	ID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return ID;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_read_info
*	功能说明: 读取器件ID,并填充器件参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void spi_flash_read_info(void)
{
	/* 自动识别串行Flash型号 */
	{
		flash_info.chipID = spi_flash_read_chipID();		/* 芯片ID */

		switch (flash_info.chipID)
		{
			case SST25VF016B_ID:
				strcpy(flash_info.chip_name, "SST25VF016B");
				flash_info.total_size = 2 * 1024 * 1024;/* 总容量 = 2M */
				flash_info.sector_size = 4 * 1024;				/* 页面大小 = 4K */
				break;

			case MX25L1606E_ID:
				strcpy(flash_info.chip_name, "MX25L1606E");
				flash_info.total_size = 2 * 1024 * 1024;/* 总容量 = 2M */
				flash_info.sector_size = 4 * 1024;				/* 页面大小 = 4K */
				break;

			case W25Q64BV_ID:
				strcpy(flash_info.chip_name, "W25Q64BV");
				flash_info.total_size = 8 * 1024 * 1024;/* 总容量 = 8M */
				flash_info.sector_size = 4 * 1024;
				break;
			
			case W25Q128FV_ID:
				strcpy(flash_info.chip_name, "W25Q128FV");
				flash_info.total_size = 16 * 1024 * 1024;/* 总容量 = 16M */
				flash_info.sector_size = 4 * 1024;
				break;

			default:
				strcpy(flash_info.chip_name, "Unknow Flash");
				flash_info.total_size = 2 * 1024 * 1024;
				flash_info.sector_size = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_erase_sector
*	功能说明: 擦除指定的扇区
*	形    参: sector_addr : 扇区地址
*	返 回 值: 无
*********************************************************************************************************
*/
void spi_flash_erase_sector(uint32_t sector_addr)
{
	spi_flash_write_enable();			/* 发送写使能命令 */

	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_SE);													/* 发送擦除命令 */
	spi_flash_send_byte((sector_addr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	spi_flash_send_byte((sector_addr & 0xFF00) >> 8);			/* 发送扇区地址中间8bit */
	spi_flash_send_byte(sector_addr & 0xFF);							/* 发送扇区地址低8bit */
	SF_CS_HIGH();									/* 禁能片选 */

	spi_flash_wait_ready();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_erase_chip
*	功能说明: 擦除整个芯片
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void spi_flash_erase_chip(void)
{
	spi_flash_write_enable();			/* 发送写使能命令 */

	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_BE);		/* 发送整片擦除命令 */
	SF_CS_HIGH();									/* 禁能片选 */

	spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_write_enable
*	功能说明: 向器件发送写使能命令
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void spi_flash_write_enable(void)
{
	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_WREN);	/* 发送命令 */
	SF_CS_HIGH();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_write_status
*	功能说明: 写状态寄存器
*	形    参: value : 状态寄存器的值
*	返 回 值: 无
*********************************************************************************************************
*/
static void spi_flash_write_status(uint8_t value)
{

	if (flash_info.chipID == SST25VF016B_ID)
	{
		/* 第1步：先使能写状态寄存器 */
		SF_CS_LOW();									/* 使能片选 */
		spi_flash_send_byte(CMD_EWRSR);/* 发送命令， 允许写状态寄存器 */
		SF_CS_HIGH();									/* 禁能片选 */

		/* 第2步：再写状态寄存器 */
		SF_CS_LOW();									/* 使能片选 */
		spi_flash_send_byte(CMD_WRSR);	/* 发送命令， 写状态寄存器 */
		spi_flash_send_byte(value);		/* 发送数据：状态寄存器的值 */
		SF_CS_HIGH();									/* 禁能片选 */
	}
	else
	{
		SF_CS_LOW();									/* 使能片选 */
		spi_flash_send_byte(CMD_WRSR);	/* 发送命令， 写状态寄存器 */
		spi_flash_send_byte(value);		/* 发送数据：状态寄存器的值 */
		SF_CS_HIGH();									/* 禁能片选 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_wait_ready
*	功能说明: 采用循环查询的方式等待器件内部写操作完成
*	形    参:  无
*	返 回 值: 0 成功， 1 超时
*********************************************************************************************************
*/
static uint8_t spi_flash_wait_ready(void)
{
	uint32_t timeout = 100000; 
	
	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_RDSR);/* 发送命令， 读状态寄存器 */
	while(((spi_flash_send_byte(DUMMY_BYTE) & WIP_FLAG) == SET) && (timeout--))	{ /* 判断状态寄存器的忙标志位 */
		udelay(10);
	}
	SF_CS_HIGH();									/* 禁能片选 */
	
	if(timeout)
		return 0;
	else
		return 1;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_cmp_data
*	功能说明: 比较Flash的数据.
*	形    参: tar 			: 数据缓冲区
*						src_addr	：Flash地址
*						size 			：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
static uint8_t spi_flash_cmp_data(uint32_t src_addr, uint8_t *tar, uint32_t size)
{
	uint8_t value;

	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((src_addr + size) > flash_info.total_size)
	{
		return 1;
	}

	if (size == 0)
	{
		return 0;
	}

	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_READ);							/* 发送读命令 */
	spi_flash_send_byte((src_addr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	spi_flash_send_byte((src_addr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	spi_flash_send_byte(src_addr & 0xFF);					/* 发送扇区地址低8bit */
	while (size--)
	{
		/* 读一个字节 */
		value = spi_flash_send_byte(DUMMY_BYTE);
		if (*tar++ != value)
		{
			SF_CS_HIGH();
			return 1;
		}
	}
	SF_CS_HIGH();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_need_erase
*	功能说明: 判断写PAGE前是否需要先擦除。
*	形    参: old_buf ：旧数据
*						new_buf ：新数据
*						len 		：数据个数，不能超过页面大小
*	返 回 值: 0 : 不需要擦除， 1 ：需要擦除
*********************************************************************************************************
*/
static uint8_t spi_flash_need_erase(uint8_t *old_buf, uint8_t *new_buf, uint16_t len)
{
	uint16_t i;
	uint8_t old;

	/*
	算法第1步：old 求反, new 不变
	    old    new
		  1101   0101
	~   1111
		= 0010   0101

	算法第2步: old 求反的结果与 new 位与
		  0010   old
	&	  0101   new
		 =0000

	算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
	*/

	for (i = 0; i < len; i++)
	{
		old = *old_buf++;
		old = ~old;

		/* 注意错误的写法: if (ucOld & (*new_buf++) != 0) */
		if ((old & (*new_buf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_auto_write_sector
*	功能说明: 写1个PAGE并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参: buf 				: 数据源缓冲区；
*						write_addr	：目标区域首地址
*						size 				：数据个数，不能超过页面大小
*	返 回 值: 0 : 错误， 1 ： 成功
*********************************************************************************************************
*/
static uint8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t i;
	uint16_t j;					/* 用于延时 */
	uint32_t first_addr;	/* 扇区首址 */
	uint8_t need_erase;		/* 1表示需要擦除 */
	uint8_t ret;

	/* 长度为0时不继续操作,直接认为成功 */
	if (size == 0)
	{
		return 1;
	}

	/* 如果偏移地址超过芯片容量则退出 */
	if (write_addr >= flash_info.total_size)
	{
		return 0;
	}

	/* 如果数据长度大于扇区容量，则退出 */
	if (size > flash_info.sector_size)
	{
		return 0;
	}

	/* 如果FLASH中的数据没有变化,则不写FLASH */
	spi_flash_read_buffer(spi_buf, write_addr, size);
	if (memcmp(spi_buf, buf, size) == 0)
	{
		return 1;
	}

	/* 判断是否需要先擦除扇区 */
	/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
	need_erase = 0;
	if (spi_flash_need_erase(spi_buf, buf, size))
	{
		need_erase = 1;
	}

	first_addr = write_addr & (~(flash_info.sector_size - 1));

	if (size == flash_info.sector_size)		/* 整个扇区都改写 */
	{
		for	(i = 0; i < flash_info.sector_size; i++)
		{
			spi_buf[i] = buf[i];
		}
	}
	else						/* 改写部分数据 */
	{
		/* 先将整个扇区的数据读出 */
		spi_flash_read_buffer(spi_buf, first_addr, flash_info.sector_size);

		/* 再用新数据覆盖 */
		i = write_addr & (flash_info.sector_size - 1);
		memcpy(&spi_buf[i], buf, size);
	}

	/* 写完之后进行校验，如果不正确则重写，最多3次 */
	ret = 0;
	for (i = 0; i < 3; i++)
	{

		/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
		if (need_erase == 1)
		{
			spi_flash_erase_sector(first_addr);		/* 擦除1个扇区 */
		}

		/* 编程一个PAGE */
		spi_flash_write_sector(spi_buf, first_addr, flash_info.sector_size);

		if (spi_flash_cmp_data(write_addr, buf, size) == 0)
		{
			ret = 1;
			break;
		}
		else
		{
			if (spi_flash_cmp_data(write_addr, buf, size) == 0)
			{
				ret = 1;
				break;
			}

			/* 失败后延迟一段时间再重试 */
			for (j = 0; j < 10000; j++);
		}
	}

	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_write_sector
*	功能说明: 向一个sector内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参: buf 			: 数据源缓冲区；
*						WriteAddr ：目标区域首地址
*						Size 			：数据个数，不能超过页面大小
*	返 回 值: 无
*********************************************************************************************************
*/
void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint32_t i, j;

	if (flash_info.chipID == SST25VF016B_ID)
	{
		/* AAI指令要求传入的数据个数是偶数 */
		if ((size<2) && (size%2))
		{
			return ;
		}

		spi_flash_write_enable();			/* 发送写使能命令 */

		SF_CS_LOW();									/* 使能片选 */
		spi_flash_send_byte(CMD_AAI);												/* 发送AAI命令(地址自动增加编程) */
		spi_flash_send_byte((write_addr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
		spi_flash_send_byte((write_addr & 0xFF00) >> 8);			/* 发送扇区地址中间8bit */
		spi_flash_send_byte(write_addr & 0xFF);							/* 发送扇区地址低8bit */
		spi_flash_send_byte(*buf++);													/* 发送第1个数据 */
		spi_flash_send_byte(*buf++);													/* 发送第2个数据 */
		SF_CS_HIGH();									/* 禁能片选 */

		spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */

		size -= 2;										/* 计算剩余字节数 */

		for (i = 0; i<(size/2); i++)
		{
			SF_CS_LOW();								/* 使能片选 */
			spi_flash_send_byte(CMD_AAI);											/* 发送AAI命令(地址自动增加编程) */
			spi_flash_send_byte(*buf++);												/* 发送数据 */
			spi_flash_send_byte(*buf++);												/* 发送数据 */
			SF_CS_HIGH();								/* 禁能片选 */
			spi_flash_wait_ready();/* 等待串行Flash内部写操作完成 */
		}

		/* 进入写保护状态 */
		SF_CS_LOW();
		spi_flash_send_byte(CMD_DISWR);
		SF_CS_HIGH();

		spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */
	}
	else	/* for MX25L1606E 、 W25Q64BV */
	{
		for (j=0; j<(size/256); j++)
		{
			spi_flash_write_enable();								/* 发送写使能命令 */

			SF_CS_LOW();									/* 使能片选 */
			spi_flash_send_byte(0x02);														/* 发送AAI命令(地址自动增加编程) */
			spi_flash_send_byte((write_addr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
			spi_flash_send_byte((write_addr & 0xFF00) >> 8);			/* 发送扇区地址中间8bit */
			spi_flash_send_byte(write_addr & 0xFF);							/* 发送扇区地址低8bit */

			for (i=0; i<256; i++)
			{
				spi_flash_send_byte(*buf++);												/* 发送数据 */
			}

			SF_CS_HIGH();								/* 禁止片选 */

			spi_flash_wait_ready();/* 等待串行Flash内部写操作完成 */

			write_addr += 256;
		}

		/* 进入写保护状态 */
		SF_CS_LOW();
		spi_flash_send_byte(CMD_DISWR);
		SF_CS_HIGH();

		spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */
	}
}

void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size)
{
	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_READ);												/* 发送读命令 */
	spi_flash_send_byte((read_addr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	spi_flash_send_byte((read_addr & 0xFF00) >> 8);				/* 发送扇区地址中间8bit */
	spi_flash_send_byte(read_addr & 0xFF);								/* 发送扇区地址低8bit */
	while (size--)
	{
		*buf++ = spi_flash_send_byte(DUMMY_BYTE);					/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	SF_CS_HIGH();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_write_buffer
*	功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参: buf 				: 数据源缓冲区；
*						write_addr	：目标区域首地址
*						size 				：数据个数，不能超过页面大小
*	返 回 值: 1 : 成功， 0 ： 失败
*********************************************************************************************************
*/
uint8_t spi_flash_write_buffer(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t NumOfSector = 0, NumOfSingle = 0, add = 0, count = 0, temp = 0;

	add = write_addr % flash_info.sector_size;
	count = flash_info.sector_size - add;
	NumOfSector =  size / flash_info.sector_size;
	NumOfSingle = size % flash_info.sector_size;

	if (add == 0) /* 起始地址是页面首地址  */
	{
		if (NumOfSector == 0) /* 数据长度小于页面大小 */
		{
			if (spi_flash_auto_write_sector(buf, write_addr, size) == 0)
			{
				return 0;
			}
		}
		else 	/* 数据长度大于等于页面大小 */
		{
			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size) == 0)
				{
					return 0;
				}
				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}
			if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle) == 0)
			{
				return 0;
			}
		}
	}
	else  /* 起始地址不是页面首地址  */
	{
		if (NumOfSector == 0) /* 数据长度小于页面大小 */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (spi_flash_auto_write_sector(buf, write_addr, count) == 0)
				{
					return 0;
				}

				write_addr +=  count;
				buf += count;

				if (spi_flash_auto_write_sector(buf, write_addr, temp) == 0)
				{
					return 0;
				}
			}
			else
			{
				if (spi_flash_auto_write_sector(buf, write_addr, size) == 0)
				{
					return 0;
				}
			}
		}
		else	/* 数据长度大于等于页面大小 */
		{
			size -= count;
			NumOfSector =  size / flash_info.sector_size;
			NumOfSingle = size % flash_info.sector_size;

			if (spi_flash_auto_write_sector(buf, write_addr, count) == 0)
			{
				return 0;
			}

			write_addr +=  count;
			buf += count;

			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size) == 0)
				{
					return 0;
				}
				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}

			if (NumOfSingle != 0)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle) == 0)
				{
					return 0;
				}
			}
		}
	}
	
	return 1;	/* 成功 */
}

/*
*********************************************************************************************************
*	函 数 名: spi_flash_read_buffer
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参: buf 			: 数据源缓冲区；
*						read_addr ：首地址
*						size 			：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
void spi_flash_read_buffer(uint8_t *buf, uint32_t read_addr, uint32_t size)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((size == 0) ||(read_addr + size) > flash_info.total_size)
	{
		return;
	}

	SF_CS_LOW();									/* 使能片选 */
	spi_flash_send_byte(CMD_READ);												/* 发送读命令 */
	spi_flash_send_byte((read_addr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	spi_flash_send_byte((read_addr & 0xFF00) >> 8);			/* 发送扇区地址中间8bit */
	spi_flash_send_byte(read_addr & 0xFF);								/* 发送扇区地址低8bit */
	while (size--)
	{
		*buf++ = spi_flash_send_byte(DUMMY_BYTE);					/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	SF_CS_HIGH();									/* 禁能片选 */
}


/*
*********************************************************************************************************
*	函 数 名: spi_flash_send_byte
*	功能说明: 向器件发送一个字节，同时从MISO口线采样器件返回的数据
*	形    参: value : 发送的字节值
*	返 回 值: 从MISO口线采样器件返回的数据
*********************************************************************************************************
*/
static uint8_t spi_flash_send_byte(uint8_t value)
{
	/* 等待上个数据未发送完毕 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI_FLASH, value);

	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}
