// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOPREFSDIALOG_H
#define _KOPREFSDIALOG_H

#include <qframe.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kdialogbase.h>

#include "kprefsdialog.h"

class QSpinBox;
class QSlider;


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
    void selectTimeBarFont();
    void selectMonthViewFont();
    void selectAgendaViewFont();

    void selectCategoryColor();
    void updateCategoryColor();

    void warningExperimental(bool on);
    void warningGroupScheduling();

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

    void setCombo(QComboBox *combo,const QString & text, const QStringList *tags = 0);

  private:
    KPrefsWidBool *mEnableGroupScheduling;

    QFrame *mPrinterTab;
  
    QFrame *personalFrame;
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
  
    QFrame *timeFrame;
    QComboBox *timeCombo;
    QComboBox *tzCombo;
  
    QFrame *fontsFrame;
    QFrame *colorsFrame;
    QFrame *viewsFrame;
    QFrame *displayFrame;
    QFrame *printerFrame;

    // font frame
    QLabel *listFontLabel;
    QLabel *agendaFontLabel;
    QLabel *monthFontLabel;

    // widgets holding preferences data
    QLineEdit *mNameEdit;
    QLineEdit *mEmailEdit;
    QLineEdit *mAdditionalEdit;
    QCheckBox *mBccCheck;
    QCheckBox *mAutoSaveCheck;
    QSpinBox  *mAutoSaveIntervalSpin;
    QCheckBox *mConfirmCheck;
    QComboBox *mHolidayCombo;
    QStringList mHolidayList;
  
    QComboBox    *mTimeZoneCombo;
    QSpinBox     *mStartTimeSpin;
    QSpinBox     *mDefaultDurationSpin;
    QComboBox    *mAlarmTimeCombo;

    QLabel *mTimeBarFont;
    QLabel *mMonthViewFont;
    QLabel *mAgendaViewFont;

    QFrame        *mCategoryColor;
    QComboBox     *mCategoryCombo;
    QDict<QColor> mCategoryDict;

    QSlider   *mHourSizeSlider;
    QCheckBox *mEnableToolTipsCheck;

    QComboBox    *mPrinterCombo;
    QButtonGroup *mPaperSizeGroup;
    QButtonGroup *mPaperOrientationGroup;
    QLineEdit    *mPrintPreviewEdit;
};

#endif
