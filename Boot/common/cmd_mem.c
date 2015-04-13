#include "includes.h"

#define CFG_NO_FLASH


#if (CONFIG_COMMANDS & CFG_CMD_MMC)
#include <mmc.h>
#endif
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif

#if (CONFIG_COMMANDS & (CFG_CMD_MEMORY	| \
			CFG_CMD_I2C	| \
			CFG_CMD_ITEST	| \
			CFG_CMD_PCI	| \
			CMD_CMD_PORTIO	) )
int cmd_get_data_size(char* arg, int default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	int len = strlen(arg);
	if (len > 2 && arg[len-2] == '.') {
		switch(arg[len-1]) {
		case 'b':
			return 1;
		case 'w':
			return 2;
		case 'l':
			return 4;
		case 's':
			return -2;
		default:
			return -1;
		}
	}
	return default_size;
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_MEMORY)

#ifdef	CMD_MEM_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

static int mod_mem(cmd_tbl_t *, int, int, int, char *[]);

/* Display values from last command.
 * Memory modify remembered values are different from display memory.
 */
uint32_t	dp_last_addr, dp_last_size;
uint32_t	dp_last_length = 0x40;
uint32_t	mm_last_addr, mm_last_size;

static	ulong	base_address = 0;

/* Memory Display
 *
 * Syntax:
 *	md{.b, .w, .l} {addr} {len}
 */
#define DISP_LINE_LEN	16
int do_mem_md ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length;
	ulong	i, nbytes, linebytes;
	unsigned char	*cp;
	int	size;
	int rc = 0;

	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	addr = dp_last_addr;
	size = dp_last_size;
	length = dp_last_length;

	if (argc < 2) {
		printf ("Usage:\r\r\n%s\r\r\n", cmdtp->usage);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		if ((size = cmd_get_data_size(argv[0], 4)) < 0)
			return 1;

		/* Address is specified since argc > 1
		*/
		addr = strtoul(argv[1], NULL, 16);
		addr += base_address;

		/* If another parameter, it is the length to display.
		 * Length is the number of objects, not number of bytes.
		 */
		if (argc > 2)
			length = strtoul(argv[2], NULL, 16);
	}

	/* Print the lines.
	 *
	 * We buffer all read data, so we can make sure data is read only
	 * once, and all accesses are with the specified bus width.
	 */
	nbytes = length * size;
	do {
		char	linebuf[DISP_LINE_LEN];
		uint32_t	*uip = (uint32_t   *)linebuf;
		uint16_t	*usp = (uint16_t *)linebuf;
		uint8_t	*ucp = (uint8_t *)linebuf;
#ifdef CONFIG_HAS_DATAFLASH
		int rc;
#endif
		printf("%08lx:", addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

#ifdef CONFIG_HAS_DATAFLASH
		if ((rc = read_dataflash(addr, (linebytes/size)*size, linebuf)) == DATAFLASH_OK){
			/* if outside dataflash */
			/*if (rc != 1) {
				dataflash_perror (rc);
				return (1);
			}*/
			for (i=0; i<linebytes; i+= size) {
				if (size == 4) {
					printf(" %08x", *uip++);
				} else if (size == 2) {
					printf(" %04x", *usp++);
				} else {
					printf(" %02x", *ucp++);
				}
				addr += size;
			}

		} else {	/* addr does not correspond to DataFlash */
#endif
		for (i=0; i<linebytes; i+= size) {
			if (size == 4) {
				printf(" %08x", (*uip++ = *((uint32_t *)addr)));
			} else if (size == 2) {
				printf(" %04x", (*usp++ = *((uint16_t *)addr)));
			} else {
				printf(" %02x", (*ucp++ = *((uint8_t *)addr)));
			}
			addr += size;
		}
#ifdef CONFIG_HAS_DATAFLASH
		}
#endif
		serial_puts ("    ");
		cp = (uint8_t *)linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				serial_putc ('.');
			else
				printf("%c", *cp);
			cp++;
		}
		serial_puts ("\r\n");
		nbytes -= linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);

	dp_last_addr = addr;
	dp_last_length = length;
	dp_last_size = size;
	return (rc);
}

