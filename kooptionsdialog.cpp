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

#include "kooptionsdialog.h"
#include "kooptionsdialog.moc"

KOOptionsDialog::KOOptionsDialog(QWidget *parent, char *name, bool modal) :
  KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
              name,modal,true)
{
  mCategoryDict.setAutoDelete(true);

  setupMainTab();
  setupTimeTab();
  setupFontsTab();
  setupColorsTab();
  setupViewsTab();
  setupPrinterTab();

  readConfig();
  
  QObject::connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  QObject::connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}


KOOptionsDialog::~KOOptionsDialog()
{
}


void KOOptionsDialog::setupMainTab()
{
  KIconEffect *iconeffect=KGlobal::iconLoader()->iconEffect();
  QFrame *topFrame = addPage(i18n("Personal"),0,
	DesktopIcon("identity",KIcon::SizeMedium));

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


  topLayout->setRowStretch(7,1);
}


void KOOptionsDialog::setupTimeTab()
{
  QFrame *topFrame = addPage(i18n("Time & Date"),0,
                             DesktopIcon("clock",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  //lukas: TODO - replace by human readable ones
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
  
  topLayout->setRowStretch(4,1);
}


void KOOptionsDialog::setupViewsTab()
{
  QFrame *topFrame = addPage(i18n("Views"),0,
                             DesktopIcon("viewmag",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  topLayout->addWidget(new QLabel(i18n("Day begins at:"),
                       topFrame),0,0);
  mDayBeginsSpin = new QSpinBox(0,23,1,topFrame);
  mDayBeginsSpin->setSuffix(":00");
  topLayout->addWidget(mDayBeginsSpin,0,1);

  QGroupBox *hourSizeGroup = new QGroupBox(1,Horizontal,
                                           i18n("Hour size in schedule view"),
                                           topFrame);
  mHourSizeSlider = new QSlider(0,100,5,10,Horizontal,hourSizeGroup);
  topLayout->addMultiCellWidget(hourSizeGroup,1,1,0,1);
// Disable hour size setting because it is not used. Has to be fixed.
  hourSizeGroup->hide();
    
  mDailyRecurCheck =
      new QCheckBox(i18n("Show events that recur daily in Date Navigator"),
                    topFrame);
  topLayout->addWidget(mDailyRecurCheck,2,0);
  
  mWeeklyRecurCheck =
      new QCheckBox(i18n("Show events that recur weekly in Date Navigator"),
                    topFrame);
  topLayout->addWidget(mWeeklyRecurCheck,3,0);
  
  mEnableToolTipsCheck =
      new QCheckBox(i18n("Enable ToolTips displaying summary of events"),
                    topFrame);
  topLayout->addWidget(mEnableToolTipsCheck,4,0);
  
  topLayout->setRowStretch(5,1);
}


void KOOptionsDialog::setupFontsTab()
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

void KOOptionsDialog::selectTimeBarFont()
{
  QFont theFont(mTimeBarFont->font());
  QString theText(mTimeBarFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mTimeBarFont->setFont(theFont);
}

void KOOptionsDialog::selectMonthViewFont()
{
  QFont theFont(mMonthViewFont->font());
  QString theText(mMonthViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mMonthViewFont->setFont(theFont);
}

void KOOptionsDialog::selectAgendaViewFont()
{
  QFont theFont(mAgendaViewFont->font());
  QString theText(mAgendaViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mAgendaViewFont->setFont(theFont);
}


void KOOptionsDialog::setupColorsTab()
{
  QFrame *topFrame = addPage(i18n("Colors"),0,
                             DesktopIcon("colorize",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  // Holiday Color
  mHolidayColor = new QFrame(topFrame);
  mHolidayColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHolidayColor,0,0);

  QPushButton *buttonHolidayColor = new QPushButton(i18n("Holiday Color"),
                                                    topFrame);
  topLayout->addWidget(buttonHolidayColor,0,1);
  connect(buttonHolidayColor,SIGNAL(clicked()),SLOT(selectHolidayColor()));

  // Highlight Color
  mHighlightColor = new QFrame(topFrame);
  mHighlightColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHighlightColor,1,0);

  QPushButton *buttonHighlightColor = new QPushButton(i18n("Highlight Color"),
                                                      topFrame);
  topLayout->addWidget(buttonHighlightColor,1,1);
  connect(buttonHighlightColor,SIGNAL(clicked()),SLOT(selectHighlightColor()));

  // Event color
  mEventColor = new QFrame(topFrame);
  mEventColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mEventColor,2,0);

  QPushButton *buttonEventColor = new QPushButton(i18n("Default Event Color"),
                                                      topFrame);
  topLayout->addWidget(buttonEventColor,2,1);
  connect(buttonEventColor,SIGNAL(clicked()),SLOT(selectEventColor()));

  // agenda view background color
  mAgendaBgColor = new QFrame(topFrame);
  mAgendaBgColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mAgendaBgColor,3,0);

  QPushButton *buttonAgendaBgColor =
      new QPushButton(i18n("Agenda View Background Color"),topFrame);
  topLayout->addWidget(buttonAgendaBgColor,3,1);
  connect(buttonAgendaBgColor,SIGNAL(clicked()),SLOT(selectAgendaBgColor()));

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox(1,Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,4,4,0,1);

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
  
  topLayout->setRowStretch(5,1);
}

void KOOptionsDialog::selectHolidayColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHolidayColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectHighlightColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHighlightColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectEventColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mEventColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectAgendaBgColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mAgendaBgColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectCategoryColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mCategoryColor->setBackgroundColor(myColor);
    mCategoryDict.insert(mCategoryCombo->currentText(),new QColor(myColor));
  }
}

void KOOptionsDialog::updateCategoryColor()
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

void KOOptionsDialog::setupPrinterTab()
{
  mPrinterTab = addPage(i18n("Printing"),0,
                             DesktopIcon("fileprint",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(mPrinterTab,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  topLayout->addWidget(new QLabel(i18n("Printer Name:"),mPrinterTab),0,0);
  mPrinterCombo = new QComboBox(mPrinterTab);
  topLayout->addWidget(mPrinterCombo,0,1);

  //lukas: TODO - replace by KPrinter capabilities in the future
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


void KOOptionsDialog::showPrinterTab()
{
  showPage(pageIndex(mPrinterTab));
}


void KOOptionsDialog::setDefaults()
{
  KOPrefs::instance()->setDefaults();
  readConfig();
}


void KOOptionsDialog::setCombo(QComboBox *combo, const QString & text,
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

void KOOptionsDialog::readConfig()
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

  mDayBeginsSpin->setValue(KOPrefs::instance()->mDayBegins);
  mHourSizeSlider->setValue(KOPrefs::instance()->mHourSize);
  mDailyRecurCheck->setChecked(KOPrefs::instance()->mDailyRecur);
  mWeeklyRecurCheck->setChecked(KOPrefs::instance()->mWeeklyRecur);
  mEnableToolTipsCheck->setChecked(KOPrefs::instance()->mEnableToolTips);

  mTimeBarFont->setFont(KOPrefs::instance()->mTimeBarFont);
  mMonthViewFont->setFont(KOPrefs::instance()->mMonthViewFont);
  mAgendaViewFont->setFont(KOPrefs::instance()->mAgendaViewFont);

  mHolidayColor->setBackgroundColor(KOPrefs::instance()->mHolidayColor);
  mHighlightColor->setBackgroundColor(KOPrefs::instance()->mHighlightColor);
  mEventColor->setBackgroundColor(KOPrefs::instance()->mEventColor);
  mAgendaBgColor->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);

  setCombo(mPrinterCombo,KOPrefs::instance()->mPrinter);

  mPaperSizeGroup->setButton(KOPrefs::instance()->mPaperSize);
  mPaperOrientationGroup->setButton(KOPrefs::instance()->mPaperOrientation);
  mPrintPreviewEdit->setText(KOPrefs::instance()->mPrintPreview);
}


void KOOptionsDialog::writeConfig()
{
  kdDebug() << "KOOptionsDialog::writeConfig()" << endl;

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

  KOPrefs::instance()->mDayBegins = mDayBeginsSpin->value();
  KOPrefs::instance()->mHourSize = mHourSizeSlider->value();
  KOPrefs::instance()->mDailyRecur = mDailyRecurCheck->isChecked();
  KOPrefs::instance()->mWeeklyRecur = mWeeklyRecurCheck->isChecked();
  KOPrefs::instance()->mEnableToolTips = mEnableToolTipsCheck->isChecked();

  KOPrefs::instance()->mTimeBarFont = mTimeBarFont->font();
  KOPrefs::instance()->mMonthViewFont = mMonthViewFont->font();
  KOPrefs::instance()->mAgendaViewFont = mAgendaViewFont->font();

  KOPrefs::instance()->mHolidayColor = mHolidayColor->backgroundColor();
  KOPrefs::instance()->mHighlightColor = mHighlightColor->backgroundColor();
  KOPrefs::instance()->mEventColor = mEventColor->backgroundColor();
  KOPrefs::instance()->mAgendaBgColor = mAgendaBgColor->backgroundColor();

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
  
  KOPrefs::instance()->writeConfig();
}


void KOOptionsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void KOOptionsDialog::slotOk()
{
  slotApply();
  accept();
}

void KOOptionsDialog::slotDefault()
{
  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Continue"))
    == KMessageBox::Continue) setDefaults(); 
}


void KOOptionsDialog::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);  
  updateCategoryColor();
}
