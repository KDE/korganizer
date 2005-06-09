%{
/*
 * deals with the holiday file. A yacc parser is used to parse the file.
 * All the holidays of the specified year are calculated at once and stored
 * in two arrays that have one entry for each day of the year. The day
 * drawing routines just use the julian date to index into these arrays.
 * There are two arrays because holidays can be printed either on a full
 * line under the day number, or as a small line to the right of the day
 * number. It's convenient to have both.
 *
 *	parse_holidays(year, force)	read the holiday file and evaluate
 *					all the holiday definitions for
 *					<year>. Sets holiday and sm_holiday
 *					arrays. If force is set, re-eval even
 *					if year is the same as last time.
 *
 * Taken from plan by Thomas Driemeyer (thomas@bitrot.de)
 * Adapted for use in KOrganizer by Preston Brown (pbrown@kde.org)
 */

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

/*** Macro definitions and constants ***/
/*
 * Before you mail and complain that the following macro is incorrect,
 * please consider that this is one of the main battlegrounds of the
 * Annual Usenet Flame Wars. 2000 is a leap year. Just trust me on this :-)
 */

#define ISLEAPYEAR(y)	!((y)&3)
#define JULIAN(m,d)	(monthbegin[m] + (d)-1+((m)>1 && ISLEAPYEAR(parse_year)))
#define LAST		999
#define ANY		0
#define	BEFORE		-1
#define AFTER		-2

/**** Private forward declarations ****/
extern int       kcallex(void);          /* external lexical analyzer */
static void      kcalerror(char *s);
static time_t    date_to_time(int day, int month, int year, 
			      int *wkday, int *julian, int *weeknum);
static time_t    tm_to_time(struct tm *tm);
static int	 day_from_name(char *str);
static int	 day_from_easter(void);
static int	 day_from_monthday(int m, int d);
static int	 day_from_wday(int day, int wday, int num);
static void	 monthday_from_day(int day, int *m, int *d, int *y);
static int	 calc_easter();
static void      setliteraldate();
static void      seteaster();
static void      setdate(int month, int day, int year, int off, int length);
static void      setwday(int num, int wday, int month, int off, int length); 
static void      setdoff(int wday, int rel, int month, int day, 
			 int year, int off, int length);
/*** Variables and structures ***/
static int	 m, d, y;
int              kcallineno;	       	/* current line # being parsed */
FILE            *kcalin;                  /* file currently being processed */
int	         yacc_small;		/* small string or on its own line? */
int	         yacc_stringcolor;	/* color of holiday name text, 1..8 */
char	        *yacc_string;		/* holiday name text */
int	         yacc_daycolor;		/* color of day number, 1..8 */
char	        *progname;		/* argv[0] */
int	         parse_year = -1;	/* year being parsed, 0=1970..99=2069*/
static char	*filename;		/* holiday filename */
static char	 errormsg[200];		/* error message if any, or "" */
static int	 easter_julian;		/* julian date of Easter Sunday */
static char	*holiday_name;		/* strdup'd yacc_string */
short 	         monthlen[12] = { 31, 28, 31, 30, 
				 31, 30, 31, 31,
				 30, 31, 30, 31 };
short	         monthbegin[12] = { 0, 31, 59, 90,
				    120, 151, 181, 
				    212, 243, 273,
				    304, 334 };

struct holiday {
  char            *string;        /* name of holiday, 0=not a holiday */
  unsigned short  dup;            /* reference count */
};

struct holiday	 holiday[366];		/* info for each day, separate for */
/*struct holiday   sm_holiday[366];*/	/* full-line texts under, and small */
					/* texts next to day number */
%}

%union { int ival; char *sval; }
%type	<ival> color offset length expr pexpr number month reldate
%token <ival> NUMBER MONTH WDAY COLOR
%token <sval> STRING
%token	IN PLUS MINUS SMALL CYEAR LEAPYEAR
%token	LENGTH EASTER EQ NE LE GE LT GT

%left OR
%left AND
%right EQ NE LE GE LT GT
%left '-' '+'
%left '*' '/' '%'
%nonassoc '!'
%nonassoc UMINUS
%left '?' ':'

%start list

%%
list	:
	| list small color STRING color		{ yacc_stringcolor = $3;
						  yacc_string	= $4;
						  yacc_daycolor	= $5; }
	  entry					{ free(yacc_string); }
	;

