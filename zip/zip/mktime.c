/* free mktime function
   Copyright 1988, 1989 by David MacKenzie <djm@ai.mit.edu>
   and Michael Haertel <mike@ai.mit.edu>
   Unlimited distribution permitted provided this copyright notice is
   retained and any functional modifications are prominently identified.  */

/* Revised 1997 by Christian Spieler:
   The code was changed to get more conformance with ANSI's (resp. modern
   UNIX releases) definition for mktime():
   - Added adjustment for out-of-range values in the fields of struct tm.
   - Added iterations to get the correct UTC result for input values at
     the gaps when daylight saving time is switched on or off.
   - Allow forcing of DST "on" or DST "off" by setting `tm_isdst' field in
     the tm struct to positive number resp. zero. The `tm_isdst' field must
     be negative on entrance of mktime() to enable automatic determination
     if DST is in effect for the requested local time.
   - Added optional check for overflowing the time_t range.  */

/* Note: This version of mktime is ignorant of the tzfile.
   When the tm structure passed to mktime represents a local time that
   is valid both as DST time and as standard time (= time value in the
   gap when switching from DST back to standard time), the behaviour
   for `tm_isdst < 0' depends on the current timezone: TZ east of GMT
   assumes winter time, TZ west of GMT assumes summer time.
   Although mktime() (resp. mkgmtime()) tries to adjust for invalid values
   of struct tm members, this may fail for input values that are far away
   from the valid ranges. The adjustment process does not check for overflows
   or wrap arounds in the struct tm components.  */

#ifndef OF
#  ifdef __STDC__
#    define OF(a) a
#  else
#    define OF(a) ()
#  endif
#endif

#ifndef ZCONST
#  define ZCONST const
#endif

#include <time.h>

time_t mkgmtime OF((struct tm *));
time_t mktime OF((struct tm *));

/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the local time and date in the exploded time structure `tm',
   adjust out of range fields in `tm' and set `tm->tm_yday', `tm->tm_wday'.
   If `tm->tm_isdst < 0' was passed to mktime(), the correct setting of
   tm_isdst is determined and returned. Otherwise, mktime() assumes this
   field as valid; its information is used when converting local time
   to UTC.
   Return -1 if time in `tm' cannot be represented as time_t value. */

time_t
mktime(tm)
     struct tm *tm;
{
  struct tm *ltm;               /* Local time. */
  time_t loctime;               /* The time_t value of local time. */
  time_t then;                  /* The time to return. */
  long tzoffset_adj;            /* timezone-adjustment `remainder' */
  int bailout_cnt;              /* counter of tries for tz correction */
  int save_isdst;               /* Copy of the tm->isdst input value */

  save_isdst = tm->tm_isdst;
  loctime = mkgmtime(tm);
  if (loctime == -1) {
    tm->tm_isdst = save_isdst;
    return (time_t)-1;
  }

  /* Correct for the timezone and any daylight savings time.
     The correction is verified and repeated when not correct, to
     take into account the rare case that a change to or from daylight
     savings time occurs between when it is the time in `tm' locally
     and when it is that time in Greenwich. After the second correction,
     the "timezone & daylight" offset should be correct in all cases. To
     be sure, we allow a third try, but then the loop is stopped. */
  bailout_cnt = 3;
  then = loctime;
  do {
    ltm = localtime(&then);
    if (ltm == (struct tm *)NULL ||
        (tzoffset_adj = loctime - mkgmtime(ltm)) == 0L)
      break;
    then += tzoffset_adj;
  } while (--bailout_cnt > 0);

  if (ltm == (struct tm *)NULL || tzoffset_adj != 0L) {
    /* Signal failure if timezone adjustment did not converge. */
    tm->tm_isdst = save_isdst;
    return (time_t)-1;
  }

  if (save_isdst >= 0) {
    if (ltm->tm_isdst  && !save_isdst)
    {
      if (then + 3600 < then)
        then = (time_t)-1;
      else
        then += 3600;
    }
    else if (!ltm->tm_isdst && save_isdst)
    {
      if (then - 3600 > then)
        then = (time_t)-1;
      else
        then -= 3600;
    }
    ltm->tm_isdst = save_isdst;
  }

  if (tm != ltm)  /* `tm' may already point to localtime's internal storage */
    *tm = *ltm;

  return then;
}


#ifndef NO_TIME_T_MAX
   /* Provide default values for the upper limit of the time_t range.
      These are the result of the decomposition into a `struct tm' for
      the time value 0xFFFFFFFEL ( = (time_t)-2 ).
      Note: `(time_t)-1' is reserved for "invalid time"!  */
