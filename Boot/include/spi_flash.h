#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

int spi_flash_read(uint32_t offset, char *buf, uint16_t len);

int spi_flash_write(uint32_t offset, char *buf, uint16_t len);

int spi_flash_erase(uint32_t offset, uint16_t len);



#endif /* _SPI_FLASH_H_ */