small	:					{ yacc_small = 0; }
	| SMALL					{ yacc_small = 1; }
	;

 color	:					{ $$ = 0; }
	 | COLOR					{ $$ = $1; }
	 ;

 entry	: EASTER offset length			{ seteaster($2, $3); }
	 | date offset length			{ setdate( m,  d,  y, $2, $3);}
	 | WDAY offset length			{ setwday( 0, $1,  0, $2, $3);}
	 | pexpr WDAY offset length		{ setwday($1, $2,  0, $3, $4);}
	 | pexpr WDAY IN month offset length	{ setwday($1, $2, $4, $5, $6);}
	 | WDAY pexpr date offset length		{ setdoff($1, $2,m,d,y,$4,$5);}
	 ;

 offset	:					{ $$ =	0; }
	 | PLUS expr				{ $$ =	$2; }
	 | MINUS expr				{ $$ = -$2; }
	 ;

 length	:					{ $$ =	1; }
	 | LENGTH expr				{ $$ =	$2; }
	 ;

 date	: pexpr '.' month			{ m = $3; d = $1; y = 0;  }
	 | pexpr '.' month '.'			{ m = $3; d = $1; y = 0;  }
	 | pexpr '.' month '.' expr		{ m = $3; d = $1; y = $5; }
	 | month '/' pexpr			{ m = $1; d = $3; y = 0;  }
	 | month '/' pexpr '/' pexpr		{ m = $1; d = $3; y = $5; }
	 | MONTH pexpr				{ m = $1; d = $2; y = 0;  }
	 | MONTH pexpr pexpr			{ m = $1; d = $2; y = $3; }
	 | pexpr MONTH				{ m = $2; d = $1; y = 0;  }
	 | pexpr MONTH pexpr			{ m = $2; d = $1; y = $3; }
	 | pexpr '.' MONTH pexpr			{ m = $3; d = $1; y = $4; }
	 | pexpr					{ monthday_from_day($1,
								 &m, &d, &y); }
	 ;

 reldate	: STRING				{ $$ = day_from_name($1); }
	 | EASTER				{ $$ = day_from_easter(); }
	 | pexpr '.' month			{ $$ = day_from_monthday
								 ($3, $1); }
	 | pexpr '.' month '.'			{ $$ = day_from_monthday
								 ($3, $1); }
	 | month '/' pexpr			{ $$ = day_from_monthday
								 ($1, $3); }
	 | pexpr MONTH				{ $$ = day_from_monthday
								 ($2, $1); }
	 | MONTH pexpr				{ $$ = day_from_monthday
								 ($1, $2); }
	 | WDAY pexpr pexpr			{ $$ = day_from_wday($3, $1,
							 $2 == -1 ? -1 : 0); }
	 | pexpr WDAY IN month			{ int d=day_from_monthday($4,1);
						   $$ = $1 == 999
						    ? day_from_wday(d+1,$2,-1)
						    : day_from_wday(d,$2,$1-1);}
	 ;

 month	: MONTH | pexpr;

 expr	: pexpr					{ $$ = $1; }
	 | expr OR  expr				{ $$ = $1 || $3; }
	 | expr AND expr				{ $$ = $1 && $3; }
	 | expr EQ  expr				{ $$ = $1 == $3; }
	 | expr NE  expr				{ $$ = $1 != $3; }
	 | expr LE  expr				{ $$ = $1 <= $3; }
	 | expr GE  expr				{ $$ = $1 >= $3; }
	 | expr LT  expr				{ $$ = $1 <  $3; }
	 | expr GT  expr				{ $$ = $1 >  $3; }
	 | expr '+' expr				{ $$ = $1 +  $3; }
	 | expr '-' expr				{ $$ = $1 -  $3; }
	 | expr '*' expr				{ $$ = $1 *  $3; }
	 | expr '/' expr				{ $$ = $3 ?  $1 / $3 : 0; }
	 | expr '%' expr				{ $$ = $3 ?  $1 % $3 : 0; }
	 | expr '?' expr ':' expr		{ $$ = $1 ?  $3 : $5; }
	 | '!' expr				{ $$ = !$2; }
	 | '[' reldate ']'			{ $$ = $2; }
	 ;

 pexpr	: '(' expr ')'				{ $$ = $2; }
	 | number				{ $$ = $1; }
	 ;

 number	: NUMBER
	 | '-' NUMBER %prec UMINUS		{ $$ = -$2; }
	 | CYEAR					{ $$ = parse_year; }
	 | LEAPYEAR pexpr			{ $$ = !(($2) & 3); }
	 ;