int do_mem_mm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_mem (cmdtp, 1, flag, argc, argv);
}
int do_mem_nm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_mem (cmdtp, 0, flag, argc, argv);
}

int do_mem_mw ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, writeval, count;
	int	size;

	if ((argc < 3) || (argc > 4)) {
		printf ("Usage:\r\r\n%s\r\r\n", cmdtp->usage);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 1)
		return 1;

	/* Address is specified since argc > 1
	*/
	addr = strtoul(argv[1], NULL, 16);
	addr += base_address;

	/* Get the value to write.
	*/
	writeval = strtoul(argv[2], NULL, 16);

	/* Count ? */
	if (argc == 4) {
		count = strtoul(argv[3], NULL, 16);
	} else {
		count = 1;
	}

	while (count-- > 0) {
		if (size == 4)
			*((ulong  *)addr) = (ulong )writeval;
		else if (size == 2)
			*((uint16_t *)addr) = (uint16_t)writeval;
		else
			*((uint8_t *)addr) = (uint8_t)writeval;
		addr += size;
	}
	return 0;
}

#ifdef CONFIG_MX_CYCLIC
int do_mem_mdc ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	ulong count;

	if (argc < 4) {
		printf ("Usage:\r\r\n%s\r\r\n", cmdtp->usage);
		return 1;
	}

	count = simple_strtoul(argv[3], NULL, 10);

	for (;;) {
		do_mem_md (NULL, 0, 3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			serial_puts("Abort\r\n");
			return 0;
		}
	}

	return 0;
}

int do_mem_mwc ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	ulong count;

	if (argc < 4) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	count = simple_strtoul(argv[3], NULL, 10);

	for (;;) {
		do_mem_mw (NULL, 0, 3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			serial_puts("Abort\r\n");
			return 0;
		}
	}

	return 0;
}
#endif /* CONFIG_MX_CYCLIC */

int do_mem_cmp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr1, addr2, count, ngood;
	int	size;
	int     rcode = 0;

	if (argc != 4) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	addr1 = strtoul(argv[1], NULL, 16);
	addr1 += base_address;

	addr2 = strtoul(argv[2], NULL, 16);
	addr2 += base_address;

	count = strtoul(argv[3], NULL, 16);

#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr1) | addr_dataflash(addr2)){
		serial_puts ("Comparison with DataFlash space not supported.\r\n\r");
		return 0;
	}
#endif

	ngood = 0;

	while (count-- > 0) {
		if (size == 4) {
			ulong word1 = *(ulong *)addr1;
			ulong word2 = *(ulong *)addr2;
			if (word1 != word2) {
				printf("word at 0x%08lx (0x%08lx) "
					"!= word at 0x%08lx (0x%08lx)\r\n",
					addr1, word1, addr2, word2);
				rcode = 1;
				break;
			}
		}
		else if (size == 2) {
			uint16_t hword1 = *(uint16_t *)addr1;
			uint16_t hword2 = *(uint16_t *)addr2;
			if (hword1 != hword2) {
				printf("halfword at 0x%08lx (0x%04x) "
					"!= halfword at 0x%08lx (0x%04x)\r\n",
					addr1, hword1, addr2, hword2);
				rcode = 1;
				break;
			}
		}
		else {
			uint8_t byte1 = *(uint8_t *)addr1;
			uint8_t byte2 = *(uint8_t *)addr2;
			if (byte1 != byte2) {
				printf("byte at 0x%08lx (0x%02x) "
					"!= byte at 0x%08lx (0x%02x)\r\n",
					addr1, byte1, addr2, byte2);
				rcode = 1;
				break;
			}
		}
		ngood++;
		addr1 += size;
		addr2 += size;
	}

	printf("Total of %ld %s%s were the same\r\n",
		ngood, size == 4 ? "word" : size == 2 ? "halfword" : "byte",
		ngood == 1 ? "" : "s");
	return rcode;
}

