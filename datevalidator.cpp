// $Id$

#include <stdio.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qapplication.h>
#include <qlineedit.h>

#include "datevalidator.h"
#include "datevalidator.moc"



DateValidator::DateValidator(QWidget *parent, int default_m, int default_y, int time_window, const char *name)
  : QValidator(parent,name)
{ 
  // the constructor does nearly nothing. But wait until you see the dtor!
  default_month=default_m;
  default_year=default_y;
  time_win=time_window;
}

DateValidator::~DateValidator(void)
{ }




QValidator::State DateValidator::translateToDate(QString &str, QDate &dat)
{
  DateDescription ddescr;
  State stat;
  int dummy;

  if((stat=parsedate(str,dummy,ddescr))==Acceptable)
    {
      if(QDate::isValid(ddescr.year,ddescr.month,ddescr.day))
	dat.setYMD(ddescr.year,ddescr.month,ddescr.day);
      else
	stat=Valid;
    };
  return stat;
}

QValidator::State DateValidator::validate(QString &str, int &cursor) const {
  DateDescription dummy;
  //  return parsedate(str,cursor,dummy);
  return 0;
}


QString &DateValidator::reFormat(QString &str, DateFormat form)
{
  int cpos=1;
  DateDescription ddescr;

  if(parsedate(str,cpos,ddescr)==Acceptable)
    {
      switch(form==Unknown ? ddescr.format : form)
	{
	case German:
	  str.sprintf("%02i.%02i.%i",ddescr.day,ddescr.month,ddescr.year);
	  break;
	case English_Slash:
	  str.sprintf("%02i/%02i/%i",ddescr.month,ddescr.day,ddescr.year);
	  break;
	case English_Minus:
	  str.sprintf("%02i-%02i-%i",ddescr.month,ddescr.day,ddescr.year);
	  break;
	case Unknown:
	  break;
	};
    };
  return str;
}






#define ISWHITESPACE(c) ( ((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r') )
void DateValidator::skipWhiteSpace(const QString &str, int &pos)
{
  while( (pos<str_length) && ISWHITESPACE(str[pos]) )
    pos++;
}


// res: Number, -1 reached end, -2 garbage
int DateValidator::findNumber(const QString &str, int &pos, int &startpos)
{
  skipWhiteSpace(str,pos);
  
  if(pos>=str_length)
    return -1;

  startpos=pos;
  while( (pos<str_length) && (str[pos]>='0') && (str[pos]<='9') )
    pos++;

  if(startpos==pos)
    return -2;

  return str.mid(startpos,pos-startpos).toInt();
}


// res: char, 0 reached end
char DateValidator::findSeparator(const QString &str, int &pos, int &seppos)
{
  skipWhiteSpace(str,pos);

  if(pos>=str_length)
    {
      seppos=-1;
      return 0;
    };

  seppos=pos;
  pos++;
  return str[seppos];
}




QValidator::State DateValidator::parsedate(const QString &str, int &cursorpos, DateDescription &ddescr) 
{
  int strpos=0,   // actual position in string
    errpos=-1;    // position of first error in string, or -1 for "no error"

  bool lookNumber, done;

  int numbers[3]; // the three numbers making up a date. Order depends on inputmode
  int numstart;
  char separator;
  int actnum=0,   // the index of the next number
    actsep=0,     // index of the next separator
    tottok=0,     // how many items/tokens have been parsed?
    seppos;       // position of last separator parsed

 
  /* For not having to call QString::length() frequently without knowing whether
   * that function starts to count it chars every time, we save that value
   */
  str_length=str.length();

  numbers[0]=numbers[1]=numbers[2]=-1;

  ddescr.format=Unknown;

  lookNumber=TRUE;
  done=FALSE;

  /* We parse the string until
   * - there is an error ( errpos!=-1 ), or
   * - we reach the end of the string ( strpos>=str.length() ), or
   * - we found everything that makes up a date
   */
  while((errpos==-1)&&(strpos<str_length)&&(!done))
    if(lookNumber)
      {
	// We are currently looking for a number
	if(( numbers[actnum]=findNumber(str,strpos,numstart) )==-1)
	    // but be reached the end of the string
	    done=TRUE;
	else
	  {
	    /* if num==-2, this means that there was anything else.
	     * this could mean
	     * that the user deleted the number in-between some separators
	     * and is just about to enter a new number
	     */
	    if(numbers[actnum]==-2)
	      numbers[actnum]=-1;

	    // since we found a number, we increase the counters
	    actnum++;
	    tottok++;

	    /* if we found a total of three numbers, we're done.
	     * if not, there should come a separator
	     */
	    if(actnum==3)
	      done=TRUE;
	    else
	      lookNumber=FALSE;
	  };//if
      }
    else
      {
	// We are currently looking for a separator
	switch(actsep)
	  {
	  case 0:
	    // It's the first sep, so look what the user preferres
	    separator=findSeparator(str,strpos,seppos);
	    switch(separator)
	      {
	      case '.':
		// german format 'dd.mm.yyyy'
		ddescr.format=German;
		break;
	      case '-':
		ddescr.format=English_Minus;
		break;
	      case '/':
		// normal format 'mm/dd/yyyy' or 'mm-dd-yyyy'
		ddescr.format=English_Slash;
		break;
	      default:
		// anything else we did not expect
		errpos=seppos;
	      };
	    break;//case first sep

	  case 1:
	    // The second sep must be the same as the first (Not 1-1/2000)
	    if(separator!=findSeparator(str,strpos,seppos))
	      errpos=seppos;
	    break;//case second sep

	  };// switch(actsep)

	// Increase all the counters
	actsep++;
	tottok++;

	lookNumber=TRUE;
      };

  /* We're through parsing.
   *
   * If there was no error, this could mean that
   * 1) the string ended before we found a complete date
   * 2) We found a complete date
   *
   * In the second case, there could be non-whitespace garbage at the end of the
   * string, which leads to an error.
   *
   * The test does nothing in the first case, since the string is already used up.
   */
  if(errpos==-1)
    if(strpos<str_length)
      {
	skipWhiteSpace(str,strpos);
	if(strpos<str_length)
	  // There is garbage. ERROR!!!
	  errpos=strpos;
      };


  // If there was an error, we can't do anymore.
  if(errpos!=-1)
    {
      cursorpos=errpos;
      return Invalid;
    };



  /* Now, we have anything the user gave us.
   *
   * So we can now check whether
   * - the user gave us enough
   * - he entered a real date
   * - he used a two-digit year which is not nice
   */


  // First, we sort the three numbers into day, month and year
  switch(ddescr.format)
    {
    case German:
      ddescr.day=numbers[0];
      ddescr.month=numbers[1];
      ddescr.year=numbers[2];
      break;
    case English_Slash:
    case English_Minus:
      ddescr.day=numbers[1];
      ddescr.month=numbers[0];
      ddescr.year=numbers[2];
      break;
    case Unknown:
      break;
    };


  insertMissing(ddescr);
  return makeState(ddescr);
}



