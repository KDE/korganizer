// $Id$

#include "config.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include "koprefs.h"

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>

KOPrefs *KOPrefs::mInstance = 0;


KOPrefs::KOPrefs()
{
  mCategoryColors.setAutoDelete(true);
  
  mDefaultCategoryColor  = QColor(196,196,196);
  mDefaultHolidayColor   = QColor("red");
  mDefaultHighlightColor = QColor("blue");
  mDefaultAgendaBgColor  = QColor(128,128,128);

  mDefaultTimeBarFont = QFont("helvetica",12,QFont::Bold);
  
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

  KSimpleConfig config( QString::fromLatin1("emaildefaults"), false );
  config.setGroup("UserInfo");
  mName = config.readEntry( "FullName", i18n("Anonymous") );
  mEmail = config.readEntry( "EmailAddress", i18n("nobody@nowhere") );

  mAdditional = "";
  mHoliday = KGlobal::locale()->country();
  
  mTimeZone = "";
  mStartTime = 10;
  mDefaultDuration = 2;
  mAlarmTime = 0;
  mDaylightSavings = 0;

  mDayBegins = 8;
  mHourSize = 10;
  mDailyRecur = true;
  mWeeklyRecur = true;
  mEnableToolTips = false;

  mTimeBarFont = mDefaultTimeBarFont;

  mHolidayColor = mDefaultHolidayColor;
  mHighlightColor = mDefaultHighlightColor;
  mEventColor = mDefaultCategoryColor;
  mAgendaBgColor = mDefaultAgendaBgColor;

  mPrinter = "";
  mPaperSize = 0;
  mPaperOrientation = 0;
  mPrintPreview = "kghostview";
  
  setCategoryDefaults();
}

void KOPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();

  mCustomCategories << i18n("Appointment") << i18n("Business")
      << i18n("Meeting") << i18n("Phone Call") << i18n("Education")
      << i18n("Holiday") << i18n("Vacation") << i18n("Special Occasion")
      << i18n("Personal") << i18n("Travel") << i18n("Miscellaneous")
      << i18n("Birthday");

  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    setCategoryColor(*it,mDefaultCategoryColor);
  }
}

void KOPrefs::readConfig()
{
  mConfig->setGroup("General");
  mAutoSave = mConfig->readBoolEntry("Auto Save",false);
  mConfirm = mConfig->readBoolEntry("Confirm Deletes",true);
  mCustomCategories = mConfig->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  mConfig->setGroup("Personal Settings");
  mName = mConfig->readEntry("user_name","");
  mEmail = mConfig->readEntry("user_email","");
  mAdditional = mConfig->readEntry("Additional","");

  mHoliday = mConfig->readEntry("Holidays", KGlobal::locale()->country());
  
  mConfig->setGroup("Time & Date");
  mTimeZone = mConfig->readNumEntry("Time Zone",0);
  mStartTime = mConfig->readNumEntry("Default Start Time",10);
  mDefaultDuration = mConfig->readNumEntry("Default Duration",2);
  mAlarmTime = mConfig->readNumEntry("Default Alarm Time",0);
  mDaylightSavings = mConfig->readNumEntry("Daylight Savings", 0);

  mConfig->setGroup("Views");
  mDayBegins = mConfig->readNumEntry("Day Begins",8);
  mHourSize = mConfig->readNumEntry("Hour Size",10);
  mDailyRecur = mConfig->readBoolEntry("Show Daily Recurrences",true);
  mWeeklyRecur = mConfig->readBoolEntry("Show Weekly Recurrences",true);
  mEnableToolTips = mConfig->readBoolEntry("Enable ToolTips",false);

  mConfig->setGroup("Fonts");
  mTimeBarFont = mConfig->readFontEntry("TimeBar Font",&mDefaultTimeBarFont);

  mConfig->setGroup("Colors");
  mHolidayColor = mConfig->readColorEntry("Holiday Color",
                                          &mDefaultHolidayColor);
  mHighlightColor = mConfig->readColorEntry("Highlight Color",
                                            &mDefaultHighlightColor);
  mEventColor = mConfig->readColorEntry("Event Color",
                                            &mDefaultCategoryColor);
  mAgendaBgColor = mConfig->readColorEntry("Agenda Background Color",
                                            &mDefaultAgendaBgColor);

  mConfig->setGroup("Category Colors");
  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    setCategoryColor(*it,mConfig->readColorEntry(*it,&mDefaultCategoryColor));
  }

  mConfig->setGroup("Printer");
  mPrinter = mConfig->readEntry("Printer Name",0);
  mPaperSize = mConfig->readNumEntry("Paper Size",0);
  mPaperOrientation = mConfig->readNumEntry("Paper Orientation",0);
  mPrintPreview = mConfig->readEntry("Preview","kghostview");
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
  mConfig->writeEntry("Time Zone",mTimeZone);
  mConfig->writeEntry("Default Start Time",mStartTime);
  mConfig->writeEntry("Default Duration",mDefaultDuration);
  mConfig->writeEntry("Default Alarm Time",mAlarmTime);
  mConfig->writeEntry("Daylight Savings",mDaylightSavings);

  mConfig->setGroup("Views");
  mConfig->writeEntry("Day Begins",mDayBegins);
  mConfig->writeEntry("Hour Size",mHourSize);
  mConfig->writeEntry("Show Daily Recurrences",mDailyRecur);
  mConfig->writeEntry("Show Weekly Recurrences",mWeeklyRecur);
  mConfig->writeEntry("Enable ToolTips",mEnableToolTips);

  mConfig->setGroup("Fonts");
  mConfig->writeEntry("TimeBar Font",mTimeBarFont);

  mConfig->setGroup("Colors");
  mConfig->writeEntry("Holiday Color",mHolidayColor);
  mConfig->writeEntry("Highlight Color",mHighlightColor);
  mConfig->writeEntry("Event Color",mEventColor);
  mConfig->writeEntry("Agenda Background Color",mAgendaBgColor);

  mConfig->setGroup("Category Colors");
  QDictIterator<QColor> it(mCategoryColors);
  while (it.current()) {
    mConfig->writeEntry(it.currentKey(),*(it.current()));
    ++it;
  }

  mConfig->setGroup("Printer");
  mConfig->writeEntry("Printer Name",mPrinter);
  mConfig->writeEntry("Paper Size",mPaperSize);
  mConfig->writeEntry("Paper Orientation",mPaperOrientation);
  mConfig->writeEntry("Preview",mPrintPreview);
  
  mConfig->sync();
}

void KOPrefs::setCategoryColor(QString cat,const QColor & color)
{
  mCategoryColors.replace(cat,new QColor(color));
}

QColor *KOPrefs::categoryColor(QString cat)
{
  QColor *color = 0;
  
  if (!cat.isEmpty()) color = mCategoryColors[cat];
  
  if (color) return color;
  else return &mDefaultCategoryColor;
}
