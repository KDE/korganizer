// $Id$

#include <qdir.h>

#include "koprefs.h"

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>

KOPrefs *KOPrefs::mInstance = 0;


KOPrefs::KOPrefs() :
  KPrefs("korganizerrc")
{
  mCategoryColors.setAutoDelete(true);
  
  mDefaultCategoryColor           = QColor(196,196,196);
  QColor defaultHolidayColor      = QColor("red");
  QColor defaultHighlightColor    = QColor("blue");
  QColor defaultAgendaBgColor     = QColor(128,128,128);
  QColor defaultWorkingHoursColor = QColor(160,160,160);

  mDefaultTimeBarFont = QFont("helvetica",12,QFont::Bold);
  mDefaultViewFont = QFont("helvetica",12);

  KPrefsItem::setCurrentGroup("General");
  
  addPrefsItem(new KPrefsItemBool("Enable Group Scheduling",
                                  &mEnableGroupScheduling,false));

  KPrefsItem::setCurrentGroup("Colors");
  
  addPrefsItem(new KPrefsItemColor("Holiday Color",&mHolidayColor,
                                   defaultHolidayColor));
  addPrefsItem(new KPrefsItemColor("Highlight Color",&mHighlightColor,
                                   defaultHighlightColor));
  addPrefsItem(new KPrefsItemColor("Event Color",&mEventColor,
                                   mDefaultCategoryColor));
  addPrefsItem(new KPrefsItemColor("Agenda Background Color",&mAgendaBgColor,
                                   defaultAgendaBgColor));
  addPrefsItem(new KPrefsItemColor("WorkingHours Color",&mWorkingHoursColor,
                                   defaultWorkingHoursColor));

  KPrefsItem::setCurrentGroup("Views");
 
  addPrefsItem(new KPrefsItemBool("Show Daily Recurrences",&mDailyRecur,true));
  addPrefsItem(new KPrefsItemBool("Show Weekly Recurrences",&mWeeklyRecur,
                                  true));
  addPrefsItem(new KPrefsItemBool("Enable ToolTips",&mEnableToolTips,false));
  addPrefsItem(new KPrefsItemBool("Enable MonthView ScrollBars",
                                  &mEnableMonthScroll,false));

  addPrefsItem(new KPrefsItemInt("Day Begins",&mDayBegins,7));
  addPrefsItem(new KPrefsItemInt("Working Hours Start",&mWorkingHoursStart,8));
  addPrefsItem(new KPrefsItemInt("Working Hours End",&mWorkingHoursEnd,17));
  addPrefsItem(new KPrefsItemBool("Exclude Holidays",&mExcludeHolidays,true));
  addPrefsItem(new KPrefsItemBool("Exclude Saturdays",&mExcludeSaturdays,true));

  addPrefsItem(new KPrefsItemBool("Full View Month",&mFullViewMonth,false));
  addPrefsItem(new KPrefsItemBool("Full View Todo",&mFullViewTodo,true));
}


KOPrefs::~KOPrefs()
{
  kdDebug() << "KOPrefs::~KOPrefs()" << endl;

  delete mInstance;
  mInstance = 0;
}


KOPrefs *KOPrefs::instance()
{
  if (!mInstance) {
    mInstance = new KOPrefs();
    mInstance->readConfig();
  }
  
  return mInstance;
}

void KOPrefs::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  mAutoSave = false;
  mAutoSaveInterval = 10;
  mConfirm = true;

  KSimpleConfig config( QString::fromLatin1("emaildefaults"), false );
  config.setGroup("UserInfo");
  mName = config.readEntry( "FullName", i18n("Anonymous") );
  mEmail = config.readEntry( "EmailAddress", i18n("nobody@nowhere") );
  mBcc = false;

  mAdditional = "";
  mHoliday = KGlobal::locale()->country();
  
  mTimeZone = "+0000";
  mStartTime = 10;
  mDefaultDuration = 2;
  mAlarmTime = 0;
  mDaylightSavings = 0;

//  mDayBegins = 8;
  mHourSize = 10;
//  mDailyRecur = true;
//  mWeeklyRecur = true;
//  mEnableToolTips = false;

  mTimeBarFont = mDefaultTimeBarFont;
  mMonthViewFont = mDefaultViewFont;
  mAgendaViewFont = mDefaultViewFont;

//  mHolidayColor = mDefaultHolidayColor;
//  mHighlightColor = mDefaultHighlightColor;
//  mEventColor = mDefaultCategoryColor;
//  mAgendaBgColor = mDefaultAgendaBgColor;

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

