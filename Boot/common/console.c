#include <includes.h>



/***************************************************************************
 * check uartdata reday?
 **************************************************************************/
int tstc (void)
{
	return serial_tstc();
}

/***************************************************************************
 * test if ctrl-c was pressed
 **************************************************************************/
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;

int ctrlc (void)
{
	if (!ctrlc_disabled) {
		if(serial_tstc()) {
			switch (serial_getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}
	return 0;
}

/***************************************************************************
 * pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 **************************************************************************/
int disable_ctrlc (int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc (void)
{
	ctrlc_was_pressed = 0;
}

