
// $Id$
// (C) 1998 by Preston Brown, Fester Zigterman, and Christopher Beard

#ifndef __OPTION_H__
#define __OPTION_H__

#include <qtabdlg.h>
#include <qframe.h>
#include <qlined.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <kconfig.h>
#include <qradiobt.h>
#include <qpushbt.h>

#include "kpropdlg.h"
#include "kpropcolor.h"

/** Dialog to change the korganizer configuration.
  * @inherits KPropDlg
  */
class OptionsDialog : public KPropDlg
{
  Q_OBJECT
public:
  /** Initialize dialog and pages */
  OptionsDialog( const char *title, 
		QWidget *parent = 0, char *name = 0, bool modal=FALSE );
  ~OptionsDialog();

public slots:
  void showPrinterTab() { showPage(printerFrame); };
  void setColorDefaults();
  void applyColorDefaults();
  void toggleSystemColors( bool );
signals:
  void closed(QWidget *);

protected slots:
  /** read the config from the KConfig object */
//  void getConfig();
  /** Write the config to the KConfig object */
//  void setConfig();
//  void setFontLabel(int);
//  void setFontLabel1() { setFontLabel(1); };
//  void setFontLabel2() { setFontLabel(2); };
//  void setFontLabel3() { setFontLabel(3); };

protected:
  /** These methods create the page contents */
  void setupMainTab();
  void setupTimeTab();
  void setupFontsTab();
  void setupColorsTab();
  void setupViewsTab();
  void setupDisplayTab();
  void setupPrinterTab();
  
  /** Optionsdlg about to close */
//  virtual void acceptConfig();
  
  enum {Main, Time, View, Printer};
  QLabel *labelList[4][10];
  QLineEdit *lineEditList[4][10];
  static char *labeltext[4][5];
  static char *confignames[4][5];

	
private:
  
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

  KPropColor *color1,*color2,*color3,*color4,*color5,*color6,*color7,*color8,*color9;
  /** pointer to kapp->getConfig(); */
  KConfig *config; 

};

/*
   The new KApplication doesn't have colors, so let's put this object here

 */
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

#endif
