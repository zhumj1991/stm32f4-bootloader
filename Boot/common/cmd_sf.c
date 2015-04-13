#include "includes.h"


#define DISP_LINE_LEN	16


int spi_flash_read(uint32_t offset, char *buf, uint16_t len)
{
	int ret = 0;

	spi_flash_read_buffer((uint8_t *)buf, offset, len);

	return ret;
}

int spi_flash_write(uint32_t offset, char *buf, uint16_t len)
{
	int ret = 0;

	spi_flash_write_buffer((uint8_t *)buf, offset, len);

	return ret;
}

int spi_flash_erase(uint32_t offset, uint16_t len)
{
	int i;
	offset &= 0xFFFFFC00;
	len = (len >> 12) + 1;
	
	for(i = 0; i < len; i++) {
		spi_flash_erase_sector(offset);
		offset += BLOCK_SIZE;
	}

	return 0;
}

static int spi_display(uint32_t offset, char *buf, uint16_t len)
{
	ulong	addr = offset;
	ulong	i, nbytes, linebytes;
	unsigned char	*cp;
	int rc = 0;
	
	nbytes = len;	
	do {
		char	linebuf[DISP_LINE_LEN];
		uint8_t	*ucp = (uint8_t *)linebuf;

		printf("%08lx:", addr);
		linebytes = (nbytes > DISP_LINE_LEN)? DISP_LINE_LEN : nbytes;

		for (i=0; i<linebytes; i++) {
			printf(" %02x", (*ucp++ = *((uint8_t *)buf)));
			buf++;
			addr++;
		}

		serial_puts ("    ");
		cp = (uint8_t *)linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				serial_putc ('.');
			else
				printf ("%c", *cp);
			cp++;
		}
		serial_puts ("\r\n");
		nbytes -= linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);
	
	return rc;
}

static int do_spi_flash_read(int argc, char *argv[])
{
	unsigned long offset;
	unsigned long len;
	char buf[1024];
	char *endp;
	int ret;


	offset = strtoul(argv[1], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		goto usage;
	len = strtoul(argv[2], &endp, 16);
	if (len > 1024)
		goto usage;
	if (*argv[3] == 0 || *endp != 0)
		goto usage;

			
	ret = spi_flash_read(offset, buf, len);

	if (ret) {
		printf("SPI flash %s failed\r\n", argv[0]);
		return 1;
	}

	spi_display(offset, buf, len);
	
	return 0;

usage:
	printf("Usage: sf %s offset len(<400)\r\n", argv[0]);
	return 1;
}

static int do_spi_flash_write(int argc, char *argv[])
{
	unsigned long offset;
	char *endp;
	int ret;
	
	offset = strtoul(argv[1], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		goto usage;
	
	ret = spi_flash_write(offset, argv[2], strlen(argv[2]));	
	
	if (ret) {
		printf("SPI flash %s failed\r\n", argv[0]);
		return 1;
	}
	
	return 0;
	
usage:
	printf("Usage: sf %s offset buf\r\n", argv[0]);
	return 1;
}

static int do_spi_flash_erase(int argc, char *argv[])
{
	unsigned long offset;
	unsigned long len;
	char *endp;
	int ret;

	if (argc == 1) {
		spi_flash_erase_chip();
		return 1;
	}
	
	if (argc < 3)
		goto usage;

	offset = strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		goto usage;
	len = strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		goto usage;

	ret = spi_flash_erase(offset, len);
	if (ret) {
		printf("SPI flash %s failed\r\n", argv[0]);
		return 1;
	}

	return 0;

usage:
	printf("Usage: sf erase [offset len]\r\n");
	return 1;
}

static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "read") == 0)
		return do_spi_flash_read(argc - 1, argv + 1);
	if (strcmp(cmd, "write") == 0)
		return do_spi_flash_write(argc - 1, argv + 1);
	if (strcmp(cmd, "erase") == 0)
		return do_spi_flash_erase(argc - 1, argv + 1);

usage:
	serial_puts(cmdtp->help);
	return 1;
}

BOOT_CMD(
	sf,	5,	0,	do_spi_flash,
	"sf      - SPI flash sub-system\r\n",
	"[ r/w/e ] [ offset] [ len ]\r\n"
	"sf read offset len\r\n"
	"    - read `len'(<400) bytes starting at `offset' to memory at `addr'\r\n"
	"sf write offset buf\r\n"
	"    - write buffer to flash at `offset'\r\n"
	"sf erase [offset len]\r\n"
	"    - erase `len' bytes from `offset'\r\n"
	"    - without paraments: erase the chip\r\n"
);
