// $Id$
// (C) 1998 by Preston Brown, Fester Zigterman, and Christopher Beard

#ifndef _KOOPTIONSDIALOG_H
#define _KOOPTIONSDIALOG_H

#include <qtabdlg.h>
#include <qframe.h>
#include <qlined.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qpushbt.h>

#include "kpropcolor.h"

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
  void setColorDefaults();
  void applyColorDefaults();
  void toggleSystemColors( bool );

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
  
  void selectTimeBarFont();
  void selectHolidayColor();
  void selectHighlightColor();

protected:
  /** These methods create the page contents */
  void setupMainTab();
  void setupTimeTab();
  void setupFontsTab();
  void setupColorsTab();
  void setupViewsTab();
  void setupDisplayTab();
  void setupPrinterTab();

  void setCombo(QComboBox *,const QString &);

  /** Optionsdlg about to close */
//  virtual void acceptConfig();
  
  enum {Main, Time, View, Printer};
  QLabel *labelList[4][10];
  QLineEdit *lineEditList[4][10];
  static char *labeltext[4][5];
  static char *confignames[4][5];

	
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

//  KPropColor *color1,*color2,*color3,*color4,*color5,*color6,*color7,*color8,*color9;

  // widgets holding preferences data
  QLineEdit *mNameEdit;
  QLineEdit *mEmailEdit;
  QLineEdit *mAdditionalEdit;
  QCheckBox *mAutoSaveCheck;
  QCheckBox *mConfirmCheck;
  QComboBox *mHolidayCombo;
  
  QButtonGroup *mTimeFormatGroup;
  QButtonGroup *mDateFormatGroup;
  QComboBox    *mTimeZoneCombo;
  QSpinBox     *mStartTimeSpin;
  QComboBox    *mAlarmTimeCombo;
  QCheckBox    *mWeekstartCheck;

  QLabel *mTimeBarFont;

  QFrame *mHolidayColor;
  QFrame *mHighlightColor;

  QSpinBox  *mDayBeginsSpin;
  QSlider   *mHourSizeSlider;
  QCheckBox *mDailyRecurCheck;

  QComboBox *mPrinterCombo;
  QButtonGroup *mPaperSizeGroup;
  QButtonGroup *mPaperOrientationGroup;
  QLineEdit *mPrintPreviewEdit;
};

/*
   The new KApplication doesn't have colors, so let's put this object here

 */
/*
struct koConfig {
    QColor windowColor;
    QColor textColor;
    QColor selectColor;
    QColor selectTextColor;
    QColor inactiveTextColor;
    QColor activeTextColor;
    QColor backgroundColor;
    QColor activeTitleColor;
    QColor inactiveTitleColor;
};

extern koConfig koconf;
*/
#endif
