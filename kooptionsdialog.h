// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KOOPTIONSDIALOG_H
#define _KOOPTIONSDIALOG_H

#include <qframe.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kdialogbase.h>

class QSpinBox;
class QSlider;


/** Dialog to change the korganizer configuration.
  */
class KOOptionsDialog : public KDialogBase
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KOOptionsDialog( QWidget *parent = 0, char *name = 0, bool modal=false );
    ~KOOptionsDialog();

  public slots:
    void showPrinterTab();
    
    /** Set all widgets to default values */
    void setDefaults();
  
    /** Read preferences from config file */
    void readConfig();

    /** Write preferences to config file */
    void writeConfig();

  signals:
    void configChanged();

  protected slots:
    /** Apply changes to preferences */
    void slotApply();  

    /** Accept changes to preferences and close dialog */
    void slotOk();
  
    /** Set preferences to default values */
    void slotDefault();
  
    void selectTimeBarFont();
    void selectHolidayColor();
    void selectHighlightColor();
    void selectCategoryColor();
  
    void updateCategoryColor();

  protected:
    void setupMainTab();
    void setupTimeTab();
    void setupFontsTab();
    void setupColorsTab();
    void setupViewsTab();
    void setupDisplayTab();
    void setupPrinterTab();

    void setCombo(QComboBox *combo,const QString & text, const QStringList *tags = 0);

  private:
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
    QCheckBox *mAutoSaveCheck;
    QCheckBox *mConfirmCheck;
    QComboBox *mHolidayCombo;
    QStringList mHolidayList;
  
    QComboBox    *mTimeZoneCombo;
    QSpinBox     *mStartTimeSpin;
    QComboBox    *mAlarmTimeCombo;

    QLabel *mTimeBarFont;

    QFrame *mHolidayColor;
    QFrame *mHighlightColor;

    QFrame        *mCategoryColor;
    QComboBox     *mCategoryCombo;
    QDict<QColor> mCategoryDict;

    QSpinBox  *mDayBeginsSpin;
    QSlider   *mHourSizeSlider;
    QCheckBox *mDailyRecurCheck;
    QCheckBox *mWeeklyRecurCheck;

    QComboBox    *mPrinterCombo;
    QButtonGroup *mPaperSizeGroup;
    QButtonGroup *mPaperOrientationGroup;
    QLineEdit    *mPrintPreviewEdit;
};

#endif
