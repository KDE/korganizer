// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFS_H
#define _KOPREFS_H


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
    int     mStartTime;
    int     mDefaultDuration;
    int     mAlarmTime;
    int     mDaylightSavings;

    int     mWorkingHoursStart;
    int     mWorkingHoursEnd;
    bool    mExcludeHolidays;
    bool    mExcludeSaturdays;

    QFont   mTimeBarFont;
    QFont   mMonthViewFont;
    QFont   mAgendaViewFont;

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

    QString mPrinter;
    int     mPaperSize;
    int     mPaperOrientation;
    QString mPrintPreview;

    QStringList mCustomCategories;

    void setCategoryColor(QString cat,const QColor & color);
    QColor *categoryColor(QString cat);

    QString mArchiveFile;
    QString mHtmlExportFile;

  private:
    QDict<QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QFont mDefaultTimeBarFont;
    QFont mDefaultViewFont;

    QString mName;
    QString mEmail;
};

#endif
