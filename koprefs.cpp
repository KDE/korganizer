// $Id$

#include "koprefs.h"

#include <kconfig.h>
#include <kstddirs.h>


KOPrefs *KOPrefs::mInstance = 0;


KOPrefs::KOPrefs()
{
  mConfig = new KConfig(locate("config","korganizerrc"));
  
  readConfig();
}


KOPrefs::~KOPrefs()
{
  qDebug ("KOPrefs::~KOPrefs()");
  delete mConfig;
  
  mInstance = 0;
}


KOPrefs *KOPrefs::instance()
{
  if (!mInstance) mInstance = new KOPrefs();
  
  return mInstance;
}

void KOPrefs::setDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  mAutoSave = false;
  mConfirm = true;

  mName = "";
  mEmail = "";
  mAdditional = "";
  mHoliday = "(none)";
  
  mTimeFormat = 0;
  mDateFormat = 0;
  mTimeZone = "";
  mStartTime = 10;
  mAlarmTime = 0;
  mWeekstart = true;
  mDaylightSavings = 0;

  mDayBegins = 8;
  mHourSize = 10;
  mDailyRecur = true;

  mTimeBarFont = QFont("helvetica",18);

  mHolidayColor = QColor("red");
  mHighlightColor = QColor("blue");

  mPrinter = "";
  mPaperSize = 0;
  mPaperOrientation = 0;
  mPrintPreview = "gv";
}

void KOPrefs::readConfig()
{
  mConfig->setGroup("General");
  mAutoSave = mConfig->readBoolEntry("Auto Save",false);
  mConfirm = mConfig->readBoolEntry("Confirm Deletes",true);
  mCustomCategories = mConfig->readListEntry("Custom Categories");

  mConfig->setGroup("Personal Settings");
  mName = mConfig->readEntry("user_name","");
  mEmail = mConfig->readEntry("user_email","");
  mAdditional = mConfig->readEntry("Additional","");

  mHoliday = mConfig->readEntry("Holidays","(none)");
  
  mConfig->setGroup("Time & Date");
  mTimeFormat = mConfig->readNumEntry("Time Format",0);
  mDateFormat = mConfig->readNumEntry("Date Format",0);
  mTimeZone = mConfig->readNumEntry("Time Zone",0);
  mStartTime = mConfig->readNumEntry("Default Start Time",10);
  mAlarmTime = mConfig->readNumEntry("Default Alarm Time",0);
  mWeekstart = mConfig->readBoolEntry("Week Starts Monday",true);
  mDaylightSavings = mConfig->readNumEntry("Daylight Savings", 0);

  mConfig->setGroup("Views");
  mDayBegins = mConfig->readNumEntry("Day Begins",8);
  mHourSize = mConfig->readNumEntry("Hour Size",10);
  mDailyRecur = mConfig->readBoolEntry("Show Daily Recurrences",true);

  mConfig->setGroup("Fonts");
  mTimeBarFont = mConfig->readFontEntry("TimeBar Font");

  mConfig->setGroup("Colors");
  mHolidayColor = mConfig->readColorEntry("Holiday Color");
  mHighlightColor = mConfig->readColorEntry("Highlight Color");

  mConfig->setGroup("Printer");
  mPrinter = mConfig->readEntry("Printer Name",0);
  mPaperSize = mConfig->readNumEntry("Paper Size",0);
  mPaperOrientation = mConfig->readNumEntry("Paper Orientation",0);
  mPrintPreview = mConfig->readEntry("Preview","gv");
}


void KOPrefs::writeConfig()
{
//  qDebug("KOPrefs::writeConfig()");

  mConfig->setGroup("General");
  mConfig->writeEntry("Auto Save",mAutoSave);
  mConfig->writeEntry("Confirm Deletes",mConfirm);
  mConfig->writeEntry("Custom Categories",mCustomCategories);

  mConfig->setGroup("Personal Settings");
  mConfig->writeEntry("user_name",mName);
  mConfig->writeEntry("user_email",mEmail);
  mConfig->writeEntry("Additional",mAdditional);
  mConfig->writeEntry("Holidays",mHoliday);

  mConfig->setGroup("Time & Date");
  mConfig->writeEntry("Time Format",mTimeFormat);
  mConfig->writeEntry("Date Format",mDateFormat);
  mConfig->writeEntry("Time Zone",mTimeZone);
  mConfig->writeEntry("Default Start Time",mStartTime);
  mConfig->writeEntry("Default Alarm Time",mAlarmTime);
  mConfig->writeEntry("Week Starts Monday",mWeekstart);
  mConfig->writeEntry("Daylight Savings",mDaylightSavings);

  mConfig->setGroup("Views");
  mConfig->writeEntry("Day Begins",mDayBegins);
  mConfig->writeEntry("Hour Size",mHourSize);
  mConfig->writeEntry("Show Daily Recurrences",mDailyRecur);

  mConfig->setGroup("Fonts");
  mConfig->writeEntry("TimeBar Font",mTimeBarFont);

  mConfig->setGroup("Colors");
  mConfig->writeEntry("Holiday Color",mHolidayColor);
  mConfig->writeEntry("Highlight Color",mHighlightColor);

  mConfig->setGroup("Printer");
  mConfig->writeEntry("Printer Name",mPrinter);
  mConfig->writeEntry("Paper Size",mPaperSize);
  mConfig->writeEntry("Paper Orientation",mPaperOrientation);
  mConfig->writeEntry("Preview",mPrintPreview);
  
  mConfig->sync();
}
