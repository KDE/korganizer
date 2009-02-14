/*
  This file is part of KOrganizer.

  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koprefsdialog.h"
#include "koprefs.h"
#include "ktimeedit.h"
#include "koglobals.h"
#include "stdcalendar.h"
#include "calendar/calendardecoration.h"
#include "korganizer/printplugin.h"

#include <kholidays/holidays.h>
using namespace KHolidays;

#include <kcal/calendarresources.h>

#include <kvbox.h>
#include <kcolorbutton.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kemailsettings.h>
#include <kcalendarsystem.h>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kocore.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <KLineEdit>
#include <KComboBox>
#include <KTabWidget>

#include <q3listview.h>
#include <q3buttongroup.h>
#include <QLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QFile>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QTextStream>
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimeEdit>
#include <QTreeWidget>

#include "ui_kogroupwareprefspage.h"

KOPrefsDialogMain::KOPrefsDialogMain( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );
  KTabWidget *tabWidget = new KTabWidget( this );
  topTopLayout->addWidget( tabWidget );

  // Personal Settings
  QWidget *personalFrame = new QWidget( this );
  QVBoxLayout *personalLayout = new QVBoxLayout( personalFrame );
  tabWidget->addTab( personalFrame, KIcon( "preferences-desktop-personal" ),
                     i18nc( "@title:tab personal settings", "Personal" ) );

  KPrefsWidBool *emailControlCenter =
    addWidBool( KOPrefs::instance()->emailControlCenterItem(), personalFrame );
  connect( emailControlCenter->checkBox(), SIGNAL(toggled(bool)), SLOT(toggleEmailSettings(bool)) );
  personalLayout->addWidget( emailControlCenter->checkBox() );

  mUserEmailSettings =
    new QGroupBox( i18nc( "@title:group email settings", "Email Settings" ), personalFrame );

  personalLayout->addWidget( mUserEmailSettings );
  QFormLayout *emailSettingsLayout = new QFormLayout( mUserEmailSettings );
  KPrefsWidString *s = addWidString( KOPrefs::instance()->userNameItem(), mUserEmailSettings );
  emailSettingsLayout->addRow ( s->label(), s->lineEdit() );

  s=addWidString( KOPrefs::instance()->userEmailItem(), mUserEmailSettings );
  emailSettingsLayout->addRow ( s->label(), s->lineEdit() );

  KPrefsWidRadios *defaultEmailAttachMethod =
    addWidRadios( KOPrefs::instance()->defaultEmailAttachMethodItem(), personalFrame );
  personalLayout->addWidget( defaultEmailAttachMethod->groupBox() );
  personalLayout->addStretch( 1 );

  // Save Settings
  QFrame *saveFrame = new QFrame( this );
  tabWidget->addTab( saveFrame, KIcon( "document-save" ),
                     i18nc( "@title:tab", "Save" ) );
  QVBoxLayout *saveLayout = new QVBoxLayout( saveFrame );

  QGroupBox *saveGroupBox =
    new QGroupBox( i18nc( "@title:group", "Saving Calendar" ), saveFrame );
  saveLayout->addWidget( saveGroupBox );
  QVBoxLayout *saveGroupLayout = new QVBoxLayout;
  saveGroupBox->setLayout( saveGroupLayout );

  KPrefsWidBool *htmlWithSave =
    addWidBool( KOPrefs::instance()->htmlWithSaveItem(), saveGroupBox );
  saveGroupLayout->addWidget( htmlWithSave->checkBox() );

  KPrefsWidBool *autoSave =
    addWidBool( KOPrefs::instance()->autoSaveItem(), saveGroupBox );
  saveGroupLayout->addWidget( autoSave->checkBox() );

  QBoxLayout *intervalLayout = new QHBoxLayout;
  saveGroupLayout->addLayout( intervalLayout );

  KPrefsWidInt *autoSaveInterval =
    addWidInt( KOPrefs::instance()->autoSaveIntervalItem(), saveGroupBox );
  connect( autoSave->checkBox(), SIGNAL(toggled(bool)),
           autoSaveInterval->label(), SLOT(setEnabled(bool)) );
  connect( autoSave->checkBox(), SIGNAL(toggled(bool)),
           autoSaveInterval->spinBox(), SLOT(setEnabled(bool)) );
  intervalLayout->addWidget( autoSaveInterval->label() );
  intervalLayout->addWidget( autoSaveInterval->spinBox() );

  KPrefsWidBool *confirmItem =
    addWidBool( KOPrefs::instance()->confirmItem(), saveFrame );
  saveLayout->addWidget( confirmItem->checkBox() );
  KPrefsWidRadios *destinationItem =
    addWidRadios( KOPrefs::instance()->destinationItem(), saveFrame );
  saveLayout->addWidget( destinationItem->groupBox() );
  saveLayout->addStretch( 1 );

  // System Tray Settings
  QFrame *systrayFrame = new QFrame( this );
  QVBoxLayout *systrayLayout = new QVBoxLayout( systrayFrame );
  tabWidget->addTab( systrayFrame, KIcon( "preferences-other" ),
                     i18nc( "@title:tab systray settings", "System Tray" ) );

  QGroupBox *systrayGroupBox =
    new QGroupBox( i18nc( "@title:group", "Show/Hide Options" ), systrayFrame );
  systrayLayout->addWidget( systrayGroupBox );
  QVBoxLayout *systrayGroupLayout = new QVBoxLayout;
  systrayGroupBox->setLayout( systrayGroupLayout );

  KPrefsWidBool *showReminderDaemonItem =
    addWidBool( KOPrefs::instance()->showReminderDaemonItem(), systrayGroupBox );
  systrayGroupLayout->addWidget( showReminderDaemonItem->checkBox() );
  showReminderDaemonItem->checkBox()->setToolTip(
    i18nc( "@info:tooltip", "Enable this setting to show the KOrganizer "
           "reminder daemon in your system tray (recommended)." ) );

  QLabel *note = new QLabel(
    i18nc( "@info",
           "<note>The daemon will continue running even if it is not shown "
           "in the system tray.</note>" ) );
  systrayGroupLayout->addWidget( note );

  systrayLayout->addStretch( 1 );

  load();
}

void KOPrefsDialogMain::toggleEmailSettings( bool on )
{
  mUserEmailSettings->setEnabled( !on );
/*  if (on) {
    KEMailSettings settings;
    mNameEdit->setText( settings.getSetting(KEMailSettings::RealName) );
    mEmailEdit->setText( settings.getSetting(KEMailSettings::EmailAddress) );
  } else {
    mNameEdit->setText( KOPrefs::instance()->mName );
    mEmailEdit->setText( KOPrefs::instance()->mEmail );
  }*/
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigmain( QWidget *parent, const char * )
  {
    return new KOPrefsDialogMain( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class KOPrefsDialogTime : public KPrefsModule
{
  public:
    KOPrefsDialogTime( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QVBoxLayout *layout = new QVBoxLayout( this );
      KTabWidget *tabWidget = new KTabWidget( this );
      layout->addWidget( tabWidget );

      QFrame *regionalPage = new QFrame( parent );
      tabWidget->addTab( regionalPage, KIcon( "flag" ),
                         i18nc( "@title:tab", "Regional" ) );

      QGridLayout *regionalLayout = new QGridLayout( regionalPage );
      regionalLayout->setSpacing( KDialog::spacingHint() );

      KHBox *timeZoneBox = new KHBox( regionalPage );
      regionalLayout->addWidget( timeZoneBox, 0, 0, 1, 2 );

      QLabel *timeZoneLabel = new QLabel( i18nc( "@label", "Time zone:" ), timeZoneBox );
      QString whatsThis = i18nc( "@info:whatsthis",
                                 "Select your time zone from the list of "
                                 "locations on this drop down box. If your "
                                 "city is not listed, select one which shares "
                                 "the same time zone. KOrganizer will "
                                 "automatically adjust for daylight savings." );
      timeZoneLabel->setWhatsThis( whatsThis );
      mTimeZoneCombo = new KComboBox( timeZoneBox );

      connect( mTimeZoneCombo, SIGNAL(activated(int)), SLOT(slotWidChanged()) );

      QString sCurrentlySet(
        i18nc( "@item:inlistbox unknown timezone", "Unknown" ) );
      KTimeZone zone = KSystemTimeZones::local();
      if ( zone.isValid() ) {
        sCurrentlySet = zone.name();
      }
      // Read all system time zones
      QStringList list;
      const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
      for ( KTimeZones::ZoneMap::ConstIterator it = timezones.constBegin();
            it != timezones.constEnd();  ++it ) {
        list.append( i18n( it.key().toUtf8() ) );
        tzonenames << it.key();
      }
      list.sort();
      mTimeZoneCombo->addItem(
        i18nc( "@item:inlistbox no timezone selected", "[No selection]" ) );
      mTimeZoneCombo->addItems( list );

      // find the currently set time zone and select it
      int nCurrentlySet = 0;
      for ( int i = 0; i < mTimeZoneCombo->count(); ++i ) {
        if ( mTimeZoneCombo->itemText(i) == sCurrentlySet ) {
          nCurrentlySet = i;
          break;
        }
      }
      mTimeZoneCombo->setCurrentIndex( nCurrentlySet );
      mTimeZoneCombo->setWhatsThis( whatsThis );

      // holiday region selection
      KHBox *holidayRegBox = new KHBox( regionalPage );
      regionalLayout->addWidget( holidayRegBox, 1, 0, 1, 2 );

      QLabel *holidayLabel = new QLabel(
        i18nc( "@label", "Use holiday region:" ), holidayRegBox );
      holidayLabel->setWhatsThis( KOPrefs::instance()->holidaysItem()->whatsThis() );

      mHolidayCombo = new KComboBox( holidayRegBox );
      connect( mHolidayCombo, SIGNAL(activated(int)), SLOT(slotWidChanged()) );

      mHolidayCombo->setWhatsThis( KOPrefs::instance()->holidaysItem()->whatsThis() );

      QString currentHolidayName;
      QStringList holidayList;
      QStringList countryList = HolidayRegion::locations();

      foreach ( const QString &country, countryList ) {
        QString countryFile = KStandardDirs::locate( "locale",
                                      "l10n/" + country + "/entry.desktop" );
        QString regionName;
        if ( !countryFile.isEmpty() ) {
          KConfig _cfg( countryFile, KConfig::SimpleConfig );
          KConfigGroup cfg(&_cfg, "KCM Locale" );
          regionName = cfg.readEntry( "Name" );
        }
        if ( regionName.isEmpty() ) {
          regionName = country;
        }

        holidayList << regionName;
        mRegionMap[regionName] = country; //store region for saving to config file

        if ( KOGlobals::self()->holidays() &&
             ( country == KOGlobals::self()->holidays()->location() ) ) {
          currentHolidayName = regionName;
        }
      }
      holidayList.sort();
      holidayList.push_front(
        i18nc( "@item:inlistbox do not use holidays", "(None)" ) );

      mHolidayCombo->addItems( holidayList );

      for ( int i=0; i < mHolidayCombo->count(); ++i ) {
        if ( mHolidayCombo->itemText(i) == currentHolidayName ) {
          mHolidayCombo->setCurrentIndex( i );
          break;
        }
      }

      KPrefsWidTime *dayBegins =
        addWidTime( KOPrefs::instance()->dayBeginsItem(), regionalPage );
      regionalLayout->addWidget( dayBegins->label(), 2, 0 );
      regionalLayout->addWidget( dayBegins->timeEdit(), 2, 1 );

      QGroupBox *workingHoursGroupBox =
        new QGroupBox( i18nc( "@title:group", "Working Hours" ), regionalPage );
      regionalLayout->addWidget( workingHoursGroupBox, 3, 0, 1, 2 );

      QBoxLayout *workingHoursLayout = new QVBoxLayout( workingHoursGroupBox );

      QBoxLayout *workDaysLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workDaysLayout );

      // Respect start of week setting
      int weekStart = KGlobal::locale()->weekStartDay();
      for ( int i=0; i < 7; ++i ) {
        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
        QString weekDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                                   KCalendarSystem::ShortDayName );
        if ( KOPrefs::instance()->mCompactDialogs ) {
          weekDayName = weekDayName.left( 1 );
        }
        int index = ( i + weekStart + 6 ) % 7;
        mWorkDays[ index ] = new QCheckBox( weekDayName );
        mWorkDays[ index ]->setWhatsThis(
          i18nc( "@info:whatsthis",
                 "Check this box to make KOrganizer mark the "
                 "working hours for this day of the week. "
                 "If this is a work day for you, check "
                 "this box, or the working hours will not be "
                 "marked with color." ) );

        connect( mWorkDays[ index ], SIGNAL(stateChanged(int)), SLOT(slotWidChanged()) );

        workDaysLayout->addWidget( mWorkDays[ index ] );
      }

      KPrefsWidTime *workStart =
        addWidTime( KOPrefs::instance()->workingHoursStartItem() );

      QHBoxLayout *workStartLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workStartLayout );

      workStartLayout->addWidget( workStart->label() );
      workStartLayout->addWidget( workStart->timeEdit() );

      KPrefsWidTime *workEnd =
        addWidTime( KOPrefs::instance()->workingHoursEndItem() );

      QHBoxLayout *workEndLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workEndLayout );

      workEndLayout->addWidget( workEnd->label() );
      workEndLayout->addWidget( workEnd->timeEdit() );

      KPrefsWidBool *excludeHolidays =
        addWidBool( KOPrefs::instance()->excludeHolidaysItem() );

      workingHoursLayout->addWidget( excludeHolidays->checkBox() );

      regionalLayout->setRowStretch( 4, 1 );

      QFrame *defaultPage = new QFrame( parent );
      tabWidget->addTab( defaultPage, KIcon( "draw-eraser" ),
                         i18nc( "@title:tab", "Default Values" ) );
      QGridLayout *defaultLayout = new QGridLayout( defaultPage );
      defaultLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidTime *defaultTime =
        addWidTime( KOPrefs::instance()->startTimeItem(), defaultPage );
      defaultLayout->addWidget( defaultTime->label(), 0, 0 );
      defaultLayout->addWidget( defaultTime->timeEdit(), 0, 1 );

      KPrefsWidDuration *defaultDuration =
        addWidDuration( KOPrefs::instance()->defaultDurationItem(), defaultPage );
      defaultLayout->addWidget( defaultDuration->label(), 1, 0 );
      defaultLayout->addWidget( defaultDuration->timeEdit(), 1, 1 );

      QStringList alarmList;
      alarmList << i18nc( "@item:inlistbox", "1 minute" )
                << i18nc( "@item:inlistbox", "5 minutes" )
                << i18nc( "@item:inlistbox", "10 minutes" )
                << i18nc( "@item:inlistbox", "15 minutes" )
                << i18nc( "@item:inlistbox", "30 minutes" );
      QLabel *alarmLabel = new QLabel( i18nc( "@label", "Default reminder time:" ), defaultPage );
      defaultLayout->addWidget( alarmLabel, 2, 0 );
      alarmLabel->setWhatsThis( KOPrefs::instance()->alarmTimeItem()->whatsThis() );
      mAlarmTimeCombo = new KComboBox( defaultPage );
      mAlarmTimeCombo->setWhatsThis( KOPrefs::instance()->alarmTimeItem()->whatsThis() );
      connect( mAlarmTimeCombo, SIGNAL(activated(int)), SLOT(slotWidChanged()) );
      mAlarmTimeCombo->addItems( alarmList );
      defaultLayout->addWidget( mAlarmTimeCombo, 2, 1 );

      defaultLayout->setRowStretch( 3, 1 );
      load();
    }

  protected:
    void usrReadConfig()
    {
      KTimeZone tz = KOPrefs::instance()->timeSpec().timeZone();
      if ( tz.isValid() ) {
        setCombo( mTimeZoneCombo, i18n( tz.name().toUtf8() ) );
      }

      mAlarmTimeCombo->setCurrentIndex( KOPrefs::instance()->mAlarmTime );
      for ( int i = 0; i < 7; ++i ) {
        mWorkDays[i]->setChecked( ( 1 << i ) & ( KOPrefs::instance()->mWorkWeekMask ) );
      }
    }

    void usrWriteConfig()
    {
      // Find untranslated selected zone
      QString selectedZone = mTimeZoneCombo->currentText();
      QStringList::Iterator tz;
      for ( tz = tzonenames.begin(); tz != tzonenames.end(); ++tz ) {
        if ( selectedZone == i18n( (*tz).toUtf8() ) ) {
          selectedZone = *tz;
          break;
        }
      }
      KTimeZone zone = KSystemTimeZones::zone( selectedZone );
      if ( zone.isValid() ) {
        KOPrefs::instance()->setTimeSpec(zone);
      }

      KOPrefs::instance()->mHolidays = ( mHolidayCombo->currentIndex() == 0 ) ?  // (None)
                                       QString() :
                                       mRegionMap[mHolidayCombo->currentText()];

      KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentIndex();
      int mask = 0;
      for ( int i = 0; i < 7; ++i ) {
        if ( mWorkDays[i]->isChecked() ) {
          mask = mask | ( 1 << i );
        }
      }
      KOPrefs::instance()->mWorkWeekMask = mask;
      KOPrefs::instance()->writeConfig();
    }

    void setCombo( KComboBox *combo, const QString &text, const QStringList *tags = 0 )
    {
      if ( tags ) {
        int i = tags->indexOf( text );
        if ( i > 0 ) {
          combo->setCurrentIndex( i );
        }
      } else {
        for ( int i=0; i < combo->count(); ++i ) {
          if ( combo->itemText( i ) == text ) {
            combo->setCurrentIndex( i );
            break;
          }
        }
      }
    }

  private:
    KComboBox    *mTimeZoneCombo;
    QStringList   tzonenames;
    KComboBox    *mHolidayCombo;
    QMap<QString,QString> mRegionMap;
    KComboBox    *mAlarmTimeCombo;
    QCheckBox    *mWorkDays[7];
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigtime( QWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalog( "timezones4" );
    return new KOPrefsDialogTime( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class KOPrefsDialogViews : public KPrefsModule
{
  public:
    KOPrefsDialogViews( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );
      KTabWidget *tabWidget = new KTabWidget( this );
      topTopLayout->addWidget( tabWidget );

      QFrame *topFrame = new QFrame( this );
      tabWidget->addTab( topFrame, KIcon( "view-choose" ),
                         i18nc( "@title:tab general settings", "General" ) );
      QBoxLayout *generalLayout = new QVBoxLayout( topFrame );
      generalLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidBool *enableToolTips =
        addWidBool( KOPrefs::instance()->enableToolTipsItem(), topFrame );
      generalLayout->addWidget( enableToolTips->checkBox() );

      KPrefsWidBool *showTodosAgenda =
        addWidBool( KOPrefs::instance()->showAllDayTodoItem(), topFrame );
      generalLayout->addWidget( showTodosAgenda->checkBox() );

      KPrefsWidBool *todosUseCategoryColors =
        addWidBool( KOPrefs::instance()->todosUseCategoryColorsItem(), topFrame );
      generalLayout->addWidget( todosUseCategoryColors->checkBox() );

      QGroupBox *dateNavGroup =
        new QGroupBox( i18nc( "@title:group", "Date Navigator" ), topFrame );
      generalLayout->addWidget( dateNavGroup );

      QBoxLayout *dateNavLayout = new QVBoxLayout( dateNavGroup );

      dateNavLayout->addWidget( addWidBool( KOPrefs::instance()->dailyRecurItem() )->checkBox() );
      dateNavLayout->addWidget( addWidBool( KOPrefs::instance()->weeklyRecurItem() )->checkBox() );

      generalLayout->addStretch( 1 );

      QFrame *agendaFrame = new QFrame( this );
      tabWidget->addTab( agendaFrame, KIcon( "view-calendar-workweek" ),
                         i18nc( "@title:tab", "Agenda View" ) );

      QBoxLayout *agendaLayout = new QVBoxLayout( agendaFrame );

      QVBoxLayout *displayLayout = new QVBoxLayout;
      QGroupBox *displayBox = new QGroupBox( i18nc( "@title:group", "Display Options" ) );

      QBoxLayout *hourSizeLayout = new QHBoxLayout;
      displayLayout->addLayout( hourSizeLayout );

      KPrefsWidInt *hourSize =
        addWidInt( KOPrefs::instance()->hourSizeItem() );
      hourSize->spinBox()->setSuffix(
        i18nc( "@label suffix in the hour size spin box", " pixel" ) );

      hourSizeLayout->addWidget( hourSize->label() );
      hourSizeLayout->addWidget( hourSize->spinBox() );
      hourSizeLayout->addStretch( 1 );

      QBoxLayout *nextDaysLayout = new QHBoxLayout;
      displayLayout->addLayout( nextDaysLayout );

      KPrefsWidInt *nextDays =
        addWidInt( KOPrefs::instance()->nextXDaysItem() );
      nextDays->spinBox()->setSuffix(
        i18nc( "@label suffix in the N days spin box", " days" ) );

      nextDaysLayout->addWidget( nextDays->label() );
      nextDaysLayout->addWidget( nextDays->spinBox() );
      nextDaysLayout->addStretch( 1 );

      KPrefsWidBool *marcusBainsEnabled =
        addWidBool( KOPrefs::instance()->marcusBainsEnabledItem() );
      displayLayout->addWidget( marcusBainsEnabled->checkBox() );

      KPrefsWidBool *marcusBainsShowSeconds =
        addWidBool( KOPrefs::instance()->marcusBainsShowSecondsItem() );
      connect( marcusBainsEnabled->checkBox(), SIGNAL(toggled(bool)),
               marcusBainsShowSeconds->checkBox(), SLOT(setEnabled(bool)) );

      displayLayout->addWidget( marcusBainsShowSeconds->checkBox() );
      displayLayout->addWidget(
        addWidBool( KOPrefs::instance()->selectionStartsEditorItem() )->checkBox() );

      displayBox->setLayout( displayLayout );

      agendaLayout->addWidget( displayBox );
      agendaLayout->addWidget(
        addWidRadios( KOPrefs::instance()->agendaViewColorsItem() )->groupBox() );
      agendaLayout->addWidget(
        addWidRadios( KOPrefs::instance()->agendaViewCalendarDisplayItem() )->groupBox() );

      agendaLayout->addStretch( 1 );

      QFrame *monthFrame = new QFrame( this );
      tabWidget->addTab( monthFrame, KIcon( "view-calendar-month" ),
                         i18nc( "@title:tab", "Month View" ) );

      QBoxLayout *monthLayout = new QVBoxLayout( monthFrame );

      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableMonthScrollItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableMonthItemIconsItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->fullViewMonthItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->monthViewUsesCategoryColorItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->monthViewUsesResourceColorItem() )->checkBox() );

      monthLayout->addStretch( 1 );

      QFrame *todoFrame = new QFrame( this );
      tabWidget->addTab( todoFrame, KIcon( "view-calendar-tasks" ),
                         i18nc( "@title:tab", "Todo View" ) );

      QBoxLayout *todoLayout = new QVBoxLayout( todoFrame );
      todoLayout->addWidget(
        addWidBool( KOPrefs::instance()->fullViewTodoItem() )->checkBox() );
      todoLayout->addWidget(
        addWidBool( KOPrefs::instance()->recordTodosInJournalsItem() )->checkBox() );
      todoLayout->addWidget(
        addWidBool( KOPrefs::instance()->sortCompletedTodosSeparatelyItem() )->checkBox() );
      todoLayout->addStretch( 1 );

      load();
    }
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigviews( QWidget *parent, const char * )
  {
    return new KOPrefsDialogViews( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogColorsAndFonts::KOPrefsDialogColorsAndFonts( const KComponentData &inst,
                                                          QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );
  KTabWidget *tabWidget = new KTabWidget( this );
  topTopLayout->addWidget( tabWidget );

  QWidget *colorFrame = new QWidget( this );
  topTopLayout->addWidget( colorFrame );
  QGridLayout *colorLayout = new QGridLayout(colorFrame);
  colorLayout->setSpacing( KDialog::spacingHint() );
  tabWidget->addTab( colorFrame, KIcon( "preferences-desktop-color" ),
                     i18nc( "@title:tab", "Colors" ) );

  // Holiday Color
  KPrefsWidColor *holidayColor =
    addWidColor( KOPrefs::instance()->agendaHolidaysBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( holidayColor->label(), 0, 0 );
  colorLayout->addWidget( holidayColor->button(), 0, 1 );

  // agenda view background color
  KPrefsWidColor *agendaBgColor =
    addWidColor( KOPrefs::instance()->agendaGridBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( agendaBgColor->label(), 3, 0 );
  colorLayout->addWidget( agendaBgColor->button(), 3, 1 );

  // working hours color
  KPrefsWidColor *agendaGridWorkHoursBackgroundColor =
    addWidColor( KOPrefs::instance()->agendaGridWorkHoursBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( agendaGridWorkHoursBackgroundColor->label(), 4, 0 );
  colorLayout->addWidget( agendaGridWorkHoursBackgroundColor->button(), 4, 1 );

  // Todo due today color
  KPrefsWidColor *todoDueTodayColor =
    addWidColor(
      KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( todoDueTodayColor->label(), 5, 0 );
  colorLayout->addWidget( todoDueTodayColor->button(), 5, 1 );

  // Todo overdue color
  KPrefsWidColor *todoOverdueColor =
    addWidColor(
      KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( todoOverdueColor->label(), 6, 0 );
  colorLayout->addWidget( todoOverdueColor->button(), 6, 1 );

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox( i18nc( "@title:group", "Categories" ), colorFrame );
  colorLayout->addWidget( categoryGroup, 7, 0, 1, 2 );

  QBoxLayout *categoryLayout = new QHBoxLayout;
  categoryGroup->setLayout( categoryLayout );

  mCategoryCombo = new KComboBox(categoryGroup);
  mCategoryCombo->addItems( KOPrefs::instance()->mCustomCategories );
  mCategoryCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select here the event category you want to modify. "
           "You can change the selected category color using "
           "the button below." ) );
  connect( mCategoryCombo, SIGNAL(activated(int)), SLOT(updateCategoryColor()) );
  categoryLayout->addWidget( mCategoryCombo );

  mCategoryButton = new KColorButton( categoryGroup );
  mCategoryButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose here the color of the event category selected "
           "using the combo box above." ) );
  connect( mCategoryButton, SIGNAL(changed(const QColor &)), SLOT(setCategoryColor()) );
  categoryLayout->addWidget( mCategoryButton );

  updateCategoryColor();

  // resources colors
  QGroupBox *resourceGroup = new QGroupBox( i18nc( "@title:group", "Resources" ), colorFrame );
  colorLayout->addWidget( resourceGroup, 8, 0, 1, 2 );

  QBoxLayout *resourceLayout = new QHBoxLayout;
  resourceGroup->setLayout( resourceLayout );

  mResourceCombo = new KComboBox(resourceGroup);
  mResourceCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select here resource you want to modify. "
           "You can change the selected resource color using "
           "the button below." ) );
  connect( mResourceCombo, SIGNAL(activated(int)), SLOT(updateResourceColor()) );
  resourceLayout->addWidget( mResourceCombo );

  mResourceButton = new KColorButton( resourceGroup );
  mResourceButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose here the color of the resource selected "
           "using the combo box above." ) );
  connect( mResourceButton, SIGNAL(changed(const QColor &)), SLOT(setResourceColor()) );
  resourceLayout->addWidget( mResourceButton );

  updateResources();

  colorLayout->setRowStretch( 9, 1 );

  QWidget *fontFrame = new QWidget( this );
  tabWidget->addTab( fontFrame, KIcon( "preferences-desktop-font" ),
                     i18nc( "@title:tab", "Fonts" ) );

  QGridLayout *fontLayout = new QGridLayout( fontFrame );
  fontLayout->setSpacing( KDialog::spacingHint() );

  KPrefsWidFont *timeBarFont =
    addWidFont( KOPrefs::instance()->agendaTimeLabelsFontItem(), fontFrame,
                KGlobal::locale()->formatTime( QTime( 12, 34 ) ) );
  fontLayout->addWidget( timeBarFont->label(), 0, 0 );
  fontLayout->addWidget( timeBarFont->preview(), 0, 1 );
  fontLayout->addWidget( timeBarFont->button(), 0, 2 );

  KPrefsWidFont *monthViewFont =
    addWidFont( KOPrefs::instance()->monthViewFontItem(), fontFrame,
                KGlobal::locale()->formatTime( QTime( 12, 34 ) ) + ' ' +
                i18nc( "@label", "Event text" ) );

  fontLayout->addWidget( monthViewFont->label(), 1, 0 );
  fontLayout->addWidget( monthViewFont->preview(), 1, 1 );
  fontLayout->addWidget( monthViewFont->button(), 1, 2 );

  KPrefsWidFont *agendaViewFont =
    addWidFont( KOPrefs::instance()->agendaViewFontItem(), fontFrame,
                i18nc( "@label", "Event text" ) );
  fontLayout->addWidget( agendaViewFont->label(), 2, 0 );
  fontLayout->addWidget( agendaViewFont->preview(), 2, 1 );
  fontLayout->addWidget( agendaViewFont->button(), 2, 2 );

  KPrefsWidFont *marcusBainsFont =
    addWidFont( KOPrefs::instance()->agendaMarcusBainsLineFontItem(), fontFrame,
                KGlobal::locale()->formatTime( QTime( 12, 34, 23 ) ) );
  fontLayout->addWidget( marcusBainsFont->label(), 3, 0 );
  fontLayout->addWidget( marcusBainsFont->preview(), 3, 1 );
  fontLayout->addWidget( marcusBainsFont->button(), 3, 2 );

  fontLayout->setColumnStretch( 1, 1 );
  fontLayout->setRowStretch( 4, 1 );

  load();
}

