// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFS_H
#define _KOPREFS_H

#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qdict.h>

#include "kprefs.h"

class KConfig;

class KOPrefs : public KPrefs
{
  public:
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
    QString mName;
    QString mEmail;
    QString mAdditional;
    bool    mBcc;
    QString mHoliday;
    bool    mAutoSave;
    int     mAutoSaveInterval;
    bool    mConfirm;
  
    QString mTimeZone;
    int     mStartTime;
    int     mDefaultDuration;
    int     mAlarmTime;
    int     mDaylightSavings;

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
    int     mWorkingHoursStart;
    int     mWorkingHoursEnd;
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
};

#endif
