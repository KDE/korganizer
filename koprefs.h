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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOPREFS_H
#define KOPREFS_H

#include <qdict.h>

#include <libkdepim/kpimprefs.h>

class KConfig;
class QFont;
class QColor;
class QStringList;

class KOPrefs : public KPimPrefs
{
  public:
    enum { FormatVCalendar, FormatICalendar };
    enum { MailClientKMail, MailClientSendmail };
    enum { IMIPDummy, IMIPKMail };
    enum { IMIPOutbox, IMIPdirectsend };
    enum { neverAuto, addressbookAuto, selectedAuto };
  
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
    bool    mAutoSave;
    int     mAutoSaveInterval;
    bool    mConfirm;
  
    bool    mEnableGroupScheduling;
    bool    mEnableProjectView;
  
    int     mDefaultFormat;
    int     mMailClient;

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
    QColor  mTodoDueTodayColor;
    QColor  mTodoOverdueColor;

    int     mDayBegins;
    int     mHourSize;
    bool    mDailyRecur;
    bool    mWeeklyRecur;
    bool    mEnableToolTips;
    bool    mEnableMonthScroll;
    bool    mFullViewMonth;
    bool    mMonthViewUsesCategoryColor;    
    bool    mFullViewTodo;
    bool    mMarcusBainsEnabled;
    int     mNextXDays;
   
    bool    mCompactDialogs;
    bool    mVerticalScreen;

    void setCategoryColor(QString cat,const QColor & color);
    QColor *categoryColor(QString cat);

    QString mArchiveFile;
    QString mHtmlExportFile;
    bool    mHtmlWithSave;

    QStringList mSelectedPlugins;

    int mIMIPScheduler;
    int mIMIPSend;
    QStringList mAdditionalMails;
    int mIMIPAutoRefresh;
    int mIMIPAutoInsertReply;
    int mIMIPAutoInsertRequest;
    int mIMIPAutoFreeBusy;
    int mIMIPAutoFreeBusyReply;

    QStringList mTodoTemplates;
    QStringList mEventTemplates;

  private:
    QDict<QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QFont mDefaultTimeBarFont;
    QFont mDefaultViewFont;
    QFont mDefaultMonthViewFont;

    QString mName;
    QString mEmail;
};

#endif
