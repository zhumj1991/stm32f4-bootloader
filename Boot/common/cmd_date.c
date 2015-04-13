#include <includes.h>


#if (CFG_COMMANDS & CFG_CMD_DATE)
/***************************************************************************
 * DIAG_CMD	: data
 * Help			: set time
 **************************************************************************/
const char *weekdays[] = {
	 "Sun", "Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur",
};

/*
 * simple conversion of two-digit string with error checking
 */
static int cnvrt2 (char *str, int *valp)
{
	int val;

	if ((*str < '0') || (*str > '9'))
		return (-1);

	val = *str - '0';

	++str;

	if ((*str < '0') || (*str > '9'))
		return (-1);

	*valp = 10 * val + (*str - '0');

	return (0);
}

/*
 * Convert date string: MMDDhhmm[[CC]YY][.ss]
 *
 * Some basic checking for valid values is done, but this will not catch
 * all possible error conditions.
 */
int mk_date (char *datestr, struct rtc_time *tmp)
{
	int len, val;
	char *ptr;

	ptr = strchr (datestr,'.');
	len = strlen (datestr);

	/* Set seconds */
	if (ptr) {
		int sec;

		*ptr++ = '\0';
		if ((len - (ptr - datestr)) != 2)
			return (-1);

		len = strlen (datestr);

		if (cnvrt2 (ptr, &sec))
			return (-1);

		tmp->tm_sec = sec;
	} else {
		tmp->tm_sec = 0;
	}

	if (len == 12) {		/* MMDDhhmmCCYY	*/
		int year, century;

		if (cnvrt2 (datestr+ 8, &century) ||
		    cnvrt2 (datestr+10, &year) ) {
			return (-1);
		}
		tmp->tm_year = 100 * century + year;
	} else if (len == 10) {		/* MMDDhhmmYY	*/
		int year, century;

		century = tmp->tm_year / 100;
		if (cnvrt2 (datestr+ 8, &year))
			return (-1);
		tmp->tm_year = 100 * century + year;
	}

	switch (len) {
	case 8:			/* MMDDhhmm	*/
		/* fall thru */
	case 10:		/* MMDDhhmmYY	*/
		/* fall thru */
	case 12:		/* MMDDhhmmCCYY	*/
		if (cnvrt2 (datestr+0, &val) ||
		    val > 12) {
			break;
		}
		tmp->tm_mon  = val;
		if (cnvrt2 (datestr+2, &val) ||
		    val > ((tmp->tm_mon==2) ? 29 : 31)) {
			break;
		}
		tmp->tm_mday = val;

		if (cnvrt2 (datestr+4, &val) ||
		    val > 23) {
			break;
		}
		tmp->tm_hour = val;

		if (cnvrt2 (datestr+6, &val) ||
		    val > 59) {
			break;
		}
		tmp->tm_min  = val;

		/* calculate day of week */
		GregorianDay (tmp);

		return (0);
	default:
		break;
	}

	return (-1);
}

int do_date (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
struct rtc_time tm;
	int rcode = 0;

	switch (argc) {
	case 2:			/* set date & time */
		if (strcmp(argv[1],"reset") == 0) {
			puts ("Reset RTC...\n");
			rtc_reset ();
		} else {
			/* initialize tm with current time */
			rtc_get (&tm);
			/* insert new date & time */
			if (mk_date (argv[1], &tm) != 0) {
				puts ("## Bad date format\n");
				return 1;
			}
			/* and write to RTC */
			rtc_set (&tm);
		}
		/* FALL TROUGH */
	case 1:			/* get date & time */
		rtc_get (&tm);

		printf ("Date: %4d/%02d/%02d(%sday)    Time: %02d:%02d:%02d\r\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday,
			(tm.tm_wday<0 || tm.tm_wday>6) ?
				"unknown " : weekdays[tm.tm_wday],
			tm.tm_hour, tm.tm_min, tm.tm_sec);

		return 0;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
	}
	return rcode;
}
	
BOOT_CMD(
	date,	2,	1,	do_date,
	"date    - get/set/reset date & time\r\n",
	"[MMDDhhmm[[CC]YY][.ss]]\r\ndate reset\r\n"	
	"  - without arguments: print date & time\r\n"
	"  - with numeric argument: set the system date & time\r\n"
	"  - with 'reset' argument: reset the RTC\r\n"
);
#endif