int do_mem_cp ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, dest, count;
	int	size;

	if (argc != 4) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	addr = strtoul(argv[1], NULL, 16);
	addr += base_address;

	dest = strtoul(argv[2], NULL, 16);
	dest += base_address;

	count = strtoul(argv[3], NULL, 16);

	if (count == 0) {
		serial_puts ("Zero length ???\r\n");
		return 1;
	}

#ifndef CFG_NO_FLASH
	/* check if we are copying to Flash */
	if ( (addr2info(dest) != NULL)
#ifdef CONFIG_HAS_DATAFLASH
	   && (!addr_dataflash(addr))
#endif
	   ) {
		int rc;

		serial_puts ("Copy to Flash... ");

		rc = flash_write ((char *)addr, dest, count*size);
		if (rc != 0) {
			flash_perror (rc);
			return (1);
		}
		serial_puts ("done\r\n");
		return 0;
	}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_MMC)
	if (mmc2info(dest)) {
		int rc;

		serial_puts ("Copy to MMC... ");
		switch (rc = mmc_write ((uchar *)addr, dest, count*size)) {
		case 0:
			serial_puts ("\r\n");
			return 1;
		case -1:
			serial_puts ("failed\r\n");
			return 1;
		default:
			printf ("%s[%d] FIXME: rc=%d\r\n",__FILE__,__LINE__,rc);
			return 1;
		}
		serial_puts ("done\r\n");
		return 0;
	}

	if (mmc2info(addr)) {
		int rc;

		serial_puts ("Copy from MMC... ");
		switch (rc = mmc_read (addr, (uchar *)dest, count*size)) {
		case 0:
			serial_puts ("\r\n");
			return 1;
		case -1:
			serial_puts ("failed\r\n");
			return 1;
		default:
			printf ("%s[%d] FIXME: rc=%d\r\n",__FILE__,__LINE__,rc);
			return 1;
		}
		serial_puts ("done\r\n");
		return 0;
	}
#endif

#ifdef CONFIG_HAS_DATAFLASH
	/* Check if we are copying from RAM or Flash to DataFlash */
	if (addr_dataflash(dest) && !addr_dataflash(addr)){
		int rc;

		serial_puts ("Copy to DataFlash... ");

		rc = write_dataflash (dest, addr, count*size);

		if (rc != 1) {
			dataflash_perror (rc);
			return (1);
		}
		serial_puts ("done\r\n");
		return 0;
	}

	/* Check if we are copying from DataFlash to RAM */
	if (addr_dataflash(addr) && !addr_dataflash(dest) && (addr2info(dest)==NULL) ){
		int rc;
		rc = read_dataflash(addr, count * size, (char *) dest);
		if (rc != 1) {
			dataflash_perror (rc);
			return (1);
		}
		return 0;
	}

	if (addr_dataflash(addr) && addr_dataflash(dest)){
		serial_puts ("Unsupported combination of source/destination.\r\n\r");
		return 1;
	}
#endif

	while (count-- > 0) {
		if (size == 4)
			*((ulong  *)dest) = *((ulong  *)addr);
		else if (size == 2)
			*((uint16_t *)dest) = *((uint16_t *)addr);
		else
			*((uint8_t *)dest) = *((uint8_t *)addr);
		addr += size;
		dest += size;
	}
	return 0;
}

int do_mem_base (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 1) {
		/* Set new base address.
		*/
		base_address = strtoul(argv[1], NULL, 16);
	}
	/* Print the current base address.
	*/
	printf("Base Address: 0x%08lx\r\n", base_address);
	return 0;
}

