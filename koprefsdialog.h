// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFSDIALOG_H
#define _KOPREFSDIALOG_H

#include <qframe.h>
#include <kdialogbase.h>

#include "kprefsdialog.h"

class QSpinBox;
class QSlider;
class KURLRequester;
class QComboBox;
class QLineEdit;

/** Dialog to change the korganizer configuration.
  */
class KOPrefsDialog : public KPrefsDialog
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KOPrefsDialog(QWidget *parent=0,char *name=0,bool modal=false);
    ~KOPrefsDialog();

  public slots:
    void showPrinterTab();
    
    /** Update controls for categories */
    void updateCategories();

  protected slots:
    void selectCategoryColor();
    void updateCategoryColor();

    void warningExperimental(bool on);
    void warningGroupScheduling();
    void warningProjectView();

    void toggleEmailSettings(bool);

  protected:
    void usrReadConfig();
    void usrWriteConfig();

    void setupMainTab();
    void setupTimeTab();
    void setupFontsTab();
    void setupColorsTab();
    void setupViewsTab();
    void setupDisplayTab();
    void setupPrinterTab();
    void setupGroupSchedulingTab();

    void setCombo(QComboBox *combo,const QString & text, const QStringList *tags = 0);

  private:
    KPrefsWidBool *mEnableGroupScheduling;
    KPrefsWidBool *mEnableProjectView;

    QFrame *mPrinterTab;
  
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
  
    QComboBox *timeCombo;
    QComboBox *tzCombo;
  
    // widgets holding preferences data
    QLineEdit *mNameEdit;
    QLineEdit *mEmailEdit;
    QLineEdit *mAdditionalEdit;
    QSpinBox  *mAutoSaveIntervalSpin;
    QComboBox *mHolidayCombo;
    QStringList mHolidayList;
  
    QComboBox    *mTimeZoneCombo;
    QSpinBox     *mStartTimeSpin;
    QSpinBox     *mDefaultDurationSpin;
    QComboBox    *mAlarmTimeCombo;

    QFrame        *mCategoryColor;
    QComboBox     *mCategoryCombo;
    QDict<QColor> mCategoryDict;

    QSlider   *mHourSizeSlider;

    KURLRequester *mPrintPreviewEdit;
};

#endif
