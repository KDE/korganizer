/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <time.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>

#include "koprefs.h"

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

  KPrefs::setCurrentGroup("General");

  addItemBool("Enable Group Scheduling",&mEnableGroupScheduling,false);
  addItemBool("Enable Project View",&mEnableProjectView,false);
  addItemBool("Auto Save",&mAutoSave,false);
  addItemInt("Auto Save Interval",&mAutoSaveInterval,10);
  addItemBool("Confirm Deletes",&mConfirm,true);
  addItemString("Archive File",&mArchiveFile);
  addItemString("Html Export File",&mHtmlExportFile,
      QDir::homeDirPath() + "/" + i18n("Default export file", "calendar.html"));

  KPrefs::setCurrentGroup("Personal Settings");

  addItemInt("Mail Client",&mMailClient,MailClientKMail);
  addItemBool("Use Control Center Email",&mEmailControlCenter,false);
  addItemBool("Bcc",&mBcc,false);

  KPrefs::setCurrentGroup("Time & Date");

  addItemString("Time Zone",&mTimeZone,"+0000");
  addItemString("TimeZoneId",&mTimeZoneId);
  addItemInt("Default Start Time",&mStartTime,10);
  addItemInt("Default Duration",&mDefaultDuration,2);
  addItemInt("Default Alarm Time",&mAlarmTime,0);
  addItemInt("Daylight Savings",&mDaylightSavings,0);

  KPrefs::setCurrentGroup("AlarmDaemon");

  addItemBool("Autostart",&mAlarmdAutostart,true);

  KPrefs::setCurrentGroup("Calendar");

  addItemInt("Default Calendar Format",&mDefaultFormat,FormatICalendar);

  KPrefs::setCurrentGroup("Fonts");

  addItemFont("TimeBar Font",&mTimeBarFont);
  addItemFont("MonthView Font",&mMonthViewFont);
  addItemFont("AgendaView Font",&mAgendaViewFont);
  addItemFont("MarcusBains Font",&mAgendaViewFont);

  KPrefs::setCurrentGroup("Colors");

  addItemColor("Holiday Color",&mHolidayColor,defaultHolidayColor);
  addItemColor("Highlight Color",&mHighlightColor,defaultHighlightColor);
  addItemColor("Event Color",&mEventColor,mDefaultCategoryColor);
  addItemColor("Agenda Background Color",&mAgendaBgColor,defaultAgendaBgColor);
  addItemColor("WorkingHours Color",&mWorkingHoursColor,defaultWorkingHoursColor);

  KPrefs::setCurrentGroup("Views");

  addItemInt("Hour Size",&mHourSize,10);
  addItemBool("Show Daily Recurrences",&mDailyRecur,true);
  addItemBool("Show Weekly Recurrences",&mWeeklyRecur,true);
  addItemBool("Enable ToolTips",&mEnableToolTips,false);
  addItemBool("Enable MonthView ScrollBars",&mEnableMonthScroll,false);
  addItemBool("Marcus Bains shows seconds",&mMarcusBainsShowSeconds,false);
  addItemBool("Show Marcus Bains",&mMarcusBainsEnabled,true);

  addItemInt("Day Begins",&mDayBegins,7);
  addItemInt("Working Hours Start",&mWorkingHoursStart,8);
  addItemInt("Working Hours End",&mWorkingHoursEnd,17);
  addItemBool("Exclude Holidays",&mExcludeHolidays,true);
  addItemBool("Exclude Saturdays",&mExcludeSaturdays,true);

  addItemBool("Full View Month",&mFullViewMonth,false);
  addItemBool("Full View Todo",&mFullViewTodo,true);

  KPrefs::setCurrentGroup("Printer");

  KPrefs::setCurrentGroup("Layout");

  addItemBool("CompactDialogs",&mCompactDialogs,false);
  addItemBool("VerticalScreen",&mVerticalScreen,false);

  addItemString("Preview",&mPrintPreview,"kghostview");
  
  KPrefs::setCurrentGroup("KOrganizer Plugins");
  
  addItemStringList("SelectedPlugins",&mSelectedPlugins,"holidays");

  KPrefs::setCurrentGroup("Group Scheduling");

  addItemInt("IMIPScheduler",&mIMIPScheduler,IMIPKMail);
  addItemInt("IMIPSend",&mIMIPSend,IMIPOutbox);
  addItemStringList("AdditionalMails",&mAdditionalMails,"");
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

  KEMailSettings settings;
  mName = settings.getSetting(KEMailSettings::RealName);
  mEmail = settings.getSetting(KEMailSettings::RealName);
  fillMailDefaults();

  mHoliday = KGlobal::locale()->country();

  mTimeZone = "+0000";

  mTimeBarFont = mDefaultTimeBarFont;
  mMonthViewFont = mDefaultViewFont;
  mAgendaViewFont = mDefaultViewFont;
  mMarcusBainsFont = mDefaultViewFont;

  setCategoryDefaults();
  
  setTimeZoneIdDefault();
}

void KOPrefs::fillMailDefaults()
{
  if (mName.isEmpty()) mName = i18n("Anonymous");
  if (mEmail.isEmpty()) mEmail = i18n("nobody@nowhere");
}

void KOPrefs::setTimeZoneIdDefault()
{
  QString zone;

  char zonefilebuf[100];
  int len = readlink("/etc/localtime",zonefilebuf,100);
  if (len > 0 && len < 100) {
    zonefilebuf[len] = '\0';
    zone = zonefilebuf;
    zone = zone.mid(zone.find("zoneinfo/") + 9);
  } else {
    tzset();
    zone = tzname[0];
  }
  
  kdDebug () << "----- time zone: " << zone << endl;

  mTimeZoneId = zone;
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
  config()->setGroup("General");
  mCustomCategories = config()->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  config()->setGroup("Personal Settings");
  mName = config()->readEntry("user_name","");
  mEmail = config()->readEntry("user_email","");
  fillMailDefaults();
  mHoliday = config()->readEntry("Holidays", KGlobal::locale()->country());

  config()->setGroup("Category Colors");
  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    setCategoryColor(*it,config()->readColorEntry(*it,&mDefaultCategoryColor));
  }
  
  if (mTimeZoneId.isEmpty()) {
    setTimeZoneIdDefault();
  }
}


void KOPrefs::usrWriteConfig()
{
  config()->setGroup("General");
  config()->writeEntry("Custom Categories",mCustomCategories);

  config()->setGroup("Personal Settings");
  config()->writeEntry("user_name",mName);
  config()->writeEntry("user_email",mEmail);
  config()->writeEntry("Holidays",mHoliday);

  config()->setGroup("Category Colors");
  QDictIterator<QColor> it(mCategoryColors);
  while (it.current()) {
    config()->writeEntry(it.currentKey(),*(it.current()));
    ++it;
  }
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

void KOPrefs::setFullName(const QString &name)
{
  mName = name;
}

void KOPrefs::setEmail(const QString &email)
{
  mEmail = email;
}

QString KOPrefs::fullName()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::RealName);
  } else {
    return mName;
  }
}

QString KOPrefs::email()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::EmailAddress);
  } else {
    return mEmail;
  }
}