int do_mem_loop (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length, i, junk;
	int	size;
	volatile uint32_t	*longp;
	volatile uint16_t *shortp;
	volatile uint8_t	*cp;

	if (argc < 3) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	/* Check for a size spefication.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	/* Address is always specified.
	*/
	addr = strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = strtoul(argv[2], NULL, 16);

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
		if (size == 4) {
			longp = (uint32_t *)addr;
			for (;;)
				i = *longp;
		}
		if (size == 2) {
			shortp = (uint16_t *)addr;
			for (;;)
				i = *shortp;
		}
		cp = (uint8_t *)addr;
		for (;;)
			i = *cp;
	}

	if (size == 4) {
		for (;;) {
			longp = (uint32_t *)addr;
			i = length;
			while (i-- > 0)
				junk = *longp++;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (uint16_t *)addr;
			i = length;
			while (i-- > 0)
				junk = *shortp++;
		}
	}
	for (;;) {
		cp = (uint8_t *)addr;
		i = length;
		while (i-- > 0)
			junk = *cp++;
	}
}

#ifdef CONFIG_LOOPW
int do_mem_loopw (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length, i, data;
	int	size;
	volatile uint32_t	*longp;
	volatile uint16_t *shortp;
	volatile uint8_t	*cp;

	if (argc < 4) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	/* Check for a size spefication.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	/* Address is always specified.
	*/
	addr = strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = strtoul(argv[2], NULL, 16);

	/* data to write */
	data = strtoul(argv[3], NULL, 16);

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
		if (size == 4) {
			longp = (uint *)addr;
			for (;;)
				*longp = data;
					}
		if (size == 2) {
			shortp = (uint16_t *)addr;
			for (;;)
				*shortp = data;
		}
		cp = (uint8_t *)addr;
		for (;;)
			*cp = data;
	}

	if (size == 4) {
		for (;;) {
			longp = (uint32_t *)addr;
			i = length;
			while (i-- > 0)
				*longp++ = data;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (uint16_t *)addr;
			i = length;
			while (i-- > 0)
				*shortp++ = data;
		}
	}
	for (;;) {
		cp = (uint8_t *)addr;
		i = length;
		while (i-- > 0)
			*cp++ = data;
	}
}
#endif /* CONFIG_LOOPW */

/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CFG_ALT_MEMTEST. The complete test loops until
 * interrupted by ctrl-c or by a failure of one of the sub-tests.
 */
