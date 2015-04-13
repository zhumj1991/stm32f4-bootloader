#include "includes.h"



static void run_app(void)
{

	typedef void (* fun_type)(void);
	static fun_type jump2app;
	
//	if(((*(vu32*)APP_ADDR_START)&0x2FFE0000) == 0x20000000) {	//检查栈顶地址是否合法.
		/* Jump to user application */
		jump2app = (fun_type)*(vu32 *)(APP_ADDR_START + 4);
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) APP_ADDR_START);
		jump2app();
//	}
	
}

static int cp_flash_to_sram(ulong spi_offset, ulong sram_offset)
{
	unsigned char buf[256];
	unsigned char *cp = (unsigned char *)(APP_ADDR_START + sram_offset);
	int size = 0;
	int i, num_bytes, len;
	int ret = 0;
	
	spi_flash_read_buffer(buf, NUM_BYTES_ADDR, 4);	/* how many bytes need to copy */
	size = buf[3];
	size = (size << 8) + buf[2];
	size = (size << 8) + buf[1];
	size = (size << 8) + buf[0];
	if (size == 0xFFFFFFFF) {
		printf("No bytes in spi flash.\r\n");
		return 0;
	}
	else
		printf("%d bytes copy from spi flash...\r\n", size);

	len = sizeof(buf);
	/* copy to sram */
	do {
		memset(buf, 0, len);		
		num_bytes = (size>len)? len : size;		
		spi_flash_read_buffer(buf, spi_offset, num_bytes);
		spi_offset += num_bytes;
		
		for(i = 0; i < num_bytes; i++){
			*cp++ = buf[i];			
		}
		size -= num_bytes;
		ret += num_bytes;
		
	} while (size > 0);

	return ret;	
}

static int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret;
	ulong spi_offset, sram_offset;
	char *endp;

	spi_offset = 0;
	sram_offset = 0;
	
	if (argc >= 2)
		spi_offset = strtoul(argv[1],	&endp, 16);
	
	if (argc > 2)
		sram_offset = strtoul(argv[2],	&endp, 16);
	
	ret = cp_flash_to_sram(spi_offset, sram_offset);
	if(ret) {
		printf("copy %d bytes to sram\r\n", ret);
	} else {
		printf("bootm failed\r\n");
		return 0;
	}
	
	run_app();
	return 0;
	
}


BOOT_CMD(
	bootm, 3, 0,	do_bootm,
	"bootm   - load binary file from spi flash to sram and run it\r\n",
	" [spi offset] [sram offset]\r\n"
);