%%
	 
/*** Private Yacc callbacks and helper functions ***/
static void kcalerror(char *msg)
{
  fprintf(stderr, "%s: %s in line %d of %s\n", progname,
	  msg, kcallineno+1, filename);
  if (!*errormsg)
    sprintf(errormsg,
	    "Problem with holiday file %s:\n%.80s in line %d",
	    filename, msg, kcallineno+1);
}

static time_t date_to_time(int day, int month, int year, 
			   int *wkday, int *julian, int *weeknum)
{
  struct tm               tm;
  time_t                  time;
  
  tm.tm_sec   = 0;
  tm.tm_min   = 0;
  tm.tm_hour  = 0;
  tm.tm_mday  = day;
  tm.tm_mon   = month;
  tm.tm_year  = year;
  time = tm_to_time(&tm);
  if (wkday)
    *wkday   = tm.tm_wday;
  if (julian)
    *julian  = tm.tm_yday;
  if (weeknum)
    *weeknum = 0
      ? tm.tm_yday / 7
      : tm.tm_yday ? ((tm.tm_yday - 1) /7) + 1 : 0;
  return(time == -1 || day != tm.tm_mday ? 0 : time);
} 

static time_t tm_to_time(struct tm *tm)
{
  time_t                  t;              /* return value */
  
  t  = monthbegin[tm->tm_mon]                     /* full months */
    + tm->tm_mday-1                              /* full days */
    + (!(tm->tm_year & 3) && tm->tm_mon > 1);    /* leap day this year*/
  tm->tm_yday = t;
  t += 365 * (tm->tm_year - 70)                   /* full years */
    + (tm->tm_year - 69)/4;                      /* past leap days */
  tm->tm_wday = (t + 4) % 7;
  
  t = t*86400 + tm->tm_hour*3600 + tm->tm_min*60 + tm->tm_sec;
  if (tm->tm_mday > monthlen[tm->tm_mon] +
      (!(tm->tm_year & 3) && tm->tm_mon == 1))
    return((time_t)-1);
  return(t);
} 

/*
 * set holiday by weekday (monday..sunday). The expression is
 * "every <num>-th <wday> of <month> plus <off> days". num and month
 * can be ANY or LAST.
 */

static void setwday(int num, int wday, int month, int off, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_num   = 0, max_num   = 4;
  int		m, n, d, l, mlen, wday1;
  int		dup = 0;
  
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (num != ANY)
    min_num = max_num = num-1;
  
  holiday_name = yacc_string;
  for (m=min_month; m <= max_month; m++) {
    (void)date_to_time(1, m, parse_year, &wday1, 0, 0);
    d = (wday-1 - (wday1-1) +7) % 7 + 1;
    mlen = monthlen[m] + (m==1 && ISLEAPYEAR(parse_year));
    if (num == LAST)
      for (l=0; l < length; l++)
	setliteraldate(m, d+28<=mlen ? d+28 : d+21,
		       off+l, &dup);
    else
      for (d+=min_num*7, n=min_num; n <= max_num; n++, d+=7)
	if (d >= 1 && d <= mlen)
	  for (l=0; l < length; l++)
	    setliteraldate(m,d,off+l,&dup);
  }
}

/*
 * set holiday by weekday (monday..sunday) date offset. The expression is
 * "every <wday> before/after <date> plus <off> days". 
 * (This routine contributed by Peter Littlefield <plittle@sofkin.ca>)
 */