int do_mem_mtest (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	volatile ulong	*addr, *start, *end;
	ulong	val;
	ulong	readback;

#if defined(CFG_ALT_MEMTEST)
	volatile ulong	addr_mask;
	volatile ulong	offset;
	volatile ulong	test_offset;
	volatile ulong	pattern;
	volatile ulong	temp;
	volatile ulong	anti_pattern;
	volatile ulong	num_words;
#if defined(CFG_MEMTEST_SCRATCH)
	volatile ulong *dummy = (vu_long*)CFG_MEMTEST_SCRATCH;
#else
	volatile ulong *dummy = 0;	/* yes, this is address 0x0, not NULL */
#endif
	int	j;
	int iterations = 1;

	static const ulong bitpattern[] = {
		0x00000001,	/* single bit */
		0x00000003,	/* two adjacent bits */
		0x00000007,	/* three adjacent bits */
		0x0000000F,	/* four adjacent bits */
		0x00000005,	/* two non-adjacent bits */
		0x00000015,	/* three non-adjacent bits */
		0x00000055,	/* four non-adjacent bits */
		0xaaaaaaaa,	/* alternating 1/0 */
	};
#else
	ulong	incr;
	ulong	pattern;
	int     rcode = 0;
#endif

	if (argc > 1) {
		start = (ulong *)strtoul(argv[1], NULL, 16);
	} else {
		start = (ulong *)CFG_MEMTEST_START;
	}

	if (argc > 2) {
		end = (ulong *)strtoul(argv[2], NULL, 16);
	} else {
		end = (ulong *)(CFG_MEMTEST_END);
	}

	if (argc > 3) {
		pattern = (ulong)strtoul(argv[3], NULL, 16);
	} else {
		pattern = 0;
	}

#if defined(CFG_ALT_MEMTEST)
	printf ("Testing %08x ... %08x:\r\n", (uint)start, (uint)end);
	PRINTF("%s:%d: start 0x%p end 0x%p\r\n",
		__FUNCTION__, __LINE__, start, end);

	for (;;) {
		if (ctrlc()) {
			serial_puts ("\r\n");
			return 1;
		}

		printf("Iteration: %6d\r", iterations);
		PRINTF("Iteration: %6d\r\n", iterations);
		iterations++;

		/*
		 * Data line test: write a pattern to the first
		 * location, write the 1's complement to a 'parking'
		 * address (changes the state of the data bus so a
		 * floating bus doen't give a false OK), and then
		 * read the value back. Note that we read it back
		 * into a variable because the next time we read it,
		 * it might be right (been there, tough to explain to
		 * the quality guys why it prints a failure when the
		 * "is" and "should be" are obviously the same in the
		 * error message).
		 *
		 * Rather than exhaustively testing, we test some
		 * patterns by shifting '1' bits through a field of
		 * '0's and '0' bits through a field of '1's (i.e.
		 * pattern and ~pattern).
		 */
		addr = start;
		for (j = 0; j < sizeof(bitpattern)/sizeof(bitpattern[0]); j++) {
		    val = bitpattern[j];
		    for(; val != 0; val <<= 1) {
			*addr  = val;
			*dummy  = ~val; /* clear the test data off of the bus */
			readback = *addr;
			if(readback != val) {
			     printf ("FAILURE (data line): "
				"expected %08lx, actual %08lx\r\n",
					  val, readback);
			}
			*addr  = ~val;
			*dummy  = val;
			readback = *addr;
			if(readback != ~val) {
			    printf ("FAILURE (data line): "
				"Is %08lx, should be %08lx\r\n",
					readback, ~val);
			}
		    }
		}

		/*
		 * Based on code whose Original Author and Copyright
		 * information follows: Copyright (c) 1998 by Michael
		 * Barr. This software is placed into the public
		 * domain and may be used for any purpose. However,
		 * this notice must not be changed or removed and no
		 * warranty is either expressed or implied by its
		 * publication or distribution.
		 */

		/*
		 * Address line test
		 *
		 * Description: Test the address bus wiring in a
		 *              memory region by performing a walking
		 *              1's test on the relevant bits of the
		 *              address and checking for aliasing.
		 *              This test will find single-bit
		 *              address failures such as stuck -high,
		 *              stuck-low, and shorted pins. The base
		 *              address and size of the region are
		 *              selected by the caller.
		 *
		 * Notes:	For best results, the selected base
		 *              address should have enough LSB 0's to
		 *              guarantee single address bit changes.
		 *              For example, to test a 64-Kbyte
		 *              region, select a base address on a
		 *              64-Kbyte boundary. Also, select the
		 *              region size as a power-of-two if at
		 *              all possible.
		 *
		 * Returns:     0 if the test succeeds, 1 if the test fails.
		 *
		 * ## NOTE ##	Be sure to specify start and end
		 *              addresses such that addr_mask has
		 *              lots of bits set. For example an
		 *              address range of 01000000 02000000 is
		 *              bad while a range of 01000000
		 *              01ffffff is perfect.
		 */
		addr_mask = ((ulong)end - (ulong)start)/sizeof(vu_long);
		pattern = (vu_long) 0xaaaaaaaa;
		anti_pattern = (vu_long) 0x55555555;

		PRINTF("%s:%d: addr mask = 0x%.8lx\r\n",
			__FUNCTION__, __LINE__,
			addr_mask);
		/*
		 * Write the default pattern at each of the
		 * power-of-two offsets.
		 */
		for (offset = 1; (offset & addr_mask) != 0; offset <<= 1) {
			start[offset] = pattern;
		}

		/*
		 * Check for address bits stuck high.
		 */
		test_offset = 0;
		start[test_offset] = anti_pattern;

		for (offset = 1; (offset & addr_mask) != 0; offset <<= 1) {
		    temp = start[offset];
		    if (temp != pattern) {
			printf ("\r\nFAILURE: Address bit stuck high @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx\r\n",
				(ulong)&start[offset], pattern, temp);
			return 1;
		    }
		}
		start[test_offset] = pattern;

		/*
		 * Check for addr bits stuck low or shorted.
		 */
		for (test_offset = 1; (test_offset & addr_mask) != 0; test_offset <<= 1) {
		    start[test_offset] = anti_pattern;

		    for (offset = 1; (offset & addr_mask) != 0; offset <<= 1) {
			temp = start[offset];
			if ((temp != pattern) && (offset != test_offset)) {
			    printf ("\r\nFAILURE: Address bit stuck low or shorted @"
				" 0x%.8lx: expected 0x%.8lx, actual 0x%.8lx\r\n",
				(ulong)&start[offset], pattern, temp);
			    return 1;
			}
		    }
		    start[test_offset] = pattern;
		}

		/*
		 * Description: Test the integrity of a physical
		 *		memory device by performing an
		 *		increment/decrement test over the
		 *		entire region. In the process every
		 *		storage bit in the device is tested
		 *		as a zero and a one. The base address
		 *		and the size of the region are
		 *		selected by the caller.
		 *
		 * Returns:     0 if the test succeeds, 1 if the test fails.
		 */
		num_words = ((ulong)end - (ulong)start)/sizeof(vu_long) + 1;

		/*
		 * Fill memory with a known pattern.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
			start[offset] = pattern;
		}

		/*
		 * Check each location and invert it for the second pass.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		    temp = start[offset];
		    if (temp != pattern) {
			printf ("\r\nFAILURE (read/write) @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\r\n",
				(ulong)&start[offset], pattern, temp);
			return 1;
		    }

		    anti_pattern = ~pattern;
		    start[offset] = anti_pattern;
		}

		/*
		 * Check each location for the inverted pattern and zero it.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		    anti_pattern = ~pattern;
		    temp = start[offset];
		    if (temp != anti_pattern) {
			printf ("\r\nFAILURE (read/write): @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\r\n",
				(ulong)&start[offset], anti_pattern, temp);
			return 1;
		    }
		    start[offset] = 0;
		}
	}

#else /* The original, quickie test */
	incr = 1;
	for (;;) {
		if (ctrlc()) {
			serial_puts ("\r\n");
			return 1;
		}

		printf ("\rPattern %08lX  Writing..."
			"%12s"
			"\b\b\b\b\b\b\b\b\b\b",
			pattern, "");

		for (addr=start,val=pattern; addr<end; addr++) {
			*addr = val;
			val  += incr;
		}

		serial_puts ("Reading...");

		for (addr=start,val=pattern; addr<end; addr++) {
			readback = *addr;
			if (readback != val) {
				printf ("\r\nMem error @ 0x%08X: "
					"found %08lX, expected %08lX\r\n",
					(uint32_t)addr, readback, val);
				rcode = 1;
			}
			val += incr;
		}

		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		}
		else {
			pattern = ~pattern;
		}
		incr = -incr;
	}
	return rcode;
#endif
}


