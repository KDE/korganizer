// Calendar class for KOrganizer
// (c) 1998 Preston Brown
// 	$Id$

#include "config.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>

#include <qclipboard.h>
#include <qdialog.h>

#include <kapp.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "qdatelist.h"
#include "koprefs.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "koexceptions.h"
#include "calfilter.h"

#include "calobject.h"
#include "calobject.moc"

extern "C" {
  char *parse_holidays(const char *, int year, short force);
  struct holiday {
    char            *string;        /* name of holiday, 0=not a holiday */
    unsigned short  dup;            /* reference count */
  };   
  extern struct holiday holiday[366];
};

CalObject::CalObject()
  : QObject()
{
  mFormat = new VCalFormat(this);
  mICalFormat = new ICalFormat(this);

  mFilter = new CalFilter;
  mFilter->setEnabled(false);

  struct passwd *pwent;
  uid_t userId;
  QString tmpStr;

  // initialization
  mTopWidget = 0;
  mDialogsOn = true;

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0L));

  // user information...
  userId = getuid();
  pwent = getpwuid(userId);
  tmpStr = KOPrefs::instance()->mName;
  if (tmpStr.isEmpty()) {    
    if (strlen(pwent->pw_gecos) > 0)
      setOwner(pwent->pw_gecos);
    else
      setOwner(pwent->pw_name);
      KOPrefs::instance()->mName = getOwner();
  } else {
    setOwner(tmpStr);
  }

  tmpStr = KOPrefs::instance()->mEmail;
  if (tmpStr.isEmpty()) {
    tmpStr = pwent->pw_name;
    tmpStr += "@";

#ifdef HAVE_GETHOSTNAME
    char cbuf[80];
    if (gethostname(cbuf, 79)) {
      // error getting hostname
      tmpStr += "localhost";
    } else {
      hostent he;
      if (gethostbyname(cbuf)) {
	  he = *gethostbyname(cbuf);
	  tmpStr += he.h_name;
      } else {
	// error getting hostname
	tmpStr += "localhost";
      }
    }
#else
    tmpStr += "localhost";
#endif
    setEmail(tmpStr);
    KOPrefs::instance()->mEmail = tmpStr;
  } else {
    setEmail(tmpStr);
  }

  readHolidayFileName();

  tmpStr = KOPrefs::instance()->mTimeZone;
//  kdDebug() << "CalObject::CalObject(): TimeZone: " << tmpStr << endl;
  int dstSetting = KOPrefs::instance()->mDaylightSavings;
  extern long int timezone;
  struct tm *now;
  time_t curtime;
  curtime = time(0);
  now = localtime(&curtime);
  int hourOff = - ((timezone / 60) / 60);
  if (now->tm_isdst)
    hourOff += 1;
  QString tzStr;
  tzStr.sprintf("%.2d%.2d",
		hourOff, 
		abs((timezone / 60) % 60));

  // if no time zone was in the config file, write what we just discovered.
  if (tmpStr.isEmpty()) {
    KOPrefs::instance()->mTimeZone = tzStr;
  } else {
    tzStr = tmpStr;
  }
  
  // if daylight savings has changed since last load time, we need
  // to rewrite these settings to the config file.
  if ((now->tm_isdst && !dstSetting) ||
      (!now->tm_isdst && dstSetting)) {
    KOPrefs::instance()->mTimeZone = tzStr;
    KOPrefs::instance()->mDaylightSavings = now->tm_isdst;
  }
  
  setTimeZone(tzStr);

  KOPrefs::instance()->writeConfig();
}

CalObject::~CalObject() 
{
  delete mFormat;
  delete mICalFormat;
}

ICalFormat *CalObject::iCalFormat()
{
  return mICalFormat;
}

VCalDrag *CalObject::createDrag(KOEvent *selectedEv, QWidget *owner)
{
  return mFormat->createDrag(selectedEv,owner);
}

VCalDrag *CalObject::createDragTodo(KOEvent *selectedEv, QWidget *owner)
{
  return mFormat->createDragTodo(selectedEv,owner);
}

KOEvent *CalObject::createDrop(QDropEvent *de)
{
  return mFormat->createDrop(de);
}

KOEvent *CalObject::createDropTodo(QDropEvent *de)
{
  kdDebug() << "CalObject::createDropTodo()" << endl;
  return mFormat->createDropTodo(de);
}

void CalObject::cutEvent(KOEvent *selectedEv)
{
  if (copyEvent(selectedEv))
    deleteEvent(selectedEv);
}

bool CalObject::copyEvent(KOEvent *selectedEv)
{
  return mFormat->copyEvent(selectedEv);
}

KOEvent *CalObject::pasteEvent(const QDate *newDate,const QTime *newTime)
{
  return mFormat->pasteEvent(newDate,newTime);
}

const QString &CalObject::getOwner() const
{
  return mOwner;
}