void KOPrefsDialogColorsAndFonts::usrWriteConfig()
{
  QHash<QString, QColor>::const_iterator i = mCategoryDict.constBegin();
  while ( i != mCategoryDict.constEnd() ) {
    KOPrefs::instance()->setCategoryColor( i.key(), i.value() );
    ++i;
  }

  i = mResourceDict.constBegin();
  while ( i != mResourceDict.constEnd() ) {
    KOPrefs::instance()->setResourceColor( i.key(), i.value() );
    ++i;
  }
}

void KOPrefsDialogColorsAndFonts::usrReadConfig()
{
  updateCategories();
  updateResources();
}

void KOPrefsDialogColorsAndFonts::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->addItems( KOPrefs::instance()->mCustomCategories );
  updateCategoryColor();
}

void KOPrefsDialogColorsAndFonts::setCategoryColor()
{
  mCategoryDict.insert( mCategoryCombo->currentText(), mCategoryButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateCategoryColor()
{
  QString cat = mCategoryCombo->currentText();
  QColor color = mCategoryDict.value( cat );
  if ( !color.isValid() ) {
    color = KOPrefs::instance()->categoryColor( cat );
  }
  if ( color.isValid() ) {
    mCategoryButton->setColor( color );
  }
}

void KOPrefsDialogColorsAndFonts::updateResources()
{
  mResourceCombo->clear();
  mResourceIdentifier.clear();

  KCal::CalendarResourceManager *manager = KOrg::StdCalendar::self()->resourceManager();

  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    if ( !(*it)->subresources().isEmpty() ) {
      QStringList subresources = (*it)->subresources();
      for ( int i = 0; i < subresources.count(); ++i ) {
        QString resource = subresources.at( i );
        if ( (*it)->subresourceActive( resource ) ) {
          mResourceCombo->addItem( (*it)->labelForSubresource( resource ) );
          mResourceIdentifier.append( resource );
        }
      }
    }

    mResourceCombo->addItem( (*it)->resourceName() );
    mResourceIdentifier.append( (*it)->identifier() );
  }

  updateResourceColor();
}

void KOPrefsDialogColorsAndFonts::setResourceColor()
{
  mResourceDict.insert( mResourceIdentifier[mResourceCombo->currentIndex()],
                        mResourceButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateResourceColor()
{
  QString res= mResourceIdentifier[mResourceCombo->currentIndex()];
  QColor color = mCategoryDict.value( res );
  if ( color.isValid() )  {
    color = KOPrefs::instance()->resourceColor( res );
  }
  if ( color.isValid() ) {
    mResourceButton->setColor( color );
  }
}
extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigcolorsandfonts( QWidget *parent, const char * )
  {
    return new KOPrefsDialogColorsAndFonts( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling( const KComponentData &inst,
                                                            QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout( topFrame );
  topLayout->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *useGroupwareBool =
    addWidBool( KOPrefs::instance()->useGroupwareCommunicationItem(), topFrame );
  topLayout->addWidget( useGroupwareBool->checkBox(), 0, 0, 1, 2 );
  // FIXME: This radio button should only be available when KMail is chosen
//   connect( thekmailradiobuttonupthere, SIGNAL(toggled(bool)),
//            useGroupwareBool->checkBox(), SLOT(enabled(bool)) );

  KPrefsWidBool *bcc =
    addWidBool( KOPrefs::instance()->bccItem(), topFrame );
  topLayout->addWidget( bcc->checkBox(), 1, 0, 1, 2 );

  KPrefsWidRadios *mailClientGroup =
    addWidRadios( KOPrefs::instance()->mailClientItem(), topFrame );
  topLayout->addWidget( mailClientGroup->groupBox(), 2, 0, 1, 2 );

#if 0
  KPrefsWidRadios *schedulerGroup =
    addWidRadios(
      i18nc( "@title", "Scheduler Mail Client" ), KOPrefs::instance()->mIMIPScheduler, topFrame );
  schedulerGroup->addRadio( "Dummy" ); // Only for debugging
  schedulerGroup->addRadio( i18nc( "@option:radio", "Mail client" ) );

  topLayout->addWidget( schedulerGroup->groupBox(), 0, 0, 1, 2 );
#endif

  QLabel *aMailsLabel = new QLabel(
    i18nc( "@label", "Additional email addresses:" ), topFrame );
  QString whatsThis = i18nc( "@info:whatsthis",
                             "Add, edit or remove additional e-mails addresses "
                             "here. These email addresses are the ones you "
                             "have in addition to the one set in personal "
                             "preferences. If you are an attendee of one event, "
                             "but use another email address there, you need to "
                             "list this address here so KOrganizer can "
                             "recognize it as yours." );
  aMailsLabel->setWhatsThis( whatsThis );
  topLayout->addWidget( aMailsLabel, 3, 0, 1, 2 );
  mAMails = new Q3ListView( topFrame );
  mAMails->setWhatsThis( whatsThis );

  mAMails->addColumn( i18nc( "@title:column email addresses", "Email" ), 300 );
  topLayout->addWidget( mAMails, 4, 0, 1, 2 );

  QLabel *aEmailsEditLabel = new QLabel( i18nc( "@label", "Additional email address:" ), topFrame );
  whatsThis = i18nc( "@info:whatsthis",
                     "Edit additional e-mails addresses here. To edit an "
                     "address select it from the list above "
                     "or press the \"New\" button below. These email "
                     "addresses are the ones you have in addition to the "
                     "one set in personal preferences." );
  aEmailsEditLabel->setWhatsThis( whatsThis );
  topLayout->addWidget( aEmailsEditLabel, 5, 0 );
  aEmailsEdit = new KLineEdit( topFrame );
  aEmailsEdit->setWhatsThis( whatsThis );
  aEmailsEdit->setEnabled( false );
  topLayout->addWidget( aEmailsEdit, 5, 1 );

  QPushButton *add = new QPushButton(
    i18nc( "@action:button add a new email address", "New" ), topFrame );
  add->setObjectName( "new" );
  whatsThis = i18nc( "@info:whatsthis",
                     "Press this button to add a new entry to the "
                     "additional e-mail addresses list. Use the edit "
                     "box above to edit the new entry." );
  add->setWhatsThis( whatsThis );
  topLayout->addWidget( add, 6, 0 );
  QPushButton *del = new QPushButton( i18nc( "@action:button", "Remove" ), topFrame );
  del->setObjectName( "remove" );
  del->setWhatsThis( whatsThis );
  topLayout->addWidget( del, 6, 1 );

  //topLayout->setRowStretch( 2, 1 );
  connect( add, SIGNAL(clicked()), this, SLOT(addItem()) );
  connect( del, SIGNAL(clicked()), this, SLOT(removeItem()) );
  connect( aEmailsEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateItem()) );
  connect( aEmailsEdit, SIGNAL(lostFocus()), this, SLOT(checkEmptyMail()) );
  connect( mAMails, SIGNAL(selectionChanged(Q3ListViewItem *)), SLOT(updateInput()) );

  load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
  mAMails->clear();
  for ( QStringList::Iterator it = KOPrefs::instance()->mAdditionalMails.begin();
            it != KOPrefs::instance()->mAdditionalMails.end(); ++it ) {
    Q3ListViewItem *item = new Q3ListViewItem(mAMails);
    item->setText( 0, *it );
    mAMails->insertItem( item );
  }
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mAdditionalMails.clear();
  Q3ListViewItem *item;
  item = mAMails->firstChild();
  while ( item ) {
    KOPrefs::instance()->mAdditionalMails.append( item->text( 0 ) );
    item = item->nextSibling();
  }
}

void KOPrefsDialogGroupScheduling::addItem()
{
  aEmailsEdit->setEnabled( true );
  Q3ListViewItem *item = new Q3ListViewItem( mAMails );
  mAMails->insertItem( item );
  mAMails->setSelected( item, true );
  aEmailsEdit->setText( i18nc( "@label", "(EmptyEmail)" ) );
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::removeItem()
{
  Q3ListViewItem *item;
  item = mAMails->selectedItem();
  if ( !item ) {
    return;
  }
  mAMails->takeItem( item );
  item = mAMails->selectedItem();
  if ( !item ) {
    aEmailsEdit->setText( "" );
    aEmailsEdit->setEnabled( false );
  }
  if ( mAMails->childCount() == 0 ) {
    aEmailsEdit->setEnabled( false );
  }
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::updateItem()
{
  Q3ListViewItem *item;
  item = mAMails->selectedItem();
  if ( !item ) {
    return;
  }
  item->setText( 0, aEmailsEdit->text() );
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::checkEmptyMail()
{
  if ( aEmailsEdit->text().isEmpty() ) {
    removeItem();
  };
}

void KOPrefsDialogGroupScheduling::updateInput()
{
  Q3ListViewItem *item;
  item = mAMails->selectedItem();
  if ( !item ) {
    return;
  }
  aEmailsEdit->setEnabled( true );
  aEmailsEdit->setText( item->text( 0 ) );
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfiggroupscheduling( QWidget *parent, const char * )
  {
    return new KOPrefsDialogGroupScheduling( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling( const KComponentData &inst,
                                                                    QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  mGroupwarePage = new Ui::KOGroupwarePrefsPage();
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "KOGrouparePrefsPage" );

  mGroupwarePage->setupUi( widget );

  mGroupwarePage->groupwareTab->setTabIcon( 0, KIcon( "go-up" ) );
  mGroupwarePage->groupwareTab->setTabIcon( 1, KIcon( "go-down" ) );

  // signals and slots connections

  connect( mGroupwarePage->publishDays, SIGNAL(valueChanged(int)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishUrl, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishUser, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishPassword, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishSavePassword, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveEnable, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveUser, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrievePassword, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveSavePassword, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveUrl, SIGNAL(textChanged(const QString&)),
           SLOT(slotWidChanged()));
  connect( mGroupwarePage->publishDelay, SIGNAL(valueChanged(int)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->fullDomainRetrieval, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishEnable, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );

  ( new QVBoxLayout( this ) )->addWidget( widget );

  load();
}

KOPrefsDialogGroupwareScheduling::~KOPrefsDialogGroupwareScheduling()
{
  delete mGroupwarePage;
}

void KOPrefsDialogGroupwareScheduling::usrReadConfig()
{
  mGroupwarePage->publishEnable->setChecked(
    KOPrefs::instance()->mFreeBusyPublishAuto );
  mGroupwarePage->publishDelay->setValue(
    KOPrefs::instance()->mFreeBusyPublishDelay );
  mGroupwarePage->publishDays->setValue(
    KOPrefs::instance()->mFreeBusyPublishDays );
  mGroupwarePage->publishUrl->setText(
    KOPrefs::instance()->mFreeBusyPublishUrl );
  mGroupwarePage->publishUser->setText(
    KOPrefs::instance()->mFreeBusyPublishUser );
  mGroupwarePage->publishPassword->setText(
    KOPrefs::instance()->mFreeBusyPublishPassword );
  mGroupwarePage->publishSavePassword->setChecked(
    KOPrefs::instance()->mFreeBusyPublishSavePassword );

  mGroupwarePage->retrieveEnable->setChecked(
    KOPrefs::instance()->mFreeBusyRetrieveAuto );
  mGroupwarePage->fullDomainRetrieval->setChecked(
    KOPrefs::instance()->mFreeBusyFullDomainRetrieval );
  mGroupwarePage->retrieveUrl->setText(
    KOPrefs::instance()->mFreeBusyRetrieveUrl );
  mGroupwarePage->retrieveUser->setText(
    KOPrefs::instance()->mFreeBusyRetrieveUser );
    mGroupwarePage->retrievePassword->setText(
    KOPrefs::instance()->mFreeBusyRetrievePassword );
  mGroupwarePage->retrieveSavePassword->setChecked(
    KOPrefs::instance()->mFreeBusyRetrieveSavePassword );
}

void KOPrefsDialogGroupwareScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mFreeBusyPublishAuto =
    mGroupwarePage->publishEnable->isChecked();
  KOPrefs::instance()->mFreeBusyPublishDelay =
    mGroupwarePage->publishDelay->value();
  KOPrefs::instance()->mFreeBusyPublishDays =
    mGroupwarePage->publishDays->value();
  KOPrefs::instance()->mFreeBusyPublishUrl =
    mGroupwarePage->publishUrl->text();
  KOPrefs::instance()->mFreeBusyPublishUser =
    mGroupwarePage->publishUser->text();
  KOPrefs::instance()->mFreeBusyPublishPassword =
    mGroupwarePage->publishPassword->text();
  KOPrefs::instance()->mFreeBusyPublishSavePassword =
    mGroupwarePage->publishSavePassword->isChecked();

  KOPrefs::instance()->mFreeBusyRetrieveAuto =
    mGroupwarePage->retrieveEnable->isChecked();
  KOPrefs::instance()->mFreeBusyFullDomainRetrieval =
    mGroupwarePage->fullDomainRetrieval->isChecked();
  KOPrefs::instance()->mFreeBusyRetrieveUrl =
    mGroupwarePage->retrieveUrl->text();
  KOPrefs::instance()->mFreeBusyRetrieveUser =
    mGroupwarePage->retrieveUser->text();
  KOPrefs::instance()->mFreeBusyRetrievePassword =
    mGroupwarePage->retrievePassword->text();
  KOPrefs::instance()->mFreeBusyRetrieveSavePassword =
    mGroupwarePage->retrieveSavePassword->isChecked();
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigfreebusy( QWidget *parent, const char * )
  {
    return new KOPrefsDialogGroupwareScheduling( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class PluginItem : public QTreeWidgetItem
{
  public:
    PluginItem( QTreeWidget *parent, KService::Ptr service )
      : QTreeWidgetItem( parent, QStringList( service->name() ) ),
        mService( service ) {}
    PluginItem( QTreeWidgetItem *parent, KService::Ptr service )
      : QTreeWidgetItem( parent, QStringList( service->name() ) ),
        mService( service ) {}

    KService::Ptr service()
    { return mService; }

  private:
    KService::Ptr mService;
};

/**
  Dialog for selecting and configuring KOrganizer plugins
*/
KOPrefsDialogPlugins::KOPrefsDialogPlugins( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( KDialog::spacingHint() );

  mTreeWidget = new QTreeWidget( topFrame );
  mTreeWidget->setColumnCount( 1 );
  mTreeWidget->setHeaderLabel( i18nc( "@title:column plugin name", "Name" ) );
  topLayout->addWidget( mTreeWidget );

  mDescription = new QLabel( topFrame );
  mDescription->setAlignment( Qt::AlignVCenter );
  mDescription->setWordWrap( true );
  mDescription->setFrameShape( QLabel::Panel );
  mDescription->setFrameShadow( QLabel::Sunken );
  mDescription->setMinimumSize( QSize( 0, 55 ) );
  QSizePolicy policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  policy.setHorizontalStretch( 0 );
  policy.setVerticalStretch( 0 );
  policy.setHeightForWidth( mDescription->sizePolicy().hasHeightForWidth() );
  mDescription->setSizePolicy( policy );
  topLayout->addWidget( mDescription );

  QWidget *buttonRow = new QWidget( topFrame );
  QBoxLayout *buttonRowLayout = new QHBoxLayout( buttonRow );
  mConfigureButton = new KPushButton(
    KGuiItem( i18nc( "@action:button", "Configure &Plugin..." ),
              "configure", QString(),
              i18nc( "@info:whatsthis",
                     "This button allows you to configure"
                     " the plugin that you have selected in the list above" ) ),
    buttonRow );
  buttonRowLayout->addWidget( mConfigureButton );
  buttonRowLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding ) );
  topLayout->addWidget( buttonRow );

  mPositioningGroupBox = new QGroupBox( i18nc( "@title:group", "Position" ), topFrame );
  mPositionMonthTop = new QCheckBox(
    i18nc( "@option:check", "Show in the month view" ), mPositioningGroupBox );
  mPositionAgendaTop = new QRadioButton(
    i18nc( "@option:check", "Show at the top of the agenda views" ), mPositioningGroupBox );
  mPositionAgendaBottom = new QRadioButton(
    i18nc( "@option:check", "Show at the bottom of the agenda views" ), mPositioningGroupBox );
  QVBoxLayout *positioningLayout = new QVBoxLayout( mPositioningGroupBox );
  positioningLayout->addWidget( mPositionMonthTop );
  positioningLayout->addWidget( mPositionAgendaTop );
  positioningLayout->addWidget( mPositionAgendaBottom );
  positioningLayout->addStretch( 1 );
  topLayout->addWidget( mPositioningGroupBox );

  connect( mConfigureButton, SIGNAL(clicked()), SLOT(configure()) );

  connect( mPositionMonthTop, SIGNAL(clicked()), SLOT(positioningChanged()) );
  connect( mPositionAgendaTop, SIGNAL(clicked()), SLOT(positioningChanged()) );
  connect( mPositionAgendaBottom, SIGNAL(clicked()), SLOT(positioningChanged()) );

  connect( mTreeWidget, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()) );
  connect( mTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(selectionChanged()) );
  connect( mTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotWidChanged()) );

  load();

  selectionChanged();
}

void KOPrefsDialogPlugins::usrReadConfig()
{
  mTreeWidget->clear();
  KService::List plugins = KOCore::self()->availablePlugins();
  plugins += KOCore::self()->availableParts();

  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  QTreeWidgetItem *decorations =
    new QTreeWidgetItem( mTreeWidget, QStringList(
                           i18nc( "@title:group", "Calendar Decorations" ) ) );
  QTreeWidgetItem *printPlugins =
    new QTreeWidgetItem( mTreeWidget, QStringList(
                           i18nc( "@title:group", "Print Plugins" ) ) );
  QTreeWidgetItem *others =
    new QTreeWidgetItem( mTreeWidget, QStringList(
                           i18nc( "@title:group", "Other Plugins" ) ) );

  KService::List::ConstIterator it;
  for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
    QTreeWidgetItem *item;
    if ( (*it)->hasServiceType( KOrg::CalendarDecoration::Decoration::serviceType() ) ) {
      item = new PluginItem( decorations, *it );
    } else if ( (*it)->hasServiceType( KOrg::PrintPlugin::serviceType() ) ){
      item = new PluginItem( printPlugins, *it );
    } else {
      item = new PluginItem( others, *it );
    }
    if ( selectedPlugins.contains( (*it)->desktopEntryName() ) ) {
      item->setCheckState( 0, Qt::Checked );
    } else {
      item->setCheckState( 0, Qt::Unchecked );
    }
  }

  decorations->setExpanded( true );
  printPlugins->setExpanded( true );
  others->setExpanded( true );

  mDecorationsAtMonthViewTop = KOPrefs::instance()->decorationsAtMonthViewTop().toSet();
  mDecorationsAtAgendaViewTop = KOPrefs::instance()->decorationsAtAgendaViewTop().toSet();
  mDecorationsAtAgendaViewBottom = KOPrefs::instance()->decorationsAtAgendaViewBottom().toSet();
}

void KOPrefsDialogPlugins::usrWriteConfig()
{
  QStringList selectedPlugins;

  for ( int i = 0; i < mTreeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *serviceTypeGroup = mTreeWidget->topLevelItem( i );
    for ( int j = 0; j < serviceTypeGroup->childCount(); j++) {
      PluginItem *item = static_cast<PluginItem *>( serviceTypeGroup->child( j ) );
      if( item->checkState( 0 ) == Qt::Checked ) {
        selectedPlugins.append( item->service()->desktopEntryName() );
      }
    }
  }
  KOPrefs::instance()->mSelectedPlugins = selectedPlugins;

  KOPrefs::instance()->setDecorationsAtMonthViewTop( mDecorationsAtMonthViewTop.toList() );
  KOPrefs::instance()->setDecorationsAtAgendaViewTop( mDecorationsAtAgendaViewTop.toList() );
  KOPrefs::instance()->setDecorationsAtAgendaViewBottom( mDecorationsAtAgendaViewBottom.toList() );
}

void KOPrefsDialogPlugins::configure()
{
  if ( mTreeWidget->selectedItems().count() != 1 ) {
    return;
  }

  PluginItem *item = static_cast<PluginItem *>( mTreeWidget->selectedItems().last() );
  if ( !item ) {
    return;
  }

  KOrg::Plugin *plugin = KOCore::self()->loadPlugin( item->service() );

  if ( plugin ) {
    plugin->configure( this );
    delete plugin;

    slotWidChanged();
  } else {
    KMessageBox::sorry( this,
                        i18nc( "@info", "Unable to configure this plugin" ),
                        "PluginConfigUnable" );
  }
}

void KOPrefsDialogPlugins::positioningChanged()
{
  if ( mTreeWidget->selectedItems().count() != 1 ) {
    return;
  }

  PluginItem *item = dynamic_cast<PluginItem*>( mTreeWidget->selectedItems().last() );
  if ( !item ) {
    return;
  }

  QString decoration = item->service()->desktopEntryName();

  if ( mPositionMonthTop->checkState() == Qt::Checked ) {
    if ( !mDecorationsAtMonthViewTop.contains( decoration ) ) {
      mDecorationsAtMonthViewTop.insert( decoration );
    }
  } else {
    mDecorationsAtMonthViewTop.remove( decoration );
  }

  if ( mPositionAgendaTop->isChecked() ) {
    if ( !mDecorationsAtAgendaViewTop.contains( decoration ) ) {
      mDecorationsAtAgendaViewTop.insert( decoration );
    }
  } else {
    mDecorationsAtAgendaViewTop.remove( decoration );
  }

  if ( mPositionAgendaBottom->isChecked() ) {
    if ( !mDecorationsAtAgendaViewBottom.contains( decoration ) ) {
      mDecorationsAtAgendaViewBottom.insert( decoration );
    }
  } else {
    mDecorationsAtAgendaViewBottom.remove( decoration );
  }

  slotWidChanged();
}

void KOPrefsDialogPlugins::selectionChanged()
{
  mPositioningGroupBox->hide();
  mPositionMonthTop->setChecked( false );
  mPositionAgendaTop->setChecked( false );
  mPositionAgendaBottom->setChecked( false );

  if ( mTreeWidget->selectedItems().count() != 1 ) {
    mConfigureButton->setEnabled( false );
    mDescription->setText( QString() );
    return;
  }

  PluginItem *item = dynamic_cast<PluginItem*>( mTreeWidget->selectedItems().last() );
  if ( !item ) {
    mConfigureButton->setEnabled( false );
    mDescription->setText( QString() );
    return;
  }

  QVariant variant = item->service()->property( "X-KDE-KOrganizer-HasSettings" );

  bool hasSettings = true;
  if ( variant.isValid() ) {
    hasSettings = variant.toBool();
  }

  mDescription->setText( item->service()->comment() );
  if ( !hasSettings ) {
    mConfigureButton->hide();
  } else {
    mConfigureButton->show();
    mConfigureButton->setEnabled( item->checkState(0) == Qt::Checked );
  }

  bool hasPosition = false;
  if ( item->service()->hasServiceType( KOrg::CalendarDecoration::Decoration::serviceType() ) ) {
    QString decoration = item->service()->desktopEntryName();
    if ( mDecorationsAtMonthViewTop.contains( decoration ) ) {
      mPositionMonthTop->setChecked( true );
      hasPosition = true;
    }
    if ( mDecorationsAtAgendaViewTop.contains( decoration ) ) {
      mPositionAgendaTop->setChecked( true );
      hasPosition = true;
    }
    if ( mDecorationsAtAgendaViewBottom.contains( decoration ) ) {
      mPositionAgendaBottom->setChecked( true );
      hasPosition = true;
    }

    if ( !hasPosition ) {
      // no position has been selected, so default to Agenda Top
      mDecorationsAtAgendaViewTop << decoration;
      mPositionAgendaTop->setChecked( true );
    }

    mPositioningGroupBox->setEnabled( item->checkState(0) == Qt::Checked );
    mPositioningGroupBox->show();
  }

  slotWidChanged();
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigplugins( QWidget *parent, const char * )
  {
    return new KOPrefsDialogPlugins( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
  KDE_EXPORT KCModule *create_korgdesignerfields( QWidget *parent, const char * ) {
    return new KOPrefsDesignerFields( KOGlobals::self()->componentData(), parent );
  }
}

KOPrefsDesignerFields::KOPrefsDesignerFields( const KComponentData &inst, QWidget *parent )
  : KCMDesignerFields( inst, parent )
{
}

QString KOPrefsDesignerFields::localUiDir()
{
  QString dir = KStandardDirs::locateLocal( "data", "korganizer/designer/event/" );
  return dir;
}

QString KOPrefsDesignerFields::uiPath()
{
  return "korganizer/designer/event/";
}

void KOPrefsDesignerFields::writeActivePages( const QStringList &activePages )
{
  KOPrefs::instance()->setActiveDesignerFields( activePages );
  KOPrefs::instance()->writeConfig();
}

QStringList KOPrefsDesignerFields::readActivePages()
{
  return KOPrefs::instance()->activeDesignerFields();
}

QString KOPrefsDesignerFields::applicationName()
{
  return "KORGANIZER";
}

#include "koprefsdialog.moc"