/* Modify memory.
 *
 * Syntax:
 *	mm{.b, .w, .l} {addr}
 *	nm{.b, .w, .l} {addr}
 */
static int
mod_mem(cmd_tbl_t *cmdtp, int incrflag, int flag, int argc, char *argv[])
{
	ulong	addr, i;
	int	nbytes, size;
	extern char console_buffer[];

	if (argc != 2) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

#ifdef CONFIG_BOOT_RETRY_TIME
	reset_cmd_timeout();	/* got a good command to get here */
#endif
	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	addr = mm_last_addr;
	size = mm_last_size;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		if ((size = cmd_get_data_size(argv[0], 4)) < 0)
			return 1;

		/* Address is specified since argc > 1
		*/
		addr = strtoul(argv[1], NULL, 16);
		addr += base_address;
	}

#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr)){
		serial_puts ("Can't modify DataFlash in place. Use cp instead.\r\n\r");
		return 0;
	}
#endif

	/* Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
		if (size == 4)
			printf(" %08x", *((uint32_t *)addr));
		else if (size == 2)
			printf(" %04x", *((uint16_t *)addr));
		else
			printf(" %02x", *((uint8_t *)addr));

		nbytes = readline (" ? ");
		if (nbytes == 0 || (nbytes == 1 && console_buffer[0] == '-')) {
			/* <CR> pressed as only input, don't modify current
			 * location and move to next. "-" pressed will go back.
			 */
			if (incrflag)
				addr += nbytes ? -size : size;
			nbytes = 1;