#  ifndef TM_YEAR_MAX
#    define TM_YEAR_MAX         2106
#  endif
#  ifndef TM_MON_MAX
#    define TM_MON_MAX          1       /* February */
#  endif
#  ifndef TM_MDAY_MAX
#    define TM_MDAY_MAX         7
#  endif
#  ifndef TM_HOUR_MAX
#    define TM_HOUR_MAX         6
#  endif
#  ifndef TM_MIN_MAX
#    define TM_MIN_MAX          28
#  endif
#  ifndef TM_SEC_MAX
#    define TM_SEC_MAX          14
#  endif
#endif /* NO_TIME_T_MAX */

/* Adjusts out-of-range values for `tm' field `tm_member'. */
#define ADJUST_TM(tm_member, tm_carry, modulus) \
  if ((tm_member) < 0) { \
    tm_carry -= (1 - ((tm_member)+1) / (modulus)); \
    tm_member = (modulus-1) + (((tm_member)+1) % (modulus)); \
  } else if ((tm_member) >= (modulus)) { \
    tm_carry += (tm_member) / (modulus); \
    tm_member = (tm_member) % (modulus); \
  }

/* Nonzero if `y' is a leap year, else zero. */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

/* Additional leapday in February of leap years. */
#define leapday(m, y) ((m) == 1 && leap (y))

/* Length of month `m' (0 .. 11) */
#define monthlen(m, y) (ydays[(m)+1] - ydays[m] + leapday (m, y))

/* Accumulated number of days from 01-Jan up to start of current month. */
static ZCONST short ydays[] =
{
  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the Greenwich Mean time and date in the exploded time structure `tm'.
   This function does always put back normalized values into the `tm' struct,
   parameter, including the calculated numbers for `tm->tm_yday',
   `tm->tm_wday', and `tm->tm_isdst'.
   Returns -1 if the time in the `tm' parameter cannot be represented
   as valid `time_t' number. */

time_t
mkgmtime(tm)
     struct tm *tm;
{
  int years, months, days, hours, minutes, seconds;

  years = tm->tm_year + 1900;   /* year - 1900 -> year */
  months = tm->tm_mon;          /* 0..11 */
  days = tm->tm_mday - 1;       /* 1..31 -> 0..30 */
  hours = tm->tm_hour;          /* 0..23 */
  minutes = tm->tm_min;         /* 0..59 */
  seconds = tm->tm_sec;         /* 0..61 in ANSI C. */

  ADJUST_TM(seconds, minutes, 60)
  ADJUST_TM(minutes, hours, 60)
  ADJUST_TM(hours, days, 24)
  ADJUST_TM(months, years, 12)
  if (days < 0)
    do {
      if (--months < 0) {
        --years;
        months = 11;
      }
      days += monthlen(months, years);
    } while (days < 0);
  else
    while (days >= monthlen(months, years)) {
      days -= monthlen(months, years);
      if (++months >= 12) {
        ++years;
        months = 0;
      }
    }

  /* Restore adjusted values in tm structure */
  tm->tm_year = years - 1900;
  tm->tm_mon = months;
  tm->tm_mday = days + 1;
  tm->tm_hour = hours;
  tm->tm_min = minutes;
  tm->tm_sec = seconds;

  /* Set `days' to the number of days into the year. */
  days += ydays[months] + (months > 1 && leap (years));
  tm->tm_yday = days;

  /* Now calculate `days' to the number of days since Jan 1, 1970. */
  days = (unsigned)days + 365 * (unsigned)(years - 1970) +
         (unsigned)(nleap (years));
  tm->tm_wday = ((unsigned)days + 4) % 7; /* Jan 1, 1970 was Thursday. */
  tm->tm_isdst = 0;

  if (years < 1970)
    return (time_t)-1;

#if (defined(TM_YEAR_MAX) && defined(TM_MON_MAX) && defined(TM_MDAY_MAX))
#if (defined(TM_HOUR_MAX) && defined(TM_MIN_MAX) && defined(TM_SEC_MAX))
  if (years > TM_YEAR_MAX ||
      (years == TM_YEAR_MAX &&
       (tm->tm_yday > ydays[TM_MON_MAX] + (TM_MDAY_MAX - 1) +
                      (TM_MON_MAX > 1 && leap (TM_YEAR_MAX)) ||
        (tm->tm_yday == ydays[TM_MON_MAX] + (TM_MDAY_MAX - 1) +
                        (TM_MON_MAX > 1 && leap (TM_YEAR_MAX)) &&
         (hours > TM_HOUR_MAX ||
          (hours == TM_HOUR_MAX &&
           (minutes > TM_MIN_MAX ||
            (minutes == TM_MIN_MAX && seconds > TM_SEC_MAX) )))))))
    return (time_t)-1;
#endif
#endif

  return (time_t)(86400L * (unsigned long)(unsigned)days +
                  3600L * (unsigned long)hours +
                  (unsigned long)(60 * minutes + seconds));
}
