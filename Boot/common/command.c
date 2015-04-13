#include <includes.h>



#define BOOT_VERSION	"BOOT Version 1.2.0";
const char version_string[] =	BOOT_VERSION;

/***************************************************************************
 * DIAG_CMD	: version
 * Help			: check boot_cmd version
 **************************************************************************/
int do_version (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf (" %s\r\n", version_string);
	return 0;
}

BOOT_CMD(
	version, 1, 1, do_version,
 	"version - print monitor version\r\n",
	NULL
);

/***************************************************************************
 * DIAG_CMD	: help
 * Help			: help messages
 **************************************************************************/
/*
 * Use serial_puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */
int do_help (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {	/*show list of commands */
		/* pointer arith! */
		int cmd_items = ((int)__boot_cmd_end - (int)__boot_cmd_start) / sizeof(cmd_tbl_t);
		const int cmd_num =10;					/* there is a bug,  cmd_num shoud be equal to cmd_items*/
		cmd_tbl_t *cmd_array[cmd_num];	/* Used to be: cmd_tbl_t *cmd_array[cmd_items] */
		int i, j, swaps;

		/* Make array of commands from .boot_cmd section */
		cmdtp = __boot_cmd_start;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp (cmd_array[j]->name,
					    cmd_array[j + 1]->name) > 0) {
					cmd_tbl_t *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			if (ctrlc ())
				return 1;
			if (usage == NULL)
				continue;
			serial_puts (usage);
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		if ((cmdtp = find_cmd (argv[i])) != NULL) {
#ifdef	CFG_LONGHELP
			/* found - print (long) help info */
			serial_puts (cmdtp->name);
			serial_putc (' ');
			if (cmdtp->help) {
				serial_puts (cmdtp->help);
			} else {
				serial_puts ("- No help available.\r\n");
				rcode = 1;
			}
			serial_putc ('\n');
#else	/* no long help available */
			if (cmdtp->usage)
				serial_puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
		} else {
			printf ("Unknown command '%s' - try 'help' without arguments"
							"for list of all known commands\r\n", argv[i]
					);
			rcode = 1;
		}
	}
	return rcode;
}

BOOT_CMD(
	help,	CFG_MAXARGS, 1, do_help,
 	"help    - print online help\r\n",
 	"[command ...]\r\n"																													\
 	"    - show help information (for 'command')\r\n"														\
 	"'help' prints online help for the monitor commands.\r\n"										\
 	"Without arguments, it prints a short usage message for all commands.\r\n"	\
 	"To get detailed help information for specific commands you can type\r\n"			\
  "'help' with one or more command names as arguments.\r\n"
);

/* This do not ust the BOOT_CMD macro as ? can't be used in symbol names */
#ifdef  CFG_LONGHELP
cmd_tbl_t __boot_cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	1,	do_help,
 	"?       - alias for 'help'\r\n",
	NULL
};
#else
cmd_tbl_t __boot_cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	1,	do_help,
 	"?       - alias for 'help'\r\n"
};
#endif /* CFG_LONGHELP */


/***************************************************************************
 * find command table entry for a command
 **************************************************************************/
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = __boot_cmd_start;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = (__boot_cmd_start);
	     cmdtp != (__boot_cmd_end);
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}