void CalObject::setOwner(const QString &os)
{
  int i;
  // mOwner = os.ascii(); // to detach it
  mOwner = os; // #### Why? This should be okay?
  i = mOwner.find(',');
  if (i != -1)
    mOwner = mOwner.left(i);
}

void CalObject::setTimeZone(const QString & tz)
{
  bool neg = FALSE;
  int hours, minutes;
  QString tmpStr(tz);

  if (tmpStr.left(1) == "-")
    neg = TRUE;
  if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
    tmpStr.remove(0, 1);
  hours = tmpStr.left(2).toInt();
  if (tmpStr.length() > 2) 
    minutes = tmpStr.right(2).toInt();
  else
    minutes = 0;
  mTimeZone = (60*hours+minutes);
  if (neg)
    mTimeZone = -mTimeZone;
}

QString CalObject::getTimeZoneStr() const 
{
  QString tmpStr;
  int hours = abs(mTimeZone / 60);
  int minutes = abs(mTimeZone % 60);
  bool neg = mTimeZone < 0;

  tmpStr.sprintf("%c%.2d%.2d",
		 (neg ? '-' : '+'),
		 hours, minutes);
  return tmpStr;
}

void CalObject::setTopwidget(QWidget *topWidget)
{
  mTopWidget = topWidget;
}

const QString &CalObject::getEmail()
{
  return mOwnerEmail;
}

void CalObject::setEmail(const QString &e)
{
  mOwnerEmail = e;
}

void CalObject::setTimeZone(int tz)
{
  mTimeZone = tz;
}

int CalObject::getTimeZone() const
{
  return mTimeZone;
}

void CalObject::showDialogs(bool d)
{
  mDialogsOn = d;
}

// don't ever call this unless a kapp exists!
void CalObject::updateConfig()
{
//  kdDebug() << "CalObject::updateConfig()" << endl;

  bool updateFlag = FALSE;

  mOwner = KOPrefs::instance()->mName;

  // update events to new organizer (email address) 
  // if it has changed...
  QString configEmail = KOPrefs::instance()->mEmail;

  if (mOwnerEmail != configEmail) {
    QString oldEmail = mOwnerEmail;
    //    oldEmail.detach();
    mOwnerEmail = configEmail;
    KOEvent *firstEvent, *currEvent;
    bool atFirst = TRUE;

    firstEvent = last();
    // gotta skip over the first one, which is same as first. 
    // I know, bad coding.
    for (currEvent = prev(); currEvent; currEvent = prev()) {
//      kdDebug() << "in calobject::updateConfig(), currEvent summary: " << currEvent->getSummary() << endl;
      if ((currEvent == firstEvent) && !atFirst) {
	break;
      }
      if (currEvent->getOrganizer() == oldEmail) {
	currEvent->setReadOnly(FALSE);
	currEvent->setOrganizer(mOwnerEmail);
        updateFlag = TRUE;
      }
      atFirst = FALSE;
    }
  }

  readHolidayFileName();

  setTimeZone(KOPrefs::instance()->mTimeZone.latin1());

  if (updateFlag)
    emit calUpdated((KOEvent *) 0L);
}

QString CalObject::getHolidayForDate(const QDate &qd)
{
  static int lastYear = 0;

//  kdDebug() << "CalendarLocal::getHolidayForDate(): Holiday: " << holidays << endl;

  if (mHolidayfile.isEmpty()) return (QString(""));

  if ((lastYear == 0) || (qd.year() != lastYear)) {
      lastYear = qd.year() - 1900; // silly parse_year takes 2 digit year...
      parse_holidays(mHolidayfile.latin1(), lastYear, 0);
  }

  if (holiday[qd.dayOfYear()-1].string) {
    QString holidayname = QString(holiday[qd.dayOfYear()-1].string);
//    kdDebug() << "Holi name: " << holidayname << endl;
    return(holidayname);
  } else {
//    kdDebug() << "No holiday" << endl;
    return(QString(""));
  }
}

void CalObject::readHolidayFileName()
{
  QString holidays(KOPrefs::instance()->mHoliday);
  if (holidays == "(none)") mHolidayfile = "";
  holidays = holidays.prepend("holiday_");
  mHolidayfile = locate("appdata",holidays);

//  kdDebug() << "holifile: " << mHolidayfile << endl;
}

void CalObject::setFilter(CalFilter *filter)
{
  delete mFilter;
  mFilter = filter;
}

CalFilter *CalObject::filter()
{
  return mFilter;
}

QList<KOEvent> CalObject::getEventsForDate(const QDate &date,bool sorted)
{
  QList<KOEvent> el = eventsForDate(date,sorted);
  mFilter->apply(&el);
  return el;
}

QList<KOEvent> CalObject::getEventsForDate(const QDateTime &qdt)
{
  QList<KOEvent> el = eventsForDate(qdt);
  mFilter->apply(&el);
  return el;
}

QList<KOEvent> CalObject::getEvents(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QList<KOEvent> el = events(start,end,inclusive);
  mFilter->apply(&el);
  return el;
}