QValidator::State DateValidator::makeState(DateDescription &ddescr)
{
  State result=Acceptable;


  // if the format is Unknown, this is Valid in any case
  if(ddescr.format==Unknown)
    return Valid;


  // First, checks that lead to Valid

  /* If a number equals -1, it means that the user did not yet type that
   * number. This is Acceptable if a default is given, but Valid if not.
   * The day never has a default.
   * If a number is 0, then this could mean y2k or garbage.
   */
  if(ddescr.day<1)
    result=Valid;
  else if(ddescr.month<1)
    result=Valid;
  else if(ddescr.year==-1)
    result=Valid;
  else if(ddescr.year<1574)
    result=Valid;

  // Now, checks that lead to Invalid
  if(ddescr.day>31)
    result=Invalid;
  else if(ddescr.month>12)
    result=Invalid;
  else if(ddescr.year>8000)
    result=Invalid;

  return result;
}

void DateValidator::insertMissing(DateDescription &ddescr)
{
  if(ddescr.month==-1)
    ddescr.month=default_month;

  if(ddescr.year==-1)
    ddescr.year=default_year;
  else if(ddescr.year<100)
    if(time_win != -1)
      /* The user entered only 1 or 2 digits for the year (or 00000002 :-) )
       * AND we have a time window
       * So, if the year is less then the last two digits of time_win,
       * the meant year is one century after the time_win's century,
       * if its not, its in the same century.
       */
      if(ddescr.year < (time_win % 100))
	ddescr.year+=((time_win/100)+1)*100;
      else
	ddescr.year+=(time_win/100)*100;
}

int DateValidator::defaultMonth(void)
{
  return default_month;
}

int DateValidator::defaultYear(void)
{
  return default_year;
}

int DateValidator::timeWindow(void)
{
  return time_win;
}

void DateValidator::setDefaultMonth(int m)
{
  if((m>0)&&(m<13))
    default_month=m;
  else
    default_month=-1;
}


void DateValidator::setDefaultYear(int y)
{
  if((y>1752)&&(y<8000))
    default_year=y;
  else
    default_year=-1;
}
 


void DateValidator::setTimeWindow(int y)
{
  if((y>1752)&&(y<8000))
    time_win=y;
  else
    time_win=-1;
}
 
QString &DateValidator::reFormat(QString &str, const QDate &dat, DateFormat f)
{
  if(dat.isValid())
    if(f!=Unknown)
      dprintf(str,f,dat.day(),dat.month(),dat.year());
  return str;
}


DateValidator::DateFormat DateValidator::format(const QString &str)
{
  DateDescription ddescr;
  int dummy;

  parsedate(str,dummy,ddescr);
  return ddescr.format;
}



void DateValidator::dprintf(QString &str, DateFormat f, int d, int m, int y)
{
  switch(f)
    {
    case German:
      str.sprintf("%02i.%02i.%i",d,m,y);
      break;
    case English_Slash:
      str.sprintf("%02i/%02i/%i",m,d,y);
      break;
    case English_Minus:
      str.sprintf("%02i-%02i-%i",m,d,y);
      break;
    case Unknown:
      break;
    };
}
