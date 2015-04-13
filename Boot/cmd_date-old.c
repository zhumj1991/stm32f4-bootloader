#include <diag_cmd.h>


#if CFG_CMD_DATE
/***************************************************************************
 * DIAG_CMD	: data
 * Help			: set time
 **************************************************************************/
const char *weekdays[] = {
	"Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur", "Sun", 
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
	int val;
	char *ptr;
	char tmp_wday[3];int i;
	
	ptr = strchr (datestr,'/');
	
	/* set date */
	if (ptr) {	
		int year, century;
		
		/* tmp->tm_year */
		if((ptr - datestr) == 2){	/* YY/MM/DD(weekday) */
			century = tmp->tm_year / 100;
			if (cnvrt2 (datestr, &year))
				return (-1);
			tmp->tm_year = 100 * century + year;	
		} else {									/* CCYY/MM/DD(weekday) */
			if (cnvrt2 (datestr, &century) ||
		    cnvrt2 (datestr+2, &year) ) {
				return (-1);
			}
			tmp->tm_year = 100 * century + year;
		}
		
		/* tmp->tm_mon */
		if (cnvrt2 (ptr+1, &val))
				return (-1);
		if(val >12)
			return (-1);
		tmp->tm_mon = val;
		
		/* tmp->tm_mday */
		if (cnvrt2 (ptr+1+3, &val))
				return (-1);
		if(val > ((tmp->tm_mon==2) ? 29 : 31))
			return (-1);
		tmp->tm_mday = val;
		
		/* tmp->tm_wday */
		ptr = strchr (datestr,'(');
		
		strncpy(tmp_wday, ptr+1, 3);
		for(i=0; i<7; i++) {
			if (strncmp(tmp_wday, weekdays[i], 3) == 0)
				break;
		}
		if(i > 7)
			return (-1);
		else
			tmp->tm_wday = ++i;
	}
	
	/* set time */
	ptr = strchr (datestr,':');		/* hh:mm[:ss] */
	if(ptr) {
		/* tmp->tm_hour */
		if (cnvrt2 (ptr-2, &val))
				return (-1);
		if(val > 23)
			return (-1);
		tmp->tm_hour = val;	
		
		/* tmp->tm_min */
		if (cnvrt2 (ptr+1, &val))
				return (-1);
		if(val > 59)
			return (-1);
		tmp->tm_min = val;

		/* tmp->tm_sec */
		if (*(ptr+3) == '\0') {
			tmp->tm_sec = 0;
		} else {
			if (cnvrt2 (ptr+4, &val))
				return (-1);
			if(val > 59)
				return (-1);
			tmp->tm_sec = val;
		}
	}
	return 0;
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
			(tm.tm_wday<1 || tm.tm_wday>8) ?
				"unknown " : weekdays[tm.tm_wday-1],
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
	"[[CC]YY/MM/DD(weekday)]hh:mm[:ss]\ndate reset\r\n"					\
	"  - weekday: Sun, Mon, Tues, Wednes, Thurs, Fri, Satur\r\n"\
	"  - without arguments: print date & time\r\n"							\
	"  - with numeric argument: set the system date & time\r\n"	\
	"  - with 'reset' argument: reset the RTC\r\n"							\
);
#endif
