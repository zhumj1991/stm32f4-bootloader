#include "includes.h"

static ulong load_serial_ymodem(ulong offset)
{
	int size;
	char buf[32];
	int err;
	int res;
	connection_info_t info;
	char ymodemBuf[1024];
	ulong store_addr = ~0;
	ulong addr = 0;


	size = 0;
	info.mode = xyzModem_ymodem;	
	res = xyzModem_stream_open (&info, &err);
}

int do_load_serial_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong offset = 0;
	ulong addr;
	
	if (argc >= 2) {
		offset = strtoul(argv[1], NULL, 16);
	}
	
	printf ("## Ready for binary (ymodem) download "
			"to 0x%08lX...\n", offset);
	
	addr = load_serial_ymodem (offset);
	
	
	
	
}




BOOT_CMD(
	loady, 3, 0,	do_load_serial_bin,
	"loady   - load binary file over serial line (ymodem mode)\r\n",
	"[ off ] [ baud ]\r\n"
	"    - load binary file over serial line"
	" with offset 'off' and baudrate 'baud'\n"
);
