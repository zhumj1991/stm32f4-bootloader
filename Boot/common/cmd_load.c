#include "includes.h"


static int getcxmodem(void) {
	if (tstc())
		return (serial_getc());
	return -1;
}

static ulong load_serial_ymodem(char *type, ulong offset)
{
	int size;
	char buf[32];
	int err;
	int res;
	connection_info_t info;
	char ymodemBuf[1024];
	ulong store_addr = 0;
	ulong addr = 0;


	size = 0;
	info.mode = xyzModem_ymodem;	
	res = xyzModem_stream_open (&info, &err);
	
	if (!res) {
		while ((res = xyzModem_stream_read (ymodemBuf, 1024, &err)) > 0) {
			store_addr = addr + offset;
			size += res;
			addr += res;
#ifndef CFG_NO_FLASH
			if (!strcmp(type, "flash")) {
				int rc;

				rc = spi_flash_write(store_addr, (char *) ymodemBuf, res);
				if (rc != 0) {
					//flash_perror (rc);
					return (~0);
				}
			} else
#endif
			{
				memcpy ((char *) (store_addr + 	CFG_LOAD_ADDR), ymodemBuf,
					res);
			}

		}
	} else {
		printf ("%s\n", xyzModem_error (err));
	}

	xyzModem_stream_close (&err);
	xyzModem_stream_terminate (false, &getcxmodem);

	if (!strcmp(type, "flash")) {

		char buf[4];
		buf[0] = (char) size;
		buf[1] = (char) (size >> 8);
		buf[2] = (char) (size >> 16);
		buf[3] = (char) (size >> 24);
		spi_flash_write(NUM_BYTES_ADDR, (char *) buf, 4);

	}

	printf ("## Total Size      = 0x%08x = %d Bytes\r\n", size, size);

	return offset;
}

int do_load_serial_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong offset = 0;
	ulong addr;
	int rcode = 0;
	
	if (argc != 3) {
		printf ("Usage: loady [type] [ off ]\r\n");
		return 1;
	}
	
	offset = strtoul(argv[2], NULL, 16);
	printf ("## Ready for binary (ymodem) download "
	"to 0x%08lX... in %s\r\n", offset, (strcmp(argv[1], "flash")? "external sram" : "flash"));
	
	addr = load_serial_ymodem (argv[1], offset);
	
	return rcode;	
}


BOOT_CMD(
	loady, 3, 0,	do_load_serial_bin,
	"loady   - load binary file over serial line (ymodem mode)\r\n",
	"[type] [ off ]\r\n"
	"    - load binary file over serial line"
	" type can be 'sram' or 'flash' with offset 'off'\r\n"
);
