/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qstrlist.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kemailsettings.h>

#if defined(USE_SOLARIS)
#include <sys/param.h>

#define ZONEINFODIR    "/usr/share/lib/zoneinfo"
#define INITFILE       "/etc/default/init"
#endif

#include "koprefs.h"

#include "koprefsdialog.h"
#include "koprefsdialog.moc"


KOPrefsDialog::KOPrefsDialog(QWidget *parent, char *name, bool modal) :
  KPrefsDialog(KOPrefs::instance(),parent,name,modal)
{
  mCategoryDict.setAutoDelete(true);

  KGlobal::locale()->insertCatalogue("timezones");

  setupMainTab();
  setupTimeTab();
  setupFontsTab();
  setupColorsTab();
  setupViewsTab();
  //setupPrinterTab();
  setupGroupSchedulingTab();
  setupGroupAutomationTab();
}


KOPrefsDialog::~KOPrefsDialog()
{
}


void KOPrefsDialog::setupMainTab()
{
  QFrame *topFrame = addPage(i18n("Personal"),0,
      DesktopIcon("identity",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());

  KPrefsWidBool *emailControlCenter =
      addWidBool(i18n("&Use email settings from Control Center"),
                 KOPrefs::instance()->mEmailControlCenter,topFrame);
  topLayout->addMultiCellWidget(emailControlCenter->checkBox(),0,0,0,1);
  connect(emailControlCenter->checkBox(),SIGNAL(toggled(bool)),
          SLOT(toggleEmailSettings(bool)));

  mNameEdit = new QLineEdit(topFrame);
  mNameLabel = new QLabel(mNameEdit, i18n("Full &name:"), topFrame);
  topLayout->addWidget(mNameLabel,1,0);
  topLayout->addWidget(mNameEdit,1,1);

  mEmailEdit = new QLineEdit(topFrame);
  mEmailLabel = new QLabel(mEmailEdit, i18n("E&mail address:"),topFrame);
  topLayout->addWidget(mEmailLabel,2,0);
  topLayout->addWidget(mEmailEdit,2,1);

  KPrefsWidBool *bcc =
      addWidBool(i18n("Send copy to owner when mailing events"),
                 KOPrefs::instance()->mBcc,topFrame);
  topLayout->addMultiCellWidget(bcc->checkBox(),4,4,0,1);


  QGroupBox *autoSaveGroup = new QGroupBox(1,Horizontal,i18n("Auto-Save"),
                                           topFrame);
  topLayout->addMultiCellWidget(autoSaveGroup,6,6,0,1);

  addWidBool(i18n("Enable automatic saving of calendar"),
             KOPrefs::instance()->mAutoSave,autoSaveGroup);

  QHBox *intervalBox = new QHBox(autoSaveGroup);
  intervalBox->setSpacing(spacingHint());

  QLabel *autoSaveIntervalLabel = new QLabel(i18n("Save &interval in minutes:"),intervalBox);
  mAutoSaveIntervalSpin = new QSpinBox(0,500,1,intervalBox);
  autoSaveIntervalLabel->setBuddy(mAutoSaveIntervalSpin);

  KPrefsWidBool *confirmCheck =
      addWidBool(i18n("Confirm &deletes"),KOPrefs::instance()->mConfirm,
                 topFrame);
  topLayout->addMultiCellWidget(confirmCheck->checkBox(),7,7,0,1);

  KPrefsWidRadios *defaultFormatGroup =
      addWidRadios(i18n("Default Calendar Format"),
                   KOPrefs::instance()->mDefaultFormat,topFrame);
  defaultFormatGroup->addRadio(i18n("vCalendar"));
  defaultFormatGroup->addRadio(i18n("iCalendar"));

  topLayout->addMultiCellWidget(defaultFormatGroup->groupBox(),10,10,0,1);

  // Default format unconditionally is iCalendar
  defaultFormatGroup->groupBox()->hide();

  KPrefsWidRadios *mailClientGroup =
      addWidRadios(i18n("Mail Client"),KOPrefs::instance()->mMailClient,
                   topFrame);
  mailClientGroup->addRadio(i18n("KMail"));
  mailClientGroup->addRadio(i18n("Sendmail"));
  topLayout->addMultiCellWidget(mailClientGroup->groupBox(),11,11,0,1);

  KPrefsWidBool *htmlsave =
      addWidBool(i18n("Export to HTML with every save"),KOPrefs::instance()->mHtmlWithSave,
                 topFrame);
  topLayout->addMultiCellWidget(htmlsave->checkBox(),12,12,0,1);

  KPrefsWidRadios *destinationGroup =
      addWidRadios(i18n("New Events/Todos should"),KOPrefs::instance()->mDestination,
                   topFrame);
  destinationGroup->addRadio(i18n("be added to the standard resource"));
  destinationGroup->addRadio(i18n("be asked which resource to use"));
  topLayout->addMultiCellWidget(destinationGroup->groupBox(),13,13,0,1);

  topLayout->setRowStretch(14,1);
}


void KOPrefsDialog::setupTimeTab()
{
  QFrame *topFrame = addPage(i18n("Time & Date"),0,
                             DesktopIcon("clock",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());

  QHBox *timeZoneBox = new QHBox( topFrame );
  topLayout->addMultiCellWidget( timeZoneBox, 0, 0, 0, 1 );

  new QLabel( i18n("Timezone:"), timeZoneBox );
  mTimeZoneCombo = new QComboBox( timeZoneBox );

  FILE *f;
  char tempstring[101] = "Unknown";
  QString sCurrentlySet(i18n("Unknown"));
  int nCurrentlySet = 0;
  QStringList list;

  // read the currently set time zone
#if defined(USE_SOLARIS)       // MARCO
    char buf[MAXPATHLEN];

    snprintf(buf, MAXPATHLEN,
            "/bin/fgrep 'TZ=' %s | /bin/head -n 1 | /bin/cut -b 4-",
            INITFILE);

    if (f = popen(buf, "r"))
      {
       if (fgets(buf, MAXPATHLEN - 1, f) != NULL)
         {
           buf[strlen(buf) - 1] = '\0';
           sCurrentlySet = QString(buf);
         }
       pclose(f);
      }
#else
  if((f = fopen("/etc/timezone", "r")) != NULL) {
    // get the currently set timezone
    fgets(tempstring, 100, f);
    tempstring[strlen(tempstring) - 1] = '\0';
    sCurrentlySet = QString(tempstring);
    fclose(f);
  }
#endif // !USE_SOLARIS

  mTimeZoneCombo->insertItem(i18n("[No selection]"));

  // Read all system time zones
#if defined(USE_SOLARIS)       // MARCO
    snprintf(buf, MAXPATHLEN,
           "/bin/find %s \\( -name src -prune \\) -o -type f -print | /bin/cut -b %d-",
           ZONEINFODIR, strlen(ZONEINFODIR) + 2);

    if (f = popen(buf, "r"))
      {
       while(fgets(buf, MAXPATHLEN - 1, f) != NULL)
         {
           buf[strlen(buf) - 1] = '\0';
           list.append(buf);
         }
       pclose(f);
      }

#else
  f = popen("grep -e  ^[^#] /usr/share/zoneinfo/zone.tab | cut -f 3","r");
  if (!f) return;
  while(fgets(tempstring, 100, f) != NULL) {
    tempstring[strlen(tempstring)-1] = '\0';
    list.append(i18n(tempstring));
    tzonenames << tempstring;
  }
  pclose(f);
#endif // !USE_SOLARIS
  list.sort();

  mTimeZoneCombo->insertStringList(list);

    // find the currently set time zone and select it
  for (int i = 0; i < mTimeZoneCombo->count(); i++)
    {
      if (mTimeZoneCombo->text(i) == sCurrentlySet)
        {
         nCurrentlySet = i;
         break;
        }
    }

  mTimeZoneCombo->setCurrentItem(nCurrentlySet);

  topLayout->addWidget(new QLabel(i18n("Default appointment time:"),
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
  topLayout->addWidget(new QLabel(i18n("Default alarm time:"),topFrame),
                       3,0);
  mAlarmTimeCombo = new QComboBox(topFrame);
  mAlarmTimeCombo->insertStringList(alarmList);
  topLayout->addWidget(mAlarmTimeCombo,3,1);


  QGroupBox *workingHoursGroup = new QGroupBox(1,Horizontal,
                                               i18n("Working Hours"),
                                               topFrame);
  topLayout->addMultiCellWidget(workingHoursGroup,4,4,0,1);

  QHBox *workStartBox = new QHBox(workingHoursGroup);

  addWidTime(i18n("Daily starting hour:"),
             KOPrefs::instance()->mWorkingHoursStart,workStartBox);

  QHBox *workEndBox = new QHBox(workingHoursGroup);

  addWidTime(i18n("Daily ending hour:"),
             KOPrefs::instance()->mWorkingHoursEnd,workEndBox);

  addWidBool(i18n("Exclude holidays"),
             KOPrefs::instance()->mExcludeHolidays,workingHoursGroup);

  addWidBool(i18n("Exclude Saturdays"),
             KOPrefs::instance()->mExcludeSaturdays,workingHoursGroup);

  KPrefsWidBool *marcusBainsShowSeconds = addWidBool(i18n("Show seconds on Marcus Bains line"),
	     KOPrefs::instance()->mMarcusBainsShowSeconds,
	     topFrame);
  topLayout->addWidget(marcusBainsShowSeconds->checkBox(),5,0);

  topLayout->setRowStretch(6,1);
}


void KOPrefsDialog::setupViewsTab()
{
  QFrame *topFrame = addPage(i18n("Views"),0,
                             DesktopIcon("viewmag",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,13,2);
  topLayout->setSpacing(spacingHint());

  QBoxLayout *dayBeginsLayout = new QHBoxLayout;
  topLayout->addLayout(dayBeginsLayout,0,0);

  KPrefsWidTime *dayBegins =
    addWidTime(i18n("Day begins at:"),KOPrefs::instance()->mDayBegins,
               topFrame);
  dayBeginsLayout->addWidget(dayBegins->label());
  dayBeginsLayout->addStretch(1);
  dayBeginsLayout->addWidget(dayBegins->spinBox());

  QBoxLayout *nextDaysLayout = new QHBoxLayout;
  topLayout->addLayout(nextDaysLayout,1,0);
  nextDaysLayout->addWidget(new QLabel(i18n("Days to show in Next-X-Days view:"),topFrame));
  mNextXDaysSpin = new QSpinBox(2,14,1,topFrame);
  nextDaysLayout->addStretch(1);
  nextDaysLayout->addWidget(mNextXDaysSpin);

  QGroupBox *hourSizeGroup = new QGroupBox(1,Horizontal,
                                           i18n("Hour Size in Schedule View"),
                                           topFrame);
  mHourSizeSlider = new QSlider(4,30,1,10,Horizontal,hourSizeGroup);
  topLayout->addMultiCellWidget(hourSizeGroup,2,2,0,1);

  KPrefsWidBool *dailyRecur =
    addWidBool(i18n("Show events that recur daily in date navigator"),
               KOPrefs::instance()->mDailyRecur,topFrame);
  topLayout->addWidget(dailyRecur->checkBox(),3,0);

  KPrefsWidBool *weeklyRecur =
    addWidBool(i18n("Show events that recur weekly in date navigator"),
               KOPrefs::instance()->mWeeklyRecur,topFrame);
  topLayout->addWidget(weeklyRecur->checkBox(),4,0);

  KPrefsWidBool *enableToolTips =
      addWidBool(i18n("Enable tooltips displaying summary of events"),
                 KOPrefs::instance()->mEnableToolTips,topFrame);
  topLayout->addWidget(enableToolTips->checkBox(),5,0);

  KPrefsWidBool *enableMonthScroll =
      addWidBool(i18n("Enable scrollbars in month view cells"),
                 KOPrefs::instance()->mEnableMonthScroll,topFrame);
  topLayout->addWidget(enableMonthScroll->checkBox(),6,0);

  KPrefsWidBool *fullViewMonth =
      addWidBool(i18n("Month view uses full window"),
                 KOPrefs::instance()->mFullViewMonth,topFrame);
  topLayout->addWidget(fullViewMonth->checkBox(),7,0);

  KPrefsWidBool *coloredCategoriesInMonthView =
      addWidBool(i18n("Month view uses category colors"),
                 KOPrefs::instance()->mMonthViewUsesCategoryColor,topFrame);
  topLayout->addWidget(coloredCategoriesInMonthView->checkBox(),8,0);

  KPrefsWidBool *fullViewTodo =
      addWidBool(i18n("To-do view uses full window"),
                 KOPrefs::instance()->mFullViewTodo,topFrame);
  topLayout->addWidget(fullViewTodo->checkBox(),9,0);

  KPrefsWidBool *marcusBainsEnabled =
      addWidBool(i18n("Show Marcus Bains line"),
                 KOPrefs::instance()->mMarcusBainsEnabled,topFrame);
  topLayout->addWidget(marcusBainsEnabled->checkBox(),10,0);

  topLayout->setRowStretch(11,1);
}


void KOPrefsDialog::setupFontsTab()
{
  QFrame *topFrame = addPage(i18n("Fonts"),0,
                             DesktopIcon("fonts",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,3);
  topLayout->setSpacing(spacingHint());

  KPrefsWidFont *timeBarFont =
      addWidFont(KGlobal::locale()->formatTime(QTime(12,34)),i18n("Time bar:"),
                 KOPrefs::instance()->mTimeBarFont,topFrame);
  topLayout->addWidget(timeBarFont->label(),0,0);
  topLayout->addWidget(timeBarFont->preview(),0,1);
  topLayout->addWidget(timeBarFont->button(),0,2);

  KPrefsWidFont *monthViewFont =
      addWidFont(KGlobal::locale()->formatTime(QTime(12,34)) + " " + i18n("Event text"),
                 i18n("Month view:"),KOPrefs::instance()->mMonthViewFont,topFrame);
  topLayout->addWidget(monthViewFont->label(),1,0);
  topLayout->addWidget(monthViewFont->preview(),1,1);
  topLayout->addWidget(monthViewFont->button(),1,2);

  KPrefsWidFont *agendaViewFont =
      addWidFont(i18n("Event text"),i18n("Agenda view:"),
                 KOPrefs::instance()->mAgendaViewFont,topFrame);
  topLayout->addWidget(agendaViewFont->label(),2,0);
  topLayout->addWidget(agendaViewFont->preview(),2,1);
  topLayout->addWidget(agendaViewFont->button(),2,2);

  KPrefsWidFont *marcusBainsFont =
      addWidFont(KGlobal::locale()->formatTime(QTime(12,34,23)),i18n("Marcus Bains line:"),
                 KOPrefs::instance()->mMarcusBainsFont,topFrame);
  topLayout->addWidget(marcusBainsFont->label(),3,0);
  topLayout->addWidget(marcusBainsFont->preview(),3,1);
  topLayout->addWidget(marcusBainsFont->button(),3,2);

  topLayout->setColStretch(1,1);
  topLayout->setRowStretch(4,1);
}

void KOPrefsDialog::setupColorsTab()
{
  QFrame *topFrame = addPage(i18n("Colors"),0,
                             DesktopIcon("colorize",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,7,2);
  topLayout->setSpacing(spacingHint());

  // Holiday Color
  KPrefsWidColor *holidayColor =
      addWidColor(i18n("Holiday color:"),
                  KOPrefs::instance()->mHolidayColor,topFrame);
  topLayout->addWidget(holidayColor->label(),0,0);
  topLayout->addWidget(holidayColor->button(),0,1);

  // Highlight Color
  KPrefsWidColor *highlightColor =
      addWidColor(i18n("Highlight color:"),
                  KOPrefs::instance()->mHighlightColor,topFrame);
  topLayout->addWidget(highlightColor->label(),1,0);
  topLayout->addWidget(highlightColor->button(),1,1);

  // Event color
  KPrefsWidColor *eventColor =
      addWidColor(i18n("Default event color:"),
                  KOPrefs::instance()->mEventColor,topFrame);
  topLayout->addWidget(eventColor->label(),2,0);
  topLayout->addWidget(eventColor->button(),2,1);

  // agenda view background color
  KPrefsWidColor *agendaBgColor =
      addWidColor(i18n("Agenda view background color:"),
                  KOPrefs::instance()->mAgendaBgColor,topFrame);
  topLayout->addWidget(agendaBgColor->label(),3,0);
  topLayout->addWidget(agendaBgColor->button(),3,1);

  // working hours color
  KPrefsWidColor *workingHoursColor =
      addWidColor(i18n("Working hours color:"),
                  KOPrefs::instance()->mWorkingHoursColor,topFrame);
  topLayout->addWidget(workingHoursColor->label(),4,0);
  topLayout->addWidget(workingHoursColor->button(),4,1);

  // Todo due today color
  KPrefsWidColor *todoDueTodayColor =
      addWidColor(i18n("Todo due today color:"),
                  KOPrefs::instance()->mTodoDueTodayColor,topFrame);
  topLayout->addWidget(todoDueTodayColor->label(),5,0);
  topLayout->addWidget(todoDueTodayColor->button(),5,1);

  // Todo overdue color
  KPrefsWidColor *todoOverdueColor =
      addWidColor(i18n("Todo overdue color:"),
                  KOPrefs::instance()->mTodoOverdueColor,topFrame);
  topLayout->addWidget(todoOverdueColor->label(),6,0);
  topLayout->addWidget(todoOverdueColor->button(),6,1);

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox(1,Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,7,7,0,1);

  mCategoryCombo = new QComboBox(categoryGroup);
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  connect(mCategoryCombo,SIGNAL(activated(int)),SLOT(updateCategoryColor()));

  mCategoryButton = new KColorButton(categoryGroup);
  connect(mCategoryButton,SIGNAL(changed(const QColor &)),SLOT(setCategoryColor()));
  updateCategoryColor();

  topLayout->setRowStretch(8,1);
}

void KOPrefsDialog::setCategoryColor()
{
  mCategoryDict.replace(mCategoryCombo->currentText(), new QColor(mCategoryButton->color()));
}

void KOPrefsDialog::updateCategoryColor()
{
  QString cat = mCategoryCombo->currentText();
  QColor *color = mCategoryDict.find(cat);
  if (!color) {
    color = KOPrefs::instance()->categoryColor(cat);
  }
  if (color) {
    mCategoryButton->setColor(*color);
  }
}

void KOPrefsDialog::setupPrinterTab()
{
  mPrinterTab = addPage(i18n("Printing"),0,
                        DesktopIcon("fileprint",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(mPrinterTab,5,2);
  topLayout->setSpacing(spacingHint());

  topLayout->setRowStretch(4,1);
}

void KOPrefsDialog::setupGroupSchedulingTab()
{
  QFrame *topFrame  = addPage(i18n("Group Scheduling"),0,
                              DesktopIcon("personal",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());

#if 0
  KPrefsWidRadios *schedulerGroup =
      addWidRadios(i18n("Scheduler Mail Client"),KOPrefs::instance()->mIMIPScheduler,
                   topFrame);
  schedulerGroup->addRadio("Dummy"); // Only for debugging
  schedulerGroup->addRadio(i18n("Mail client"));

  topLayout->addMultiCellWidget(schedulerGroup->groupBox(),0,0,0,1);
#endif

  KPrefsWidRadios *sendGroup =
      addWidRadios(i18n("Scheduler Mails Should Be"),KOPrefs::instance()->mIMIPSend,
                   topFrame);
  sendGroup->addRadio(i18n("Send to outbox"));
  sendGroup->addRadio(i18n("Send directly"));

  topLayout->addMultiCellWidget(sendGroup->groupBox(),1,1,0,1);

  topLayout->addMultiCellWidget(new QLabel(i18n("Additional email addresses:"),topFrame),2,2,0,1);
  mAMails = new QListView(topFrame);
  mAMails->addColumn(i18n("Email"),300);
  topLayout->addMultiCellWidget(mAMails,3,3,0,1);

  topLayout->addWidget(new QLabel(i18n("Additional email address:"),topFrame),4,0);
  aEmailsEdit = new QLineEdit(topFrame);
  aEmailsEdit->setEnabled(false);
  topLayout->addWidget(aEmailsEdit,4,1);

  QPushButton *add = new QPushButton(i18n("New"),topFrame,"new");
  topLayout->addWidget(add,5,0);
  QPushButton *del = new QPushButton(i18n("Remove"),topFrame,"remove");
  topLayout->addWidget(del,5,1);

  //topLayout->setRowStretch(2,1);
  connect(add, SIGNAL( clicked() ), this, SLOT(addItem()) );
  connect(del, SIGNAL( clicked() ), this, SLOT(removeItem()) );
  connect(aEmailsEdit,SIGNAL( textChanged(const QString&) ), this,SLOT(updateItem()));
  connect(mAMails,SIGNAL(selectionChanged(QListViewItem *)),SLOT(updateInput()));
}

void KOPrefsDialog::setupGroupAutomationTab()
{
  QFrame *topFrame  = addPage(i18n("Group Automation"),0,
                              DesktopIcon("personal",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,1);
  topLayout->setSpacing(spacingHint());

  KPrefsWidRadios *autoRefreshGroup =
      addWidRadios(i18n("Auto Send Refresh"),
                   KOPrefs::instance()->mIMIPAutoRefresh,topFrame);
  autoRefreshGroup->addRadio(i18n("Never"));
  autoRefreshGroup->addRadio(i18n("If attendee is in addressbook"));
  //autoRefreshGroup->addRadio(i18n("selected emails"));
  topLayout->addMultiCellWidget(autoRefreshGroup->groupBox(),0,0,0,0);

  KPrefsWidRadios *autoInsertGroup =
      addWidRadios(i18n("Auto Insert IMIP Replies"),
                   KOPrefs::instance()->mIMIPAutoInsertReply,topFrame);
  autoInsertGroup->addRadio(i18n("Never"));
  autoInsertGroup->addRadio(i18n("If attendee is in addressbook"));
  //autoInsertGroup->addRadio(i18n("selected emails"));
  topLayout->addMultiCellWidget(autoInsertGroup->groupBox(),1,1,0,0);

  KPrefsWidRadios *autoRequestGroup =
      addWidRadios(i18n("Auto Insert IMIP Requests"),
                   KOPrefs::instance()->mIMIPAutoInsertRequest,topFrame);
  autoRequestGroup->addRadio(i18n("Never"));
  autoRequestGroup->addRadio(i18n("If organizer is in addressbook"));
  //autoInsertGroup->addRadio(i18n("selected emails"));
  topLayout->addMultiCellWidget(autoRequestGroup->groupBox(),2,2,0,0);

  KPrefsWidRadios *autoFreeBusyGroup =
      addWidRadios(i18n("Auto Send FreeBusy Information"),
                   KOPrefs::instance()->mIMIPAutoFreeBusy,topFrame);
  autoFreeBusyGroup->addRadio(i18n("Never"));
  autoFreeBusyGroup->addRadio(i18n("If requested from an email in addressbook"));
  //autoFreeBusyGroup->addRadio(i18n("selected emails"));
  topLayout->addMultiCellWidget(autoFreeBusyGroup->groupBox(),3,3,0,0);

  KPrefsWidRadios *autoFreeBusyReplyGroup =
      addWidRadios(i18n("Auto Save FreeBusy Replies"),
                   KOPrefs::instance()->mIMIPAutoFreeBusyReply,topFrame);
  autoFreeBusyReplyGroup->addRadio(i18n("Never"));
  autoFreeBusyReplyGroup->addRadio(i18n("If attendee is in addressbook"));
  //autoFreeBusyGroup->addRadio(i18n("selected emails"));
  topLayout->addMultiCellWidget(autoFreeBusyReplyGroup->groupBox(),4,4,0,0);

  topLayout->setRowStretch( 5, 1 );
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
  mNameEdit->setText(KOPrefs::instance()->fullName());
  mEmailEdit->setText(KOPrefs::instance()->email());

  mAutoSaveIntervalSpin->setValue(KOPrefs::instance()->mAutoSaveInterval);

  setCombo(mTimeZoneCombo,i18n(KOPrefs::instance()->mTimeZoneId.utf8()));

  mStartTimeSpin->setValue(KOPrefs::instance()->mStartTime);
  mDefaultDurationSpin->setValue(KOPrefs::instance()->mDefaultDuration);
  mAlarmTimeCombo->setCurrentItem(KOPrefs::instance()->mAlarmTime);

  mHourSizeSlider->setValue(KOPrefs::instance()->mHourSize);

  mNextXDaysSpin->setValue(KOPrefs::instance()->mNextXDays);
  mAMails->clear();
  for ( QStringList::Iterator it = KOPrefs::instance()->mAdditionalMails.begin();
            it != KOPrefs::instance()->mAdditionalMails.end(); ++it ) {
    QListViewItem *item = new QListViewItem(mAMails);
    item->setText(0,*it);
    mAMails->insertItem(item);
  }
}


void KOPrefsDialog::usrWriteConfig()
{
  KOPrefs::instance()->setFullName(mNameEdit->text());
  KOPrefs::instance()->setEmail(mEmailEdit->text());

  KOPrefs::instance()->mAutoSaveInterval = mAutoSaveIntervalSpin->value();

  // Find untranslated selected zone
  QStringList::Iterator tz;
  for (tz = tzonenames.begin(); tz != tzonenames.end(); tz++)
    if (mTimeZoneCombo->currentText() == i18n((*tz).utf8()))
      break;
  if (tz != tzonenames.end())
    KOPrefs::instance()->mTimeZoneId = (*tz);
  else
    KOPrefs::instance()->mTimeZoneId = mTimeZoneCombo->currentText();

  KOPrefs::instance()->mStartTime = mStartTimeSpin->value();
  KOPrefs::instance()->mDefaultDuration = mDefaultDurationSpin->value();
  KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentItem();

  KOPrefs::instance()->mHourSize = mHourSizeSlider->value();

  QDictIterator<QColor> it(mCategoryDict);
  while (it.current()) {
    KOPrefs::instance()->setCategoryColor(it.currentKey(),*it.current());
    ++it;
  }

  KOPrefs::instance()->mNextXDays = mNextXDaysSpin->value();

  KOPrefs::instance()->mAdditionalMails.clear();
  QListViewItem *item;
  item = mAMails->firstChild();
  while (item)
  {
    KOPrefs::instance()->mAdditionalMails.append( item->text(0) );
    item = item->nextSibling();
  }

}

void KOPrefsDialog::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  updateCategoryColor();
}

void KOPrefsDialog::warningExperimental(bool on)
{
  if (on) {
    KMessageBox::information(this,i18n("This is an experimental feature. "
        "It may not work, it may do nothing useful and it may cause data loss. "
        "Use with care.\n"
        "You have to restart KOrganizer for this setting to take effect."));
  } else {
    KMessageBox::information(this,
        i18n("You have to restart KOrganizer for this setting to take effect."));
  }
}

void KOPrefsDialog::toggleEmailSettings(bool on)
{
  if (on) {
    mEmailEdit->setEnabled(false);
    mNameEdit->setEnabled(false);
    mEmailLabel->setEnabled(false);
    mNameLabel->setEnabled(false);

    KEMailSettings settings;
    mNameEdit->setText(settings.getSetting(KEMailSettings::RealName));
    mEmailEdit->setText(settings.getSetting(KEMailSettings::EmailAddress));
  } else {
    mEmailEdit->setEnabled(true);
    mNameEdit->setEnabled(true);
    mEmailLabel->setEnabled(true);
    mNameLabel->setEnabled(true);
  }
}

void KOPrefsDialog::addItem()
{
  aEmailsEdit->setEnabled(true);
  QListViewItem *item = new QListViewItem(mAMails);
  mAMails->insertItem(item);
  mAMails->setSelected(item,true);
  aEmailsEdit->setText(i18n("(EmptyEmail)"));
}

void KOPrefsDialog::removeItem()
{
  QListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  mAMails->takeItem(item);
  item = mAMails->selectedItem();
  if (!item) {
    aEmailsEdit->setText("");
    aEmailsEdit->setEnabled(false);
  }
  if (mAMails->childCount() == 0) {
    aEmailsEdit->setEnabled(false);
  }
}

void KOPrefsDialog::updateItem()
{
  QListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  item->setText(0,aEmailsEdit->text());
}

void KOPrefsDialog::updateInput()
{
  QListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  aEmailsEdit->setEnabled(true);
  aEmailsEdit->setText(item->text(0));
}
