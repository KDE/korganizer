// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcolordlg.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "koprefs.h"

#include "koprefsdialog.h"
#include "koprefsdialog.moc"


KOPrefsDialog::KOPrefsDialog(QWidget *parent, char *name, bool modal) :
  KPrefsDialog(KOPrefs::instance(),parent,name,modal)
{
  mCategoryDict.setAutoDelete(true);

  setupMainTab();
  setupTimeTab();
  setupFontsTab();
  setupColorsTab();
  setupViewsTab();
  setupPrinterTab();
}


KOPrefsDialog::~KOPrefsDialog()
{
}


void KOPrefsDialog::setupMainTab()
{
//  KIconEffect *iconeffect=KGlobal::iconLoader()->iconEffect();
  QFrame *topFrame = addPage(i18n("Personal"),0,
      DesktopIcon("identity",KIcon::SizeMedium));
//    iconeffect->apply(QPixmap(user_xpm),KIcon::Desktop,KIcon::DefaultState));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  
  topLayout->addWidget(new QLabel(i18n("Your name:"),topFrame),0,0);
  mNameEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mNameEdit,0,1);

  topLayout->addWidget(new QLabel(i18n("Email address:"),topFrame),1,0);
  mEmailEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mEmailEdit,1,1);
  
  topLayout->addWidget(new QLabel(i18n("Additional:"),topFrame),2,0);
  mAdditionalEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mAdditionalEdit,2,1);
  

  QGroupBox *autoSaveGroup = new QGroupBox(1,Horizontal,i18n("Auto-Save"),
                                           topFrame);
  topLayout->addMultiCellWidget(autoSaveGroup,3,3,0,1);

  mAutoSaveCheck = new QCheckBox(i18n("Enable automatic saving of calendar"),
                                 autoSaveGroup);

  QHBox *intervalBox = new QHBox(autoSaveGroup);
  intervalBox->setSpacing(spacingHint());

  (void)new QLabel(i18n("Save interval in minutes:"),intervalBox);
  mAutoSaveIntervalSpin = new QSpinBox(0,500,1,intervalBox);


  mHolidayList << QString::null;
  QStringList countryList = KGlobal::dirs()->findAllResources("data",
      "korganizer/holiday_*", false, true);
  for ( QStringList::Iterator it = countryList.begin();
        it != countryList.end();
        ++it )
    mHolidayList << (*it).mid((*it).findRev('_') + 1);

  topLayout->addWidget(new QLabel(i18n("Holidays:"),topFrame),4,0);
  mHolidayCombo = new QComboBox(topFrame);
  mHolidayCombo->insertStringList(mHolidayList);

  topLayout->addWidget(mHolidayCombo,4,1);


  mBccCheck = new QCheckBox(i18n("Send copy to owner when mailing events"),
                            topFrame);
  topLayout->addMultiCellWidget(mBccCheck,5,5,0,1);


  mConfirmCheck = new QCheckBox(i18n("Confirm Deletes"),topFrame);
  topLayout->addMultiCellWidget(mConfirmCheck,6,6,0,1);

  mEnableGroupScheduling =
      new KPrefsWidBool(i18n("Enable Group Scheduling"),
                        &(KOPrefs::instance()->mEnableGroupScheduling),this,
                        topFrame);
  topLayout->addWidget(mEnableGroupScheduling->checkBox(),7,0);
  connect(mEnableGroupScheduling->checkBox(),SIGNAL(clicked()),
          SLOT(warningGroupScheduling()));

  mEnableProjectView =
      new KPrefsWidBool(i18n("Enable Project View"),
                        &(KOPrefs::instance()->mEnableProjectView),this,
                        topFrame);
  topLayout->addWidget(mEnableProjectView->checkBox(),8,0);
  connect(mEnableProjectView->checkBox(),SIGNAL(clicked()),
          SLOT(warningProjectView()));

  // Disable settings for experimental features
  mEnableGroupScheduling->checkBox()->hide();
//  mEnableProjectView->checkBox()->hide();

  KPrefsWidRadios *defaultFormatGroup =
      new KPrefsWidRadios(i18n("Default Calendar Format"),
                          &(KOPrefs::instance()->mDefaultFormat),this,topFrame);
  defaultFormatGroup->addRadio(i18n("vCalendar"));
  defaultFormatGroup->addRadio(i18n("iCalendar"));

  topLayout->addMultiCellWidget(defaultFormatGroup->groupBox(),9,9,0,1);

  topLayout->setRowStretch(10,1);
}


