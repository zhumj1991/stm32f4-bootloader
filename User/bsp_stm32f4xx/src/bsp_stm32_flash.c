#include "includes.h"

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

  return sector;
}

int stm32_flash_write(uint32_t offset, uint8_t *buf, uint16_t len)
{
	uint32_t StartSector, EndSector;
	uint32_t flash_user_start_addr, flash_user_end_addr;
	uint32_t addr;
	uint8_t i;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	/* Get the number of the start and end sectors */
	flash_user_start_addr = APP_ADDR_START + offset;
	flash_user_end_addr = flash_user_start_addr + len;
//  StartSector = GetSector(flash_user_start_addr);
//	EndSector = GetSector(flash_user_end_addr);
//	
//	/* Erase sector */
//	for (i = StartSector; i < EndSector; i += 8) {
//    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
//       be done by word */ 
//    if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE) { 
//			return -1;
//    }
//	}

	addr = flash_user_start_addr;
	while (addr < flash_user_end_addr)
  {
    if (FLASH_ProgramByte(addr, *buf) != FLASH_COMPLETE) {
      return -1;
    }
		addr++;
		buf++;
  }
  
  FLASH_Lock();
	
	return 0;
}

int stm32_flash_read(uint32_t offset, uint8_t *buf, uint16_t len)
{
	uint32_t flash_user_start_addr;
	
	flash_user_start_addr = ADDR_FLASH_SECTOR_0 + offset;
	do {
		*buf = *(uint8_t *)flash_user_start_addr;
		buf++;
		flash_user_start_addr++;
		len--;
	}	while (len > 0);
	
	return 0;
}

int stm32_flash_erase(uint32_t offset, uint16_t len)
{
	uint32_t StartSector, EndSector;
	uint32_t flash_user_start_addr, flash_user_end_addr;
	uint32_t addr;
	uint8_t i;
	

	/* Get the number of the start and end sectors */
	flash_user_start_addr = ADDR_FLASH_SECTOR_0 + offset;
	flash_user_end_addr = flash_user_start_addr + len;
  StartSector = GetSector(flash_user_start_addr);
	EndSector = GetSector(flash_user_end_addr);
	
	/* Erase sector */
	for (i = StartSector; i <= EndSector; i += 8) {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE) { 
			return -1;
    }
	}

	return 0;
}
