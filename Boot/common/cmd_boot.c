#include <includes.h>


int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long	addr, rc;
	int     rcode = 0;

	if (argc < 2) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	addr = strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lX ...\r\n", addr);

	run_app();
	
	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */

	rc = ((unsigned long (*)(int, char *[]))addr) (--argc, &argv[1]);

	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\r\n", rc);
	return rcode;
}

BOOT_CMD(
	go, CFG_MAXARGS, 1,	do_go,
	"go      - start application at address 'addr'\r\n",
	"addr [arg ...]\r\n"
	"    - start application at address 'addr'\r\n"
	"      passing 'arg' as arguments\r\n"
);

/* ------------------------------------------------------------------- */
/* reset the cpu by setting up the watchdog timer and let him time out */
int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	disable_interrupts();
	printf("reset... \n\n\r\n");
	
	while (1)
	{
		if (serial_tstc())
		{
			serial_getc();
			break;
		}
	}
	return(0);
}

BOOT_CMD(
	reset, CFG_MAXARGS, 1,	do_reset,
	"reset   - Perform RESET of the CPU\r\n",
	NULL
);