void KOPrefsDialog::setupTimeTab()
{
  QFrame *topFrame = addPage(i18n("Time & Date"),0,
                             DesktopIcon("clock",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  const char *tzList[] = { "-1200", "-1130", "-1100", "-1030", "-1000",
                           "-0930", "-0900", "-0830", "-0800", "-0730",
                           "-0700", "-0630", "-0600", "-0530", "-0500",
                           "-0430", "-0400", "-0330", "-0300", "-0230",
                           "-0200", "-0130", "-0100", "-0030", "+0000",
                           "+0030", "+0100", "+0130", "+0200", "+0230",
                           "+0300", "+0330", "+0400", "+0430", "+0500",
                           "+0530", "+0600", "+0630", "+0700", "+0730",
                           "+0800", "+0830", "+0900", "+0930", "+1000",
                           "+1030", "+1100", "+1130", "+1200", "+1230",
                           "+1300", "+1330", "+1400", 0L };

  topLayout->addWidget(new QLabel(i18n("TimeZone:"),topFrame),0,0);
  mTimeZoneCombo = new QComboBox(topFrame);
  mTimeZoneCombo->insertStrList(tzList);
  topLayout->addWidget(mTimeZoneCombo,0,1);

  topLayout->addWidget(new QLabel(i18n("Default Appointment Time:"),
                       topFrame),1,0);
  mStartTimeSpin = new QSpinBox(0,23,1,topFrame);
  mStartTimeSpin->setSuffix(":00");
  topLayout->addWidget(mStartTimeSpin,1,1);

  topLayout->addWidget(new QLabel(i18n("Default duration of new appointment:"),
                       topFrame),2,0);
  mDefaultDurationSpin = new QSpinBox(0,23,1,topFrame);
  mDefaultDurationSpin->setSuffix(":00");
  topLayout->addWidget(mDefaultDurationSpin,2,1);

  QStringList alarmList;  
  alarmList << i18n("1 minute") << i18n("5 minutes") << i18n("10 minutes")
            << i18n("15 minutes") << i18n("30 minutes");
  topLayout->addWidget(new QLabel(i18n("Default Alarm Time:"),topFrame),
                       3,0);
  mAlarmTimeCombo = new QComboBox(topFrame);
  mAlarmTimeCombo->insertStringList(alarmList);
  topLayout->addWidget(mAlarmTimeCombo,3,1);

  QGroupBox *workingHoursGroup = new QGroupBox(1,Horizontal,
                                               i18n("Working Hours"),
                                               topFrame);
  topLayout->addMultiCellWidget(workingHoursGroup,4,4,0,1);

  QHBox *workStartBox = new QHBox(workingHoursGroup);
//  KPrefsWidTime *workingHoursStart =
    new KPrefsWidTime(i18n("Daily Starting Hour:"),
                      &(KOPrefs::instance()->mWorkingHoursStart),
                      this,workStartBox);
//  topLayout->addWidget(workingHoursStart->label(),6,0);
//  topLayout->addWidget(workingHoursStart->spinBox(),6,1);

  QHBox *workEndBox = new QHBox(workingHoursGroup);
//  KPrefsWidTime *workingHoursEnd =
    new KPrefsWidTime(i18n("Daily Ending Hour:"),
                      &(KOPrefs::instance()->mWorkingHoursEnd),
                      this,workEndBox);
//  topLayout->addWidget(workingHoursEnd->label(),7,0);
//  topLayout->addWidget(workingHoursEnd->spinBox(),7,1);

//  KPrefsWidBool *excludeHolidays =
      new KPrefsWidBool(i18n("Exclude Holidays"),
                        &(KOPrefs::instance()->mExcludeHolidays),this,
                        workingHoursGroup);
//  topLayout->addWidget(fullViewMonth->checkBox(),8,0);

//  KPrefsWidBool *excludeSaturdays =
      new KPrefsWidBool(i18n("Exclude Saturdays"),
                        &(KOPrefs::instance()->mExcludeSaturdays),this,
                        workingHoursGroup);
//  topLayout->addWidget(fullViewMonth->checkBox(),8,0);

  topLayout->setRowStretch(5,1);
}


void KOPrefsDialog::setupViewsTab()
{
  QFrame *topFrame = addPage(i18n("Views"),0,
                             DesktopIcon("viewmag",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  KPrefsWidTime *dayBegins =
    new KPrefsWidTime(i18n("Day begins at:"),&(KOPrefs::instance()->mDayBegins),
                      this,topFrame);                    
  topLayout->addWidget(dayBegins->label(),0,0);
  topLayout->addWidget(dayBegins->spinBox(),0,1);

// TODO: make hour size work
  QGroupBox *hourSizeGroup = new QGroupBox(1,Horizontal,
                                           i18n("Hour size in schedule view"),
                                           topFrame);
  mHourSizeSlider = new QSlider(0,100,5,10,Horizontal,hourSizeGroup);
  topLayout->addMultiCellWidget(hourSizeGroup,1,1,0,1);
// Disable hour size setting because it is not used. Has to be fixed.
  hourSizeGroup->hide();

  KPrefsWidBool *dailyRecur =
    new KPrefsWidBool(i18n("Show events that recur daily in Date Navigator"),
                      &(KOPrefs::instance()->mDailyRecur),this,topFrame);
  topLayout->addWidget(dailyRecur->checkBox(),2,0);

  KPrefsWidBool *weeklyRecur =
    new KPrefsWidBool(i18n("Show events that recur weekly in Date Navigator"),
                      &(KOPrefs::instance()->mWeeklyRecur),this,topFrame);
  topLayout->addWidget(weeklyRecur->checkBox(),3,0);

  mEnableToolTipsCheck =
      new QCheckBox(i18n("Enable ToolTips displaying summary of events"),
                    topFrame);
  topLayout->addWidget(mEnableToolTipsCheck,4,0);

  KPrefsWidBool *enableMonthScroll =
      new KPrefsWidBool(i18n("Enable Scrollbars in Month View cells"),
                        &(KOPrefs::instance()->mEnableMonthScroll),this,
                        topFrame);
  topLayout->addWidget(enableMonthScroll->checkBox(),5,0);
  
  KPrefsWidBool *fullViewMonth =
      new KPrefsWidBool(i18n("Month View uses full window"),
                        &(KOPrefs::instance()->mFullViewMonth),this,
                        topFrame);
  topLayout->addWidget(fullViewMonth->checkBox(),8,0);

  KPrefsWidBool *fullViewTodo =
      new KPrefsWidBool(i18n("Todo View uses full window"),
                        &(KOPrefs::instance()->mFullViewTodo),this,
                        topFrame);
  topLayout->addWidget(fullViewTodo->checkBox(),9,0);

  topLayout->setRowStretch(10,1);
}


void KOPrefsDialog::setupFontsTab()
{
  QFrame *topFrame = addPage(i18n("Fonts"),0,
                             DesktopIcon("fonts",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  mTimeBarFont = new QLabel(KGlobal::locale()->formatTime(QTime(12,34)),
                            topFrame);
  mTimeBarFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mTimeBarFont,0,0);

  QPushButton *buttonTimeBar = new QPushButton(i18n("Time Bar"),topFrame);
  topLayout->addWidget(buttonTimeBar,0,1);
  connect(buttonTimeBar,SIGNAL(clicked()),SLOT(selectTimeBarFont()));

  mMonthViewFont = new QLabel(KGlobal::locale()->formatTime(QTime(12,34)) +
                              " " + i18n("Event Text"),topFrame);
  mMonthViewFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mMonthViewFont,1,0);

  QPushButton *buttonMonthView = new QPushButton(i18n("Month View"),topFrame);
  topLayout->addWidget(buttonMonthView,1,1);
  connect(buttonMonthView,SIGNAL(clicked()),SLOT(selectMonthViewFont()));

  mAgendaViewFont = new QLabel(i18n("Event Text"),topFrame);
  mAgendaViewFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mAgendaViewFont,2,0);

  QPushButton *buttonAgendaView = new QPushButton(i18n("Agenda View"),topFrame);
  topLayout->addWidget(buttonAgendaView,2,1);
  connect(buttonAgendaView,SIGNAL(clicked()),SLOT(selectAgendaViewFont()));

  topLayout->setRowStretch(3,1);
}

void KOPrefsDialog::selectTimeBarFont()
{
  QFont theFont(mTimeBarFont->font());
  QString theText(mTimeBarFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mTimeBarFont->setFont(theFont);
}

void KOPrefsDialog::selectMonthViewFont()
{
  QFont theFont(mMonthViewFont->font());
  QString theText(mMonthViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mMonthViewFont->setFont(theFont);
}

void KOPrefsDialog::selectAgendaViewFont()
{
  QFont theFont(mAgendaViewFont->font());
  QString theText(mAgendaViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mAgendaViewFont->setFont(theFont);
}


void KOPrefsDialog::setupColorsTab()
{
  QFrame *topFrame = addPage(i18n("Colors"),0,
                             DesktopIcon("colorize",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  // Holiday Color
  KPrefsWidColor *holidayColor =
      new KPrefsWidColor(i18n("Holiday Color"),
                         &(KOPrefs::instance()->mHolidayColor),this,topFrame);
  topLayout->addWidget(holidayColor->preview(),0,0);
  topLayout->addWidget(holidayColor->button(),0,1);

  // Highlight Color
  KPrefsWidColor *highlightColor =
      new KPrefsWidColor(i18n("Highlight Color"),
                         &(KOPrefs::instance()->mHighlightColor),this,topFrame);
  topLayout->addWidget(highlightColor->preview(),1,0);
  topLayout->addWidget(highlightColor->button(),1,1);

  // Event color
  KPrefsWidColor *eventColor =
      new KPrefsWidColor(i18n("Default Event Color"),
                         &(KOPrefs::instance()->mEventColor),this,topFrame);
  topLayout->addWidget(eventColor->preview(),2,0);
  topLayout->addWidget(eventColor->button(),2,1);

  // agenda view background color
  KPrefsWidColor *agendaBgColor =
      new KPrefsWidColor(i18n("Agenda View Background Color"),
                         &(KOPrefs::instance()->mAgendaBgColor),this,topFrame);
  topLayout->addWidget(agendaBgColor->preview(),3,0);
  topLayout->addWidget(agendaBgColor->button(),3,1);

  // working hours color
  KPrefsWidColor *workingHoursColor = 
      new KPrefsWidColor(i18n("Working Hours Color"),
                         &(KOPrefs::instance()->mWorkingHoursColor),this,
                         topFrame);
  topLayout->addWidget(workingHoursColor->preview(),4,0);
  topLayout->addWidget(workingHoursColor->button(),4,1);

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox(1,Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,5,5,0,1);

  mCategoryCombo = new QComboBox(categoryGroup);
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  connect(mCategoryCombo,SIGNAL(activated(int)),SLOT(updateCategoryColor()));

  QHBox *categoryBox = new QHBox(categoryGroup);
  categoryBox->setSpacing(spacingHint());

  mCategoryColor = new QFrame(categoryBox);
  mCategoryColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  
  QPushButton *categoryButton = new QPushButton(i18n("Select Color"),
                                                categoryBox);
  connect(categoryButton,SIGNAL(clicked()),SLOT(selectCategoryColor()));
  updateCategoryColor();
  
  topLayout->setRowStretch(6,1);
}

void KOPrefsDialog::selectCategoryColor()
{
  QColor myColor(mCategoryColor->backgroundColor());
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mCategoryColor->setBackgroundColor(myColor);
    mCategoryDict.insert(mCategoryCombo->currentText(),new QColor(myColor));
  }
}

void KOPrefsDialog::updateCategoryColor()
{
  QString cat = mCategoryCombo->currentText();
  QColor *color = mCategoryDict.find(cat);
  if (!color) {
    color = KOPrefs::instance()->categoryColor(cat);
  }
  if (color) {
    mCategoryColor->setBackgroundColor(*color);
  }
}

void KOPrefsDialog::setupPrinterTab()
{
  mPrinterTab = addPage(i18n("Printing"),0,
                             DesktopIcon("fileprint",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(mPrinterTab,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  topLayout->addWidget(new QLabel(i18n("Printer Name:"),mPrinterTab),0,0);
  mPrinterCombo = new QComboBox(mPrinterTab);
  topLayout->addWidget(mPrinterCombo,0,1);

  QString prName;
  QFile printcap("/etc/printcap");

  if ( printcap.open(IO_ReadOnly) ) {
    QTextStream t (&printcap);
    while ( !t.eof() ){
      QString whole_line = t.readLine();
      whole_line = whole_line.stripWhiteSpace();
      if (whole_line.left(1) != "#" &&
	  whole_line.left(1) != ":" &&
	  whole_line != "") { // ignore certain lines
	QString pname(whole_line.mid(whole_line.find("|")+1,
				     whole_line.length()));
	if (pname.find("|") != -1) { // If there is no 2nd bar.XCXC
	  // Try to account for poorly formatted lines
	  prName = pname.left(pname.find("|"));
	} else {
	  prName = pname.left(pname.find(":"));
	}
	mPrinterCombo->insertItem(prName);
      }
    }
    printcap.close();
  }

  mPaperSizeGroup = new QButtonGroup(1,Horizontal,i18n("Paper Size:"),
                                     mPrinterTab);
  (void)new QRadioButton(i18n("A4"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("B5"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Letter"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Legal"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Executive"),mPaperSizeGroup);
  topLayout->addMultiCellWidget(mPaperSizeGroup,1,1,0,1);

  mPaperOrientationGroup = new QButtonGroup(1,Horizontal,
                                            i18n("Paper Orientation:"),
                                            mPrinterTab);
  (void)new QRadioButton(i18n("Portrait"),mPaperOrientationGroup);
  (void)new QRadioButton(i18n("Landscape"),mPaperOrientationGroup);
  topLayout->addMultiCellWidget(mPaperOrientationGroup,2,2,0,1);

  topLayout->addWidget(new QLabel(i18n("Preview Program:"),mPrinterTab),3,0);
  mPrintPreviewEdit = new QLineEdit(mPrinterTab);
  topLayout->addWidget(mPrintPreviewEdit,3,1);

  // Add some pixels spacing to avoid scrollbars in icon field. Not safe, but
  // works for me :-)
  topLayout->addRowSpacing(4,27);

  topLayout->setRowStretch(4,1);
}


void KOPrefsDialog::showPrinterTab()
{
  showPage(pageIndex(mPrinterTab));
}


void KOPrefsDialog::setCombo(QComboBox *combo, const QString & text,
                               const QStringList *tags)
{
  if (tags) {
    int i = tags->findIndex(text);
    if (i > 0) combo->setCurrentItem(i);
  } else {
    for(int i=0;i<combo->count();++i) {
      if (combo->text(i) == text) {
        combo->setCurrentItem(i);
        break;
      }
    }
  }
}

void KOPrefsDialog::usrReadConfig()
{
  mNameEdit->setText(KOPrefs::instance()->mName);
  mEmailEdit->setText(KOPrefs::instance()->mEmail);
  mAdditionalEdit->setText(KOPrefs::instance()->mAdditional);
  mBccCheck->setChecked(KOPrefs::instance()->mBcc);

  mAutoSaveCheck->setChecked(KOPrefs::instance()->mAutoSave);
  mAutoSaveIntervalSpin->setValue(KOPrefs::instance()->mAutoSaveInterval);
  mConfirmCheck->setChecked(KOPrefs::instance()->mConfirm);

  setCombo(mHolidayCombo,KOPrefs::instance()->mHoliday, &mHolidayList);
  
  setCombo(mTimeZoneCombo,KOPrefs::instance()->mTimeZone);

  mStartTimeSpin->setValue(KOPrefs::instance()->mStartTime);
  mDefaultDurationSpin->setValue(KOPrefs::instance()->mDefaultDuration);
  mAlarmTimeCombo->setCurrentItem(KOPrefs::instance()->mAlarmTime);

//  mDayBeginsSpin->setValue(KOPrefs::instance()->mDayBegins);
  mHourSizeSlider->setValue(KOPrefs::instance()->mHourSize);
//  mDailyRecurCheck->setChecked(KOPrefs::instance()->mDailyRecur);
//  mWeeklyRecurCheck->setChecked(KOPrefs::instance()->mWeeklyRecur);
  mEnableToolTipsCheck->setChecked(KOPrefs::instance()->mEnableToolTips);

  mTimeBarFont->setFont(KOPrefs::instance()->mTimeBarFont);
  mMonthViewFont->setFont(KOPrefs::instance()->mMonthViewFont);
  mAgendaViewFont->setFont(KOPrefs::instance()->mAgendaViewFont);

//  mHolidayColor->setBackgroundColor(KOPrefs::instance()->mHolidayColor);
//  mHighlightColor->setBackgroundColor(KOPrefs::instance()->mHighlightColor);
//  mEventColor->setBackgroundColor(KOPrefs::instance()->mEventColor);
//  mAgendaBgColor->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);

  setCombo(mPrinterCombo,KOPrefs::instance()->mPrinter);

  mPaperSizeGroup->setButton(KOPrefs::instance()->mPaperSize);
  mPaperOrientationGroup->setButton(KOPrefs::instance()->mPaperOrientation);
  mPrintPreviewEdit->setText(KOPrefs::instance()->mPrintPreview);
}


void KOPrefsDialog::usrWriteConfig()
{
  kdDebug() << "KOPrefsDialog::usrWriteConfig()" << endl;

  KOPrefs::instance()->mBcc = mBccCheck->isChecked();
  KOPrefs::instance()->mAutoSave = mAutoSaveCheck->isChecked();
  KOPrefs::instance()->mAutoSaveInterval = mAutoSaveIntervalSpin->value();
  KOPrefs::instance()->mConfirm = mConfirmCheck->isChecked();

  KOPrefs::instance()->mName = mNameEdit->text();
  KOPrefs::instance()->mEmail = mEmailEdit->text();
  KOPrefs::instance()->mAdditional = mAdditionalEdit->text();
  KOPrefs::instance()->mHoliday = *mHolidayList.at(mHolidayCombo->currentItem());
  kdDebug() << "Holiday: " << KOPrefs::instance()->mHoliday << endl;

  KOPrefs::instance()->mTimeZone = mTimeZoneCombo->currentText();
  KOPrefs::instance()->mStartTime = mStartTimeSpin->value();
  KOPrefs::instance()->mDefaultDuration = mDefaultDurationSpin->value();
  KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentItem();

//  KOPrefs::instance()->mDayBegins = mDayBeginsSpin->value();
  KOPrefs::instance()->mHourSize = mHourSizeSlider->value();
//  KOPrefs::instance()->mDailyRecur = mDailyRecurCheck->isChecked();
//  KOPrefs::instance()->mWeeklyRecur = mWeeklyRecurCheck->isChecked();
  KOPrefs::instance()->mEnableToolTips = mEnableToolTipsCheck->isChecked();

  KOPrefs::instance()->mTimeBarFont = mTimeBarFont->font();
  KOPrefs::instance()->mMonthViewFont = mMonthViewFont->font();
  KOPrefs::instance()->mAgendaViewFont = mAgendaViewFont->font();

//  KOPrefs::instance()->mHolidayColor = mHolidayColor->backgroundColor();
//  KOPrefs::instance()->mHighlightColor = mHighlightColor->backgroundColor();
//  KOPrefs::instance()->mEventColor = mEventColor->backgroundColor();
//  KOPrefs::instance()->mAgendaBgColor = mAgendaBgColor->backgroundColor();

  QDictIterator<QColor> it(mCategoryDict);
  while (it.current()) {
    KOPrefs::instance()->setCategoryColor(it.currentKey(),*it.current());
    ++it;
  }
  mCategoryDict.clear();

  KOPrefs::instance()->mPrinter = mPrinterCombo->currentText();
  KOPrefs::instance()->mPaperSize = 
      mPaperSizeGroup->id(mPaperSizeGroup->selected());
  KOPrefs::instance()->mPaperOrientation =
      mPaperOrientationGroup->id(mPaperOrientationGroup->selected());
  KOPrefs::instance()->mPrintPreview = mPrintPreviewEdit->text();
}

void KOPrefsDialog::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);  
  updateCategoryColor();
}

void KOPrefsDialog::warningGroupScheduling()
{
  warningExperimental(mEnableGroupScheduling->checkBox()->isChecked());
}

void KOPrefsDialog::warningProjectView()
{
  warningExperimental(mEnableProjectView->checkBox()->isChecked());
}

void KOPrefsDialog::warningExperimental(bool on)
{
  if (on) {
    KMessageBox::information(this,"This is an experimental feature. "
        "It may not work, it may do nothing useful and it may cause data loss. "
        "Use with care.\n"
        "You have to restart KOrganizer for this setting to take effect.");
  } else {
    KMessageBox::information(this,
        "You have to restart KOrganizer for this setting to take effect.");
  }
}