void KOPrefs::usrReadConfig()
{
  kdDebug() << "KOPrefs::usrReadConfig()" << endl;

  mConfig->setGroup("General");
  mAutoSave = mConfig->readBoolEntry("Auto Save",false);
  mAutoSaveInterval = mConfig->readNumEntry("Auto Save Interval",10);
  mConfirm = mConfig->readBoolEntry("Confirm Deletes",true);
  mCustomCategories = mConfig->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  mArchiveFile = mConfig->readEntry("Archive File");
  mHtmlExportFile = mConfig->readEntry("Html Export File",QDir::homeDirPath() +
                                       "/calendar.html");

  mConfig->setGroup("Personal Settings");
  mName = mConfig->readEntry("user_name","");
  mEmail = mConfig->readEntry("user_email","");
  mAdditional = mConfig->readEntry("Additional","");
  mBcc = mConfig->readBoolEntry("Bcc",false);

  mHoliday = mConfig->readEntry("Holidays", KGlobal::locale()->country());
  
  mConfig->setGroup("Time & Date");
  mTimeZone = mConfig->readEntry("Time Zone","+0000");
  mStartTime = mConfig->readNumEntry("Default Start Time",10);
  mDefaultDuration = mConfig->readNumEntry("Default Duration",2);
  mAlarmTime = mConfig->readNumEntry("Default Alarm Time",0);
  mDaylightSavings = mConfig->readNumEntry("Daylight Savings", 0);

  mConfig->setGroup("Views");
//  mDayBegins = mConfig->readNumEntry("Day Begins",8);
  mHourSize = mConfig->readNumEntry("Hour Size",10);
//  mDailyRecur = mConfig->readBoolEntry("Show Daily Recurrences",true);
//  mWeeklyRecur = mConfig->readBoolEntry("Show Weekly Recurrences",true);
//  mEnableToolTips = mConfig->readBoolEntry("Enable ToolTips",false);

  mConfig->setGroup("Fonts");
  mTimeBarFont = mConfig->readFontEntry("TimeBar Font",&mDefaultTimeBarFont);
  mMonthViewFont = mConfig->readFontEntry("MonthView Font",
                                          &mDefaultViewFont);
  mAgendaViewFont = mConfig->readFontEntry("AgendaView Font",
                                          &mDefaultViewFont);

  mConfig->setGroup("Colors");
//  mHolidayColor = mConfig->readColorEntry("Holiday Color",
//                                         &mDefaultHolidayColor);
//  mHighlightColor = mConfig->readColorEntry("Highlight Color",
//                                            &mDefaultHighlightColor);
//  mEventColor = mConfig->readColorEntry("Event Color",
//                                            &mDefaultCategoryColor);
//  mAgendaBgColor = mConfig->readColorEntry("Agenda Background Color",
//                                            &mDefaultAgendaBgColor);

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

  kdDebug() << "KOPrefs::usrReadConfig() done" << endl;  
}


void KOPrefs::usrWriteConfig()
{
//  kdDebug() << "KOPrefs::writeConfig()" << endl;

  mConfig->setGroup("General");
  mConfig->writeEntry("Auto Save",mAutoSave);
  mConfig->writeEntry("Auto Save Interval",mAutoSaveInterval);
  mConfig->writeEntry("Confirm Deletes",mConfirm);
  mConfig->writeEntry("Custom Categories",mCustomCategories);

  mConfig->writeEntry("Archive File",mArchiveFile);
  mConfig->writeEntry("Html Export File",mHtmlExportFile);

  mConfig->setGroup("Personal Settings");
  mConfig->writeEntry("user_name",mName);
  mConfig->writeEntry("user_email",mEmail);
  mConfig->writeEntry("Additional",mAdditional);
  mConfig->writeEntry("Bcc",mBcc);
  mConfig->writeEntry("Holidays",mHoliday);

  mConfig->setGroup("Time & Date");
  mConfig->writeEntry("Time Zone",mTimeZone);
  mConfig->writeEntry("Default Start Time",mStartTime);
  mConfig->writeEntry("Default Duration",mDefaultDuration);
  mConfig->writeEntry("Default Alarm Time",mAlarmTime);
  mConfig->writeEntry("Daylight Savings",mDaylightSavings);

  mConfig->setGroup("Views");
//  mConfig->writeEntry("Day Begins",mDayBegins);
  mConfig->writeEntry("Hour Size",mHourSize);
//  mConfig->writeEntry("Show Daily Recurrences",mDailyRecur);
//  mConfig->writeEntry("Show Weekly Recurrences",mWeeklyRecur);
//  mConfig->writeEntry("Enable ToolTips",mEnableToolTips);

  mConfig->setGroup("Fonts");
  mConfig->writeEntry("TimeBar Font",mTimeBarFont);
  mConfig->writeEntry("MonthView Font",mMonthViewFont);
  mConfig->writeEntry("AgendaView Font",mAgendaViewFont);

  mConfig->setGroup("Colors");
//  mConfig->writeEntry("Holiday Color",mHolidayColor);
//  mConfig->writeEntry("Highlight Color",mHighlightColor);
//  mConfig->writeEntry("Event Color",mEventColor);
//  mConfig->writeEntry("Agenda Background Color",mAgendaBgColor);

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