static void setdoff(int wday, int rel, int month, int day, 
		    int year, int off, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_day   = 1, max_day   = 31;
  int		m, d, nd, l, wday1;
  int		dup = 0;
  
  if (year != ANY) {
    year %= 100;
    if (year < 70) year += 100;
    if (year != parse_year)
      return;
  }
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (day != ANY)
    min_day   = max_day   = day;
  
  holiday_name = yacc_string;
  for (m=min_month; m <= max_month; m++)
    if (day == LAST) {
      (void)date_to_time(monthlen[m], m, parse_year,
			 &wday1, 0, 0);
      nd = (((wday - wday1 + 7) % 7) -
	    ((rel == BEFORE) ? 7 : 0)) % 7;
      for (l=0; l < length; l++)
	setliteraldate(m,monthlen[m]+nd, off+l, &dup);
    } else
      for (d=min_day; d <= max_day; d++) {
	(void)date_to_time(d, m, parse_year,
			   &wday1, 0, 0);
	nd = (((wday - wday1 + 7) % 7) -
	      ((rel == BEFORE) ? 7 : 0)) % 7;
	for (l=0; l < length; l++)
	  setliteraldate(m, d+nd, off+l, &dup);
      }
}


/*
 * set holiday by date. Ignore holidays in the wrong year. The code is
 * complicated by expressions such as "any/last/any" (every last day of
 * the month).
 */

static void setdate(int month, int day, int year, int off, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_day   = 1, max_day   = 31;
  int		m, d, l;
  int		dup = 0;
  
  if (year != ANY) {
    year %= 100;
    if (year < 70) year += 100;
    if (year != parse_year)
      return;
  }
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (day != ANY)
    min_day   = max_day   = day;
  
  holiday_name = yacc_string;
  for (m=min_month; m <= max_month; m++)
    if (day == LAST)
      for (l=0; l < length; l++)
	setliteraldate(m, monthlen[m], off+l, &dup);
    else
      for (d=min_day; d <= max_day; d++)
	for (l=0; l < length; l++)
	  setliteraldate(m, d, off+l, &dup);
}


/*
 * After the two routines above have removed ambiguities (ANY) and resolved
 * weekday specifications, this routine registers the holiday in the holiday
 * array. There are two of these, for full-line holidays (they take away one
 * appointment line in the month calendar daybox) and "small" holidays, which
 * appear next to the day number. If the day is already some other holiday,
 * ignore the new one. <dup> is information stored for parse_holidays(), it
 * will free() the holiday name only if its dup field is 0 (because many
 * string fields can point to the same string, which was allocated only once
 * by the lexer, and should therefore only be freed once).
 */

static void setliteraldate(int month, int day, int off, int *dup)
{
  int julian = JULIAN(month, day) + off;
  /*  struct holiday *hp = yacc_small ? &sm_holiday[julian]
      : &holiday[julian]; */
  struct holiday *hp = &holiday[julian];

  if (julian >= 0 && julian <= 365 && !hp->string) {
    if (!*dup)
      holiday_name = strdup(holiday_name);
    hp->string	= holiday_name;
    hp->dup		= (*dup)++;
  }
}


/*
 * set a holiday relative to Easter
 */

static void seteaster(int off, int length)
{
  int		dup = 0;	/* flag for later free() */
  int julian = easter_julian + off;
  /*  struct holiday *hp = yacc_small ? &sm_holiday[julian]
      : &holiday[julian];*/
  struct holiday *hp = &holiday[julian];
  
  holiday_name = yacc_string;
  while (length-- > 0) {
    if (julian >= 0 && julian <= 365 && !hp->string) {
      if (!dup)
	holiday_name = strdup(holiday_name);
      hp->string	= holiday_name;
      hp->dup		= dup++;
    }
    julian++, hp++;
  }
}


/*
 * calculate Easter Sunday as a julian date. I got this from Armin Liebl
 * <liebla@informatik.tu-muenchen.de>, who got it from Knuth. I hope I got
 * all this right...
 */

static int calc_easter(int year)
{
  int golden, cent, grcor, clcor, extra, epact, easter;
  
  golden = (year/19)*(-19);
  golden += year+1;
  cent = year/100+1;
  grcor = (cent*3)/(-4)+12;
  clcor = ((cent-18)/(-25)+cent-16)/3;
  extra = (year*5)/4+grcor-10;
  epact = golden*11+20+clcor+grcor;
  epact += (epact/30)*(-30);
  if (epact<=0)
    epact += 30;
  if (epact==25) {
    if (golden>11)
      epact += 1;
  } else {
    if (epact==24)
      epact += 1;
  }
  easter = epact*(-1)+44;
  if (easter<21)
    easter += 30;
  extra += easter;
  extra += (extra/7)*(-7);
  extra *= -1;
  easter += extra+7;
  easter += 31+28+!(year&3)-1;
  return(easter);
}