#ifdef CONFIG_BOOT_RETRY_TIME
			reset_cmd_timeout(); /* good enough to not time out */
#endif
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2) {
			break;	/* timed out, exit the command	*/
		}
#endif
		else {
			char *endp;
			i = strtoul(console_buffer, &endp, 16);
			nbytes = endp - console_buffer;
			if (nbytes) {
#ifdef CONFIG_BOOT_RETRY_TIME
				/* good enough to not time out
				 */
				reset_cmd_timeout();
#endif
				if (size == 4)
					*((uint32_t *)addr) = i;
				else if (size == 2)
					*((uint16_t *)addr) = i;
				else
					*((uint8_t *)addr) = i;
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	mm_last_addr = addr;
	mm_last_size = size;
	return 0;
}

/**************************************************/
#if (CONFIG_COMMANDS & CFG_CMD_MEMORY)
BOOT_CMD(
	md,     3,     1,      do_mem_md,
	"md      - memory display\r\n",
	"[.b, .w, .l] address [# of objects]\r\n    - memory display\r\n"
);


BOOT_CMD(
	mm,     2,      1,       do_mem_mm,
	"mm      - memory modify (auto-incrementing)\r\n",
	"[.b, .w, .l] address\r\n" "    - memory modify, auto increment address\r\n"
);


BOOT_CMD(
	nm,     2,	    1,     	do_mem_nm,
	"nm      - memory modify (constant address)\r\n",
	"[.b, .w, .l] address\r\n    - memory modify, read and keep address\r\n"
);

BOOT_CMD(
	mw,    4,    1,     do_mem_mw,
	"mw      - memory write (fill)\r\n",
	"[.b, .w, .l] address value [count]\r\n    - write memory\r\n"
);

BOOT_CMD(
	cp,    4,    1,    do_mem_cp,
	"cp      - memory copy\r\n",
	"[.b, .w, .l] source target count\r\n    - copy memory\r\n"
);

BOOT_CMD(
	cmp,    4,     1,     do_mem_cmp,
	"cmp     - memory compare\r\n",
	"[.b, .w, .l] addr1 addr2 count\r\n    - compare memory\r\n"
);

BOOT_CMD(
	base,    2,    1,     do_mem_base,
	"base    - print or set address offset\r\n",
	"\r\n    - print address offset for memory commands\r\n"
	"base off\r\n    - set address offset for memory commands to 'off'\r\n"
);

BOOT_CMD(
	loop,    3,    1,    do_mem_loop,
	"loop    - infinite loop on address range\r\n",
	"[.b, .w, .l] address number_of_objects\r\n"
	"    - loop on a set of addresses\r\n"
);

#ifdef CONFIG_LOOPW
BOOT_CMD(
	loopw,    4,    1,    do_mem_loopw,
	"loopw   - infinite write loop on address range\r\n",
	"[.b, .w, .l] address number_of_objects data_to_write\r\n"
	"    - loop on a set of addresses\r\n"
);
#endif /* CONFIG_LOOPW */

BOOT_CMD(
	mtest,    4,    1,     do_mem_mtest,
	"mtest   - simple RAM test\r\n",
	"[start [end [pattern]]]\r\n"
	"    - simple RAM read/write test\r\n"
);

#ifdef CONFIG_MX_CYCLIC
BOOT_CMD(
	mdc,     4,     1,      do_mem_mdc,
	"mdc     - memory display cyclic\r\n",
	"[.b, .w, .l] address count delay(ms)\r\n    - memory display cyclic\r\n"
);

BOOT_CMD(
	mwc,     4,     1,      do_mem_mwc,
	"mwc     - memory write cyclic\r\n",
	"[.b, .w, .l] address value delay(ms)\r\n    - memory write cyclic\r\n"
);
#endif /* CONFIG_MX_CYCLIC */

#endif
#endif	/* CFG_CMD_MEMORY */
