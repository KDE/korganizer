/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef _KOPREFS_H
#define _KOPREFS_H
// $Id$

#include <qdict.h>

#include "kprefs.h"

class KConfig;
class QFont;
class QColor;
class QStringList;

class KOPrefs : public KPrefs
{
  public:
    enum { FormatVCalendar, FormatICalendar };
    enum { MailClientKMail, MailClientSendmail };
    enum { IMIPDummy, IMIPKMail };
    enum { IMIPOutbox, IMIPdirectsend };
  
    virtual ~KOPrefs();
  
    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();
  
    /** Set preferences to default values */
    void usrSetDefaults();
  
    /** Read preferences from config file */
    void usrReadConfig();

    /** Write preferences to config file */
    void usrWriteConfig();

  protected:
    void setCategoryDefaults();
    void setTimeZoneIdDefault();

    /** Fill empty mail fields with default values. */
    void fillMailDefaults();

  private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();

    static KOPrefs *mInstance;

  public:
    // preferences data
    void setFullName(const QString &);
    QString fullName();
    void setEmail(const QString &);
    QString email();

    QString mAdditional;

    bool    mEmailControlCenter;

    bool    mBcc;
    QString mHoliday;
    bool    mAutoSave;
    int     mAutoSaveInterval;
    bool    mConfirm;
  
    bool    mEnableGroupScheduling;
    bool    mEnableProjectView;
  
    int     mDefaultFormat;
    int     mMailClient;

    bool    mAlarmdAutostart;
  
    QString mTimeZone;
    QString mTimeZoneId;
    int     mStartTime;
    int     mDefaultDuration;
    int     mAlarmTime;
    int     mDaylightSavings;

    int     mWorkingHoursStart;
    int     mWorkingHoursEnd;
    bool    mExcludeHolidays;
    bool    mExcludeSaturdays;
    bool    mMarcusBainsShowSeconds;

    QFont   mTimeBarFont;
    QFont   mMonthViewFont;
    QFont   mAgendaViewFont;
    QFont   mMarcusBainsFont;

    QColor  mHolidayColor;
    QColor  mHighlightColor;
    QColor  mEventColor;
    QColor  mAgendaBgColor;
    QColor  mWorkingHoursColor;

    int     mDayBegins;
    int     mHourSize;
    bool    mDailyRecur;
    bool    mWeeklyRecur;
    bool    mEnableToolTips;
    bool    mEnableMonthScroll;
    bool    mFullViewMonth;
    bool    mFullViewTodo;
    bool    mMarcusBainsEnabled;

    QString mPrintPreview;

    QStringList mCustomCategories;

    void setCategoryColor(QString cat,const QColor & color);
    QColor *categoryColor(QString cat);

    QString mArchiveFile;
    QString mHtmlExportFile;

    QStringList mSelectedPlugins;

    int mIMIPScheduler;
    int mIMIPSend;

  private:
    QDict<QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QFont mDefaultTimeBarFont;
    QFont mDefaultViewFont;

    QString mName;
    QString mEmail;
};

#endif