/*
 * functions used for [] syntax: (Erwin Hugo Achermann <acherman@inf.ethz.ch>)
 *
 * day_from_name (str)			gets day from symbolic name
 * day_from_easter ()			gets day as easter sunday
 * day_from_monthday (m, d)		gets <day> from <month/day>
 * day_from_wday (day, wday, num)	gets num-th day (wday) after <day> day
 * monthday_from_day (day, *m, *d, *y)	gets month/day/cur_year from <day>
 */

static int day_from_name(char *str)
{
  int	i;
  char	*name;
  
  for (i=0; i < 366; i++) {
    name = holiday[i].string;
    if (name && !strcmp(str, name))
      return(i);
  }
  return(-1);
}


static int day_from_easter(void)
{
  return(easter_julian);
}


static int day_from_monthday(int m, int d)
{
  if (m == 13)
    return(365 + ISLEAPYEAR(parse_year));
  return(JULIAN(m - 1, d));
}


static void monthday_from_day(int day, int *m, int *d, int *y)
{
  int	i, len;
  
  *y = parse_year;
  *m = 0;
  *d = 0;
  if (day < 0)
    return;
  for (i=0; i < 12; i++) {
    len = monthlen[i] + (i == 1 && ISLEAPYEAR(parse_year));
    if (day < len) {
      *m = i + 1;
      *d = day + 1;
      break;
    }
    day -= len;
  }
}


static int day_from_wday(int day, int wday, int num)
{
  int	wkday, yday, weeknum;
  
  (void)date_to_time(1, 0, parse_year, &wkday, &yday, &weeknum);
  day += (wday - wkday - day + 1001) % 7;
  day += num * 7;
  return (day);
}

static char *resolve_tilde(char *path)
{
  struct passwd   *pw;                    /* for searching home dirs */
  static char     pathbuf[512];           /* path with ~ expanded */
  char            *p, *q;                 /* username copy pointers */
  char            *home = 0;              /* home dir (if ~ in path) */
  
  if (*path != '~')
    return(path);
  
  if (!path[1] || path[1] == '/') {
    *pathbuf = 0;
    if (!(home = getenv("HOME")))
      home = getenv("home");
  } else {
    for (p=path+1, q=pathbuf; *p && *p != '/'; p++, q++)
      *q = *p;
    *q = 0;
    if ((pw = getpwnam(pathbuf)))
      home = pw->pw_dir;
  }
  if (!home) {
    fprintf(stderr, "%s: can't evaluate ~%s in %s, using .\n",
	    progname, pathbuf, path);
    home = ".";
  }
  sprintf(pathbuf, "%s/%s", home, path+1);
  return(pathbuf);
}

/*** Public Functions ***/
/*
 * parse the holiday text file, and set up the holiday arrays for a year.
 * If year is -1, re-parse the last year parsed (this is used when the
 * holiday file changes). If there is a CPP_PATH, check if the executable
 * really exists, and if so, pipe the holioday files through it.
 * Return an error message if an error occurred, 0 otherwise.
 */

char *parse_holidays(const char *holidays, int year, short force)
{
  register struct holiday *hp;
  register int		d, n;
  short			piped = 0;
  char			buf[200];
  char *kdedir;

  if (year == parse_year && !force)
      return(0);
  if (year < 0)
      year = parse_year;
  parse_year = year;
  easter_julian = calc_easter(year + 1900);
  
  for (hp=holiday, d=0; d < 366; d++, hp++)
      if (hp->string) {
	  if (!hp->dup)
	      free(hp->string);
	  hp->string      = 0;
      }
  /*  for (hp=sm_holiday, d=0; d < 366; d++, hp++)
      if (hp->string) {
      if (!hp->dup)
      free(hp->string);
      hp->string      = 0;
      }*/

  filename = holidays;
  if (access(filename, R_OK)) return(0);
  kcalin = fopen(filename, "r");
  if (!kcalin) return(0);
  *errormsg = 0;
  kcallineno = 0;
  kcalparse();
  if (piped) pclose(kcalin);
  else fclose(kcalin);
  if (*errormsg) return(errormsg);

  return(0);
}
