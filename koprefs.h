// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFS_H
#define _KOPREFS_H

#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qdict.h>

class KConfig;

class KOPrefs
{
  public:
    ~KOPrefs();
  
    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();
  
    /** Set preferences to default values */
    void setDefaults();
  
    /** Read preferences from config file */
    void readConfig();

    /** Write preferences to config file */
    void writeConfig();

  protected:
    void setCategoryDefaults();

  private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();

    KConfig *mConfig;  // pointer to KConfig object

    static KOPrefs *mInstance;

  public:
    // preferences data
    QString mName;
    QString mEmail;
    QString mAdditional;
    QString mHoliday;
    bool    mAutoSave;
    bool    mConfirm;
  
    QString mTimeZone;
    int     mStartTime;
    int     mDefaultDuration;
    int     mAlarmTime;
    int     mDaylightSavings;

    QFont   mTimeBarFont;

    QColor  mHolidayColor;
    QColor  mHighlightColor;
    QColor  mEventColor;
    QColor  mAgendaBgColor;

    int     mDayBegins;
    int     mHourSize;
    bool    mDailyRecur;
    bool    mWeeklyRecur;
    bool    mEnableToolTips;

    QString mPrinter;
    int     mPaperSize;
    int     mPaperOrientation;
    QString mPrintPreview;

    QStringList mCustomCategories;

    void setCategoryColor(QString cat,const QColor & color);
    QColor *categoryColor(QString cat);
    
  private:
    QDict<QColor> mCategoryColors;
    QColor mDefaultCategoryColor;
    QColor mDefaultHolidayColor;
    QColor mDefaultHighlightColor;
    QColor mDefaultAgendaBgColor;

    QFont mDefaultTimeBarFont;
};

#endif
