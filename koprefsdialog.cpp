/*
    This file is part of KOrganizer.

    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QFile>
#include <QComboBox>


#include <q3grid.h>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <q3listview.h>
#include <QTabWidget>
#include <q3buttongroup.h>
#include <QTextStream>
//Added by qt3to4:
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kemailsettings.h>
#include <kcalendarsystem.h>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kocore.h>
#include <kcal/calendarresources.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kholidays.h>

#if defined(USE_SOLARIS)
#include <sys/param.h>

#define INITFILE       "/etc/default/init"
#endif

#include "koprefs.h"

#include "ui_kogroupwareprefspage.h"
#include "koprefsdialog.h"
#include "ktimeedit.h"
#include "koglobals.h"
#include "stdcalendar.h"
#include <kdemacros.h>
#include <kvbox.h>


KOPrefsDialogMain::KOPrefsDialogMain( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  KVBox *topFrame = new KVBox( this );
  topTopLayout->addWidget( topFrame );

  topFrame->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *emailControlCenter =
      addWidBool( KOPrefs::instance()->emailControlCenterItem(), topFrame );
  connect(emailControlCenter->checkBox(),SIGNAL(toggled(bool)),
          SLOT(toggleEmailSettings(bool)));

  mUserEmailSettings = new Q3Grid( 2, topFrame );

  addWidString( KOPrefs::instance()->userNameItem(), mUserEmailSettings );
  addWidString( KOPrefs::instance()->userEmailItem(), mUserEmailSettings );

  QGroupBox *saveGroupBox = new QGroupBox( i18n("Saving Calendar"), topFrame );

  QVBoxLayout *saveGroupLayout = new QVBoxLayout;
  saveGroupBox->setLayout( saveGroupLayout );

  KPrefsWidBool *htmlWithSave = addWidBool(
    KOPrefs::instance()->htmlWithSaveItem(), saveGroupBox );
  saveGroupLayout->addWidget( htmlWithSave->checkBox() );

  KPrefsWidBool *autoSave = addWidBool( KOPrefs::instance()->autoSaveItem(),
    saveGroupBox );
  saveGroupLayout->addWidget( autoSave->checkBox() );
    
  QBoxLayout *intervalLayout = new QHBoxLayout;
  saveGroupLayout->addLayout( intervalLayout );

  KPrefsWidInt *autoSaveInterval = addWidInt(
    KOPrefs::instance()->autoSaveIntervalItem(), saveGroupBox );
  connect( autoSave->checkBox(), SIGNAL( toggled( bool ) ),
           autoSaveInterval->label(), SLOT( setEnabled( bool ) ) );
  connect( autoSave->checkBox(), SIGNAL( toggled( bool ) ),
           autoSaveInterval->spinBox(), SLOT( setEnabled( bool ) ) );
  intervalLayout->addWidget( autoSaveInterval->label() );
  intervalLayout->addWidget( autoSaveInterval->spinBox() );

  addWidBool( KOPrefs::instance()->confirmItem(), topFrame );
  addWidRadios( KOPrefs::instance()->destinationItem(), topFrame);

  topTopLayout->addStretch( 1 );

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
    return new KOPrefsDialogMain( KOGlobals::self()->componentData(), parent /*, "kcmkorganizermain"*/ );
  }
}


class KOPrefsDialogTime : public KPrefsModule
{
  public:
    KOPrefsDialogTime( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );

      QWidget *topFrame = new QWidget( this );
      topTopLayout->addWidget( topFrame );

      QGridLayout *topLayout = new QGridLayout(topFrame);
      topLayout->setSpacing( KDialog::spacingHint() );

      KHBox *timeZoneBox = new KHBox( topFrame );
      topLayout->addWidget( timeZoneBox, 0, 0, 1, 2 );

      QLabel *timeZoneLabel = new QLabel( i18n("Time zone:"), timeZoneBox );
      QString whatsThis = i18n( "Select your time zone from the list of "
                                "locations on this drop down box. If your city "
                                "is not listed, select one which shares the "
                                "same time zone. KOrganizer will automatically "
                                "adjust for daylight savings." );
      timeZoneLabel->setWhatsThis( whatsThis );
      mTimeZoneCombo = new QComboBox( timeZoneBox );

      connect( mTimeZoneCombo, SIGNAL( activated( int ) ),
               SLOT( slotWidChanged() ) );

      QString sCurrentlySet(i18n("Unknown"));
      const KTimeZone *zone = KSystemTimeZones::local();
      if (zone)
        sCurrentlySet = zone->name();
      // Read all system time zones
      QStringList list;
      const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
      for (KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it) {
        list.append(i18n(it.key().toUtf8()));
        tzonenames << it.key();
      }
      list.sort();
      mTimeZoneCombo->addItem(i18n("[No selection]"));
      mTimeZoneCombo->addItems( list );


      // find the currently set time zone and select it
      int nCurrentlySet = 0;
      for ( int i = 0; i < mTimeZoneCombo->count(); ++i )
        {
          if (mTimeZoneCombo->itemText(i) == sCurrentlySet)
            {
             nCurrentlySet = i;
             break;
            }
        }
      mTimeZoneCombo->setCurrentIndex(nCurrentlySet);
      mTimeZoneCombo->setWhatsThis( whatsThis );

      // holiday region selection
      KHBox *holidayRegBox = new KHBox( topFrame );
      topLayout->addWidget( holidayRegBox, 1, 0, 1, 2 );

      QLabel *holidayLabel = new QLabel( i18n( "Use holiday region:" ), holidayRegBox );
      whatsThis = i18n( "Select from which region you want to use the "
                        "holidays here. Defined holidays are shown as "
                        "non-working days in the date navigator, the "
                        "agenda view, etc." );
      holidayLabel->setWhatsThis( whatsThis );

      mHolidayCombo = new QComboBox( holidayRegBox );
      connect( mHolidayCombo, SIGNAL( activated( int ) ),
               SLOT( slotWidChanged() ) );

      mHolidayCombo->setWhatsThis( whatsThis );

      QString currentHolidayName;
      QStringList holidayList;
      QStringList countryList = KHolidays::locations();
      QStringList::ConstIterator it;

      for ( it = countryList.begin(); it != countryList.end(); ++it ) {
        QString countryFile = KStandardDirs::locate( "locale",
                                      "l10n/" + (*it) + "/entry.desktop" );
        QString regionName;
        if ( !countryFile.isEmpty() ) {
          KConfig _cfg( countryFile, KConfig::OnlyLocal );
          KConfigGroup cfg(&_cfg, "KCM Locale" );
          regionName = cfg.readEntry( "Name" );
        }
        if (regionName.isEmpty()) regionName = (*it);

        holidayList << regionName;
        mRegionMap[regionName] = (*it); //store region for saving to config file

        if ( KOGlobals::self()->holidays()
             && ((*it) == KOGlobals::self()->holidays()->location()) )
          currentHolidayName = regionName;
      }
      holidayList.sort();
      holidayList.push_front( i18n("(None)") );  //be able to disable holidays

      mHolidayCombo->addItems( holidayList );

      for (int i=0; i < mHolidayCombo->count(); ++i) {
        if ( mHolidayCombo->itemText(i) == currentHolidayName ) {
          mHolidayCombo->setCurrentIndex(i);
          break;
        }
      }

      KPrefsWidTime *dayBegins =
        addWidTime( KOPrefs::instance()->dayBeginsItem(), topFrame );
      topLayout->addWidget( dayBegins->label(), 2, 0 );
      topLayout->addWidget( dayBegins->timeEdit(), 2, 1 );

      KPrefsWidTime *defaultTime =
        addWidTime( KOPrefs::instance()->startTimeItem(), topFrame );
      topLayout->addWidget( defaultTime->label(), 3, 0);
      topLayout->addWidget( defaultTime->timeEdit(), 3, 1);

      KPrefsWidDuration *defaultDuration =
        addWidDuration( KOPrefs::instance()->defaultDurationItem(), topFrame );
      topLayout->addWidget( defaultDuration->label(), 4, 0 );
      topLayout->addWidget( defaultDuration->timeEdit(), 4, 1 );

      QStringList alarmList;
      alarmList << i18n( "1 minute" ) << i18n( "5 minutes" )
                << i18n( "10 minutes" ) << i18n( "15 minutes" )
                << i18n( "30 minutes" );
      QLabel *alarmLabel = new QLabel( i18n( "Default reminder time:" ), topFrame );
      topLayout->addWidget( alarmLabel, 5, 0 );
      alarmLabel->setWhatsThis(
                       i18n( "Enter the default reminder time here." ) );
      mAlarmTimeCombo = new QComboBox( topFrame );
      mAlarmTimeCombo->setWhatsThis(
                       i18n( "Enter the default reminder time here." ) );
      connect( mAlarmTimeCombo, SIGNAL( activated( int ) ),
               SLOT( slotWidChanged() ) );
      mAlarmTimeCombo->addItems( alarmList );
      topLayout->addWidget( mAlarmTimeCombo, 5, 1 );


      QGroupBox *workingHoursGroupBox = new QGroupBox( i18n("Working Hours"), topFrame);
      topLayout->addWidget( workingHoursGroupBox, 6, 0, 1, 2 );

      QBoxLayout *workingHoursLayout = new QVBoxLayout( workingHoursGroupBox );

      QBoxLayout *workDaysLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workDaysLayout );

      // Respect start of week setting
      int weekStart=KGlobal::locale()->weekStartDay();
      for ( int i = 0; i < 7; ++i ) {
        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
        QString weekDayName = calSys->weekDayName( (i + weekStart + 6)%7 + 1, true );
        if ( KOPrefs::instance()->mCompactDialogs ) {
          weekDayName = weekDayName.left( 1 );
        }
        int index = ( i + weekStart + 6 ) % 7;
        mWorkDays[ index ] = new QCheckBox( weekDayName );
        mWorkDays[ index ]->setWhatsThis(
                         i18n( "Check this box to make KOrganizer mark the "
                               "working hours for this day of the week. "
                               "If this is a work day for you, check "
                               "this box, or the working hours will not be "
                               "marked with color." ) );

        connect( mWorkDays[ index ], SIGNAL( stateChanged( int ) ),
               SLOT( slotWidChanged() ) );

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

      topLayout->setRowStretch(7,1);

      load();
    }

  protected:
    void usrReadConfig()
    {
      const KTimeZone *tz = KOPrefs::instance()->timeSpec().timeZone();
      if (tz)
        setCombo( mTimeZoneCombo, i18n( tz->name().toUtf8() ) );

      mAlarmTimeCombo->setCurrentIndex( KOPrefs::instance()->mAlarmTime );
      for ( int i = 0; i < 7; ++i ) {
        mWorkDays[i]->setChecked( (1<<i) & (KOPrefs::instance()->mWorkWeekMask) );
      }
    }

    void usrWriteConfig()
    {
      // Find untranslated selected zone
      QString selectedZone = mTimeZoneCombo->currentText();
      QStringList::Iterator tz;
      for ( tz = tzonenames.begin(); tz != tzonenames.end(); ++tz )
        if (selectedZone == i18n((*tz).toUtf8())) {
          selectedZone = *tz;
          break;
        }
      const KTimeZone* zone = KSystemTimeZones::zone(selectedZone);
      if (zone)
        KOPrefs::instance()->setTimeSpec(zone);

      KOPrefs::instance()->mHolidays = ( mHolidayCombo->currentIndex() == 0 ) ?  // (None)
                                       QString() :
                                       mRegionMap[mHolidayCombo->currentText()];

      KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentIndex();
      int mask = 0;
      for ( int i = 0; i < 7; ++i ) {
        if (mWorkDays[i]->isChecked()) mask = mask | (1<<i);
      }
      KOPrefs::instance()->mWorkWeekMask = mask;
      KOPrefs::instance()->writeConfig();
    }

    void setCombo( QComboBox *combo, const QString &text,
                   const QStringList *tags = 0 )
    {
      if (tags) {
        int i = tags->indexOf(text);
        if (i > 0) combo->setCurrentIndex(i);
      } else {
        for(int i=0;i<combo->count();++i) {
          if (combo->itemText(i) == text) {
            combo->setCurrentIndex(i);
            break;
          }
        }
      }
    }

  private:
    QComboBox    *mTimeZoneCombo;
    QStringList   tzonenames;
    QComboBox    *mHolidayCombo;
    QMap<QString,QString> mRegionMap;
    QComboBox    *mAlarmTimeCombo;
    QCheckBox    *mWorkDays[7];
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigtime( QWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalog( "timezones" );
    return new KOPrefsDialogTime( KOGlobals::self()->componentData(), parent /*, "kcmkorganizertime"*/ );
  }
}


class KOPrefsDialogViews : public KPrefsModule
{
  public:
    KOPrefsDialogViews( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );

      QWidget *topFrame = new QWidget( this );
      topTopLayout->addWidget( topFrame );

      QBoxLayout *topLayout = new QVBoxLayout( topFrame );
      topLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidBool *enableToolTips =
          addWidBool( KOPrefs::instance()->enableToolTipsItem(), topFrame );
      topLayout->addWidget( enableToolTips->checkBox() );

      KPrefsWidBool *showTodosAgenda =
          addWidBool( KOPrefs::instance()->showAllDayTodoItem(), topFrame );
      topLayout->addWidget( showTodosAgenda->checkBox() );


      QGroupBox *dateNavGroup = new QGroupBox( i18n("Date Navigator"), topFrame );
      topLayout->addWidget( dateNavGroup );

      QBoxLayout *dateNavLayout = new QVBoxLayout( dateNavGroup );
      
      dateNavLayout->addWidget( addWidBool(
        KOPrefs::instance()->dailyRecurItem() )->checkBox() );
      dateNavLayout->addWidget( addWidBool(
        KOPrefs::instance()->weeklyRecurItem() )->checkBox() );


      QGroupBox *agendaGroup = new QGroupBox( i18n("Agenda View"), topFrame );
      topLayout->addWidget( agendaGroup );

      QBoxLayout *agendaLayout = new QVBoxLayout( agendaGroup );

      
      QBoxLayout *hourSizeLayout = new QHBoxLayout;
      agendaLayout->addLayout( hourSizeLayout );
      
      KPrefsWidInt *hourSize =
          addWidInt( KOPrefs::instance()->hourSizeItem() );
      hourSize->spinBox()->setSuffix(i18nc("suffix in the hour size spin box", " pixel"));

      hourSizeLayout->addWidget( hourSize->label() );
      hourSizeLayout->addWidget( hourSize->spinBox() );
      hourSizeLayout->addStretch( 1 );


      QBoxLayout *nextDaysLayout = new QHBoxLayout;
      agendaLayout->addLayout( nextDaysLayout );
      
      KPrefsWidInt *nextDays =
        addWidInt( KOPrefs::instance()->nextXDaysItem() );
      nextDays->spinBox()->setSuffix(i18nc("suffix in the N days spin box", " days"));

      nextDaysLayout->addWidget( nextDays->label() );
      nextDaysLayout->addWidget( nextDays->spinBox() );
      nextDaysLayout->addStretch( 1 );


      KPrefsWidBool *marcusBainsEnabled =
          addWidBool( KOPrefs::instance()->marcusBainsEnabledItem() );
      agendaLayout->addWidget( marcusBainsEnabled->checkBox() );

      KPrefsWidBool *marcusBainsShowSeconds =
          addWidBool( KOPrefs::instance()->marcusBainsShowSecondsItem() );
      connect( marcusBainsEnabled->checkBox(), SIGNAL( toggled( bool ) ),
               marcusBainsShowSeconds->checkBox(), SLOT( setEnabled( bool ) ) );
      agendaLayout->addWidget( marcusBainsShowSeconds->checkBox() );

      agendaLayout->addWidget( addWidBool(
        KOPrefs::instance()->selectionStartsEditorItem() )->checkBox() );

      agendaLayout->addWidget( addWidBool(
        KOPrefs::instance()->agendaViewUsesResourceColorItem() )->checkBox() );


      QGroupBox *monthGroup = new QGroupBox( i18n("Month View"), topFrame );
      topLayout->addWidget( monthGroup );
      
      QBoxLayout *monthLayout = new QVBoxLayout( monthGroup );
      
      monthLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->enableMonthScrollItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->fullViewMonthItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->monthViewUsesCategoryColorItem() )->checkBox() );
      monthLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->monthViewUsesResourceColorItem() )->checkBox() );


      QGroupBox *todoGroup = new QGroupBox( i18n("To-do View"), topFrame );
      topLayout->addWidget( todoGroup );

      QBoxLayout *todoLayout = new QVBoxLayout( todoGroup );
      
      todoLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->fullViewTodoItem() )->checkBox() );
      todoLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->useSplitListViewsItem() )->checkBox() );
      todoLayout->addWidget(
        addWidBool(
          KOPrefs::instance()->recordTodosInJournalsItem() )->checkBox() );

      topLayout->addStretch( 1 );

      load();
    }
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigviews( QWidget *parent, const char * )
  {
    return new KOPrefsDialogViews( KOGlobals::self()->componentData(), parent /*, "kcmkorganizerviews"*/ );
  }
}


class KOPrefsDialogFonts : public KPrefsModule
{
  public:
    KOPrefsDialogFonts( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );

      QWidget *topFrame = new QWidget( this );
      topTopLayout->addWidget( topFrame );

      QGridLayout *topLayout = new QGridLayout(topFrame);
      topLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidFont *timeBarFont =
          addWidFont( KOPrefs::instance()->timeBarFontItem(), topFrame,
                      KGlobal::locale()->formatTime( QTime( 12, 34 ) ) );
      topLayout->addWidget(timeBarFont->label(),0,0);
      topLayout->addWidget(timeBarFont->preview(),0,1);
      topLayout->addWidget(timeBarFont->button(),0,2);

      KPrefsWidFont *monthViewFont =
          addWidFont( KOPrefs::instance()->monthViewFontItem(), topFrame,
                      KGlobal::locale()->formatTime(QTime(12,34)) + ' ' +
                      i18n("Event text") );

      topLayout->addWidget(monthViewFont->label(),1,0);
      topLayout->addWidget(monthViewFont->preview(),1,1);
      topLayout->addWidget(monthViewFont->button(),1,2);

      KPrefsWidFont *agendaViewFont =
          addWidFont( KOPrefs::instance()->agendaViewFontItem(),
                      topFrame, i18n("Event text") );
      topLayout->addWidget(agendaViewFont->label(),2,0);
      topLayout->addWidget(agendaViewFont->preview(),2,1);
      topLayout->addWidget(agendaViewFont->button(),2,2);

      KPrefsWidFont *marcusBainsFont =
          addWidFont( KOPrefs::instance()->marcusBainsFontItem(), topFrame,
                      KGlobal::locale()->formatTime( QTime( 12, 34, 23 ) ) );
      topLayout->addWidget(marcusBainsFont->label(),3,0);
      topLayout->addWidget(marcusBainsFont->preview(),3,1);
      topLayout->addWidget(marcusBainsFont->button(),3,2);

      topLayout->setColumnStretch(1,1);
      topLayout->setRowStretch(4,1);

      load();
    }
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigfonts( QWidget *parent, const char * )
  {
    return new KOPrefsDialogFonts( KOGlobals::self()->componentData(), parent /*, "kcmkorganizerfonts"*/ );
  }
}


KOPrefsDialogColors::KOPrefsDialogColors( const KComponentData &inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout(topFrame);
  topLayout->setSpacing( KDialog::spacingHint() );

  // Holiday Color
  KPrefsWidColor *holidayColor =
      addWidColor( KOPrefs::instance()->holidayColorItem(), topFrame );
  topLayout->addWidget(holidayColor->label(),0,0);
  topLayout->addWidget(holidayColor->button(),0,1);

  // Highlight Color
  KPrefsWidColor *highlightColor =
      addWidColor( KOPrefs::instance()->highlightColorItem(), topFrame );
  topLayout->addWidget(highlightColor->label(),1,0);
  topLayout->addWidget(highlightColor->button(),1,1);

  KPrefsWidColor *eventColor =
      addWidColor( KOPrefs::instance()->eventColorItem(), topFrame );
  topLayout->addWidget(eventColor->label(),2,0);
  topLayout->addWidget(eventColor->button(),2,1);

  // agenda view background color
  KPrefsWidColor *agendaBgColor =
      addWidColor( KOPrefs::instance()->agendaBgColorItem(), topFrame );
  topLayout->addWidget(agendaBgColor->label(),3,0);
  topLayout->addWidget(agendaBgColor->button(),3,1);

  // working hours color
  KPrefsWidColor *workingHoursColor =
      addWidColor( KOPrefs::instance()->workingHoursColorItem(), topFrame );
  topLayout->addWidget(workingHoursColor->label(),4,0);
  topLayout->addWidget(workingHoursColor->button(),4,1);

  // Todo due today color
  KPrefsWidColor *todoDueTodayColor =
      addWidColor( KOPrefs::instance()->todoDueTodayColorItem(), topFrame );
  topLayout->addWidget(todoDueTodayColor->label(),5,0);
  topLayout->addWidget(todoDueTodayColor->button(),5,1);

  // Todo overdue color
  KPrefsWidColor *todoOverdueColor =
      addWidColor( KOPrefs::instance()->todoOverdueColorItem(), topFrame );
  topLayout->addWidget(todoOverdueColor->label(),6,0);
  topLayout->addWidget(todoOverdueColor->button(),6,1);

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox( i18n("Categories"), topFrame );
  topLayout->addWidget( categoryGroup, 7, 0, 1, 2 );

  QBoxLayout *categoryLayout = new QHBoxLayout;
  categoryGroup->setLayout( categoryLayout );

  mCategoryCombo = new QComboBox(categoryGroup);
  mCategoryCombo->addItems( KOPrefs::instance()->mCustomCategories );
  mCategoryCombo->setWhatsThis(
                   i18n( "Select here the event category you want to modify. "
                         "You can change the selected category color using "
                         "the button below." ) );
  connect(mCategoryCombo,SIGNAL(activated(int)),SLOT(updateCategoryColor()));
  categoryLayout->addWidget( mCategoryCombo );

  mCategoryButton = new KColorButton(categoryGroup);
  mCategoryButton->setWhatsThis(
                   i18n( "Choose here the color of the event category selected "
                         "using the combo box above." ) );
  connect(mCategoryButton,SIGNAL(changed(const QColor &)),SLOT(setCategoryColor()));
  categoryLayout->addWidget( mCategoryButton );

  updateCategoryColor();

  // resources colors
  QGroupBox *resourceGroup = new QGroupBox( i18n("Resources"), topFrame );
  topLayout->addWidget( resourceGroup, 8, 0, 1, 2 );

  QBoxLayout *resourceLayout = new QHBoxLayout;
  resourceGroup->setLayout( resourceLayout );

  mResourceCombo = new QComboBox(resourceGroup);
  mResourceCombo->setWhatsThis(
                   i18n( "Select here resource you want to modify. "
                         "You can change the selected resource color using "
                         "the button below." ) );
  connect(mResourceCombo,SIGNAL(activated(int)),SLOT(updateResourceColor()));
  resourceLayout->addWidget( mResourceCombo );

  mResourceButton = new KColorButton(resourceGroup);
  mResourceButton->setWhatsThis(
                   i18n( "Choose here the color of the resource selected "
                         "using the combo box above." ) );
  connect(mResourceButton,SIGNAL(changed(const QColor &)),SLOT(setResourceColor()));
  resourceLayout->addWidget( mResourceButton );

  updateResources();

  topLayout->setRowStretch(9,1);

  load();
}

void KOPrefsDialogColors::usrWriteConfig()
{
  QHash<QString, QColor>::const_iterator i = mCategoryDict.constBegin();
  while (i != mCategoryDict.constEnd()) {
    KOPrefs::instance()->setCategoryColor(i.key(), i.value() );
    ++i;
  }
    
  i = mResourceDict.constBegin();
  while (i != mResourceDict.constEnd()) {
    KOPrefs::instance()->setResourceColor(i.key(), i.value() );
    ++i;
  }
}

void KOPrefsDialogColors::usrReadConfig()
{
  updateCategories();
  updateResources();
}

void KOPrefsDialogColors::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->addItems( KOPrefs::instance()->mCustomCategories );
  updateCategoryColor();
}

void KOPrefsDialogColors::setCategoryColor()
{
  mCategoryDict.insert( mCategoryCombo->currentText(), mCategoryButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColors::updateCategoryColor()
{
  QString cat = mCategoryCombo->currentText();
  QColor color = mCategoryDict.value( cat  );
  if ( !color.isValid() ) {
    color = KOPrefs::instance()->categoryColor(cat);
  }
  if ( color.isValid() ) {
    mCategoryButton->setColor(color);
  }
}

void KOPrefsDialogColors::updateResources()
{
  mResourceCombo->clear();
  mResourceIdentifier.clear();
  kDebug( 5850) << "KOPrefsDialogColors::updateResources()" << endl;

  KCal::CalendarResourceManager *manager = KOrg::StdCalendar::self()->resourceManager();

  kDebug(5850) << "Loading Calendar resources...:" << endl;
  KCal::CalendarResourceManager::Iterator it;
  for( it = manager->begin(); it != manager->end(); ++it ) {
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

void KOPrefsDialogColors::setResourceColor()
{
  kDebug( 5850) << "KOPrefsDialogColors::setResorceColor()" << endl;

  mResourceDict.insert( mResourceIdentifier[mResourceCombo->currentIndex()],
    mResourceButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColors::updateResourceColor()
{
  kDebug( 5850 ) << "KOPrefsDialogColors::updateResourceColor()" << endl;
  QString res= mResourceIdentifier[mResourceCombo->currentIndex()];
  QColor color = mCategoryDict.value( res );
  if( color.isValid() )  {
    color = KOPrefs::instance()->resourceColor( res );
  }
  if( color.isValid() ) {
    mResourceButton->setColor( color );
  }
}
extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigcolors( QWidget *parent, const char * )
  {
    return new KOPrefsDialogColors( KOGlobals::self()->componentData(), parent /*, "kcmkorganizercolors"*/ );
  }
}


KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout(topFrame);
  topLayout->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *useGroupwareBool =
      addWidBool( KOPrefs::instance()->useGroupwareCommunicationItem(),
      topFrame );
  topLayout->addWidget( useGroupwareBool->checkBox(), 0, 0, 1, 2 );
  // FIXME: This radio button should only be available when KMail is chosen
//   connect(thekmailradiobuttonupthere,SIGNAL(toggled(bool)),
//           useGroupwareBool->checkBox(), SLOT(enabled(bool)));

  KPrefsWidBool *bcc =
      addWidBool( KOPrefs::instance()->bccItem(), topFrame );
  topLayout->addWidget( bcc->checkBox(), 1,0, 1, 2 );

  KPrefsWidRadios *mailClientGroup =
      addWidRadios( KOPrefs::instance()->mailClientItem(), topFrame );
  topLayout->addWidget( mailClientGroup->groupBox(), 2, 0, 1, 2 );


#if 0
  KPrefsWidRadios *schedulerGroup =
      addWidRadios(i18n("Scheduler Mail Client"),KOPrefs::instance()->mIMIPScheduler,
                   topFrame);
  schedulerGroup->addRadio("Dummy"); // Only for debugging
  schedulerGroup->addRadio(i18n("Mail client"));

  topLayout->addWidget( schedulerGroup->groupBox(), 0, 0, 1, 2 );
#endif

  QLabel *aMailsLabel = new QLabel(i18n("Additional email addresses:"),topFrame);
  QString whatsThis = i18n( "Add, edit or remove additional e-mails addresses "
                            "here. These email addresses are the ones you "
                            "have in addition to the one set in personal "
                            "preferences. If you are an attendee of one event, "
                            "but use another email address there, you need to "
                            "list this address here so KOrganizer can "
                            "recognize it as yours." );
  aMailsLabel->setWhatsThis( whatsThis );
  topLayout->addWidget( aMailsLabel, 3, 0, 1, 2 );
  mAMails = new Q3ListView(topFrame);
  mAMails->setWhatsThis( whatsThis );

  mAMails->addColumn(i18n("Email"),300);
  topLayout->addWidget( mAMails, 4, 0, 1, 2 );

  QLabel *aEmailsEditLabel = new QLabel(i18n("Additional email address:"),topFrame);
  whatsThis = i18n( "Edit additional e-mails addresses here. To edit an "
                    "address select it from the list above "
                    "or press the \"New\" button below. These email "
                    "addresses are the ones you have in addition to the "
                    "one set in personal preferences." );
  aEmailsEditLabel->setWhatsThis( whatsThis );
  topLayout->addWidget(aEmailsEditLabel,5,0);
  aEmailsEdit = new QLineEdit(topFrame);
  aEmailsEdit->setWhatsThis( whatsThis );
  aEmailsEdit->setEnabled(false);
  topLayout->addWidget(aEmailsEdit,5,1);

  QPushButton *add = new QPushButton( i18n("New"), topFrame );
  add->setObjectName( "new" );
  whatsThis = i18n( "Press this button to add a new entry to the "
                    "additional e-mail addresses list. Use the edit "
                    "box above to edit the new entry." );
  add->setWhatsThis( whatsThis );
  topLayout->addWidget(add,6,0);
  QPushButton *del = new QPushButton( i18n("Remove"), topFrame );
  del->setObjectName( "remove" );
  del->setWhatsThis( whatsThis );
  topLayout->addWidget(del,6,1);

  //topLayout->setRowStretch(2,1);
  connect(add, SIGNAL( clicked() ), this, SLOT(addItem()) );
  connect(del, SIGNAL( clicked() ), this, SLOT(removeItem()) );
  connect(aEmailsEdit,SIGNAL( textChanged(const QString&) ), this,SLOT(updateItem()));
  connect(mAMails,SIGNAL(selectionChanged(Q3ListViewItem *)),SLOT(updateInput()));

  load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
  mAMails->clear();
  for ( QStringList::Iterator it = KOPrefs::instance()->mAdditionalMails.begin();
            it != KOPrefs::instance()->mAdditionalMails.end(); ++it ) {
    Q3ListViewItem *item = new Q3ListViewItem(mAMails);
    item->setText(0,*it);
    mAMails->insertItem(item);
  }
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mAdditionalMails.clear();
  Q3ListViewItem *item;
  item = mAMails->firstChild();
  while (item)
  {
    KOPrefs::instance()->mAdditionalMails.append( item->text(0) );
    item = item->nextSibling();
  }
}

void KOPrefsDialogGroupScheduling::addItem()
{
  aEmailsEdit->setEnabled(true);
  Q3ListViewItem *item = new Q3ListViewItem(mAMails);
  mAMails->insertItem(item);
  mAMails->setSelected(item,true);
  aEmailsEdit->setText(i18n("(EmptyEmail)"));
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::removeItem()
{
  Q3ListViewItem *item;
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
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::updateItem()
{
  Q3ListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  item->setText(0,aEmailsEdit->text());
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::updateInput()
{
  Q3ListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  aEmailsEdit->setEnabled(true);
  aEmailsEdit->setText(item->text(0));
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfiggroupscheduling( QWidget *parent,
                                                     const char * )
  {
    return new KOPrefsDialogGroupScheduling( KOGlobals::self()->componentData(), parent /*,
                                             "kcmkorganizergroupscheduling"*/ );
  }
}


KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  mGroupwarePage = new Ui::KOGroupwarePrefsPage();
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "KOGrouparePrefsPage" );
  
  mGroupwarePage->setupUi( widget );
  
      // signals and slots connections
  connect(mGroupwarePage->publishDays, SIGNAL(valueChanged(int)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishUrl, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishUser, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishPassword, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishSavePassword, SIGNAL(toggled(bool)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->retrieveEnable, SIGNAL(toggled(bool)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->retrieveUser, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->retrievePassword, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->retrieveSavePassword, SIGNAL(toggled(bool)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->retrieveUrl, SIGNAL(textChanged(const QString&)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishDelay, SIGNAL(valueChanged(int)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->fullDomainRetrieval, SIGNAL(toggled(bool)), SLOT(slotWidChanged()));
  connect(mGroupwarePage->publishEnable, SIGNAL(toggled(bool)), SLOT(slotWidChanged()));

  ( new QVBoxLayout( this ) )->addWidget( widget );

  load();
}

KOPrefsDialogGroupwareScheduling::~KOPrefsDialogGroupwareScheduling() 
{
  delete mGroupwarePage;
}

void KOPrefsDialogGroupwareScheduling::usrReadConfig()
{
  mGroupwarePage->publishEnable->setChecked( KOPrefs::instance()->mFreeBusyPublishAuto );
  mGroupwarePage->publishDelay->setValue( KOPrefs::instance()->mFreeBusyPublishDelay );
  mGroupwarePage->publishDays->setValue( KOPrefs::instance()->mFreeBusyPublishDays );

  mGroupwarePage->publishUrl->setText( KOPrefs::instance()->mFreeBusyPublishUrl );
  mGroupwarePage->publishUser->setText( KOPrefs::instance()->mFreeBusyPublishUser );
  mGroupwarePage->publishPassword->setText( KOPrefs::instance()->mFreeBusyPublishPassword );
  mGroupwarePage->publishSavePassword->setChecked( KOPrefs::instance()->mFreeBusyPublishSavePassword );

  mGroupwarePage->retrieveEnable->setChecked( KOPrefs::instance()->mFreeBusyRetrieveAuto );
  mGroupwarePage->fullDomainRetrieval->setChecked( KOPrefs::instance()->mFreeBusyFullDomainRetrieval );
  mGroupwarePage->retrieveUrl->setText( KOPrefs::instance()->mFreeBusyRetrieveUrl );
  mGroupwarePage->retrieveUser->setText( KOPrefs::instance()->mFreeBusyRetrieveUser );
  mGroupwarePage->retrievePassword->setText( KOPrefs::instance()->mFreeBusyRetrievePassword );
  mGroupwarePage->retrieveSavePassword->setChecked( KOPrefs::instance()->mFreeBusyRetrieveSavePassword );
}

void KOPrefsDialogGroupwareScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mFreeBusyPublishAuto = mGroupwarePage->publishEnable->isChecked();
  KOPrefs::instance()->mFreeBusyPublishDelay = mGroupwarePage->publishDelay->value();
  KOPrefs::instance()->mFreeBusyPublishDays = mGroupwarePage->publishDays->value();

  KOPrefs::instance()->mFreeBusyPublishUrl = mGroupwarePage->publishUrl->text();
  KOPrefs::instance()->mFreeBusyPublishUser = mGroupwarePage->publishUser->text();
  KOPrefs::instance()->mFreeBusyPublishPassword = mGroupwarePage->publishPassword->text();
  KOPrefs::instance()->mFreeBusyPublishSavePassword = mGroupwarePage->publishSavePassword->isChecked();

  KOPrefs::instance()->mFreeBusyRetrieveAuto = mGroupwarePage->retrieveEnable->isChecked();
  KOPrefs::instance()->mFreeBusyFullDomainRetrieval = mGroupwarePage->fullDomainRetrieval->isChecked();
  KOPrefs::instance()->mFreeBusyRetrieveUrl = mGroupwarePage->retrieveUrl->text();
  KOPrefs::instance()->mFreeBusyRetrieveUser = mGroupwarePage->retrieveUser->text();
  KOPrefs::instance()->mFreeBusyRetrievePassword = mGroupwarePage->retrievePassword->text();
  KOPrefs::instance()->mFreeBusyRetrieveSavePassword = mGroupwarePage->retrieveSavePassword->isChecked();
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigfreebusy( QWidget *parent, const char * )
  {
    return new KOPrefsDialogGroupwareScheduling( KOGlobals::self()->componentData(), parent /*,
                                                 "kcmkorganizerfreebusy"*/ );
  }
}



class PluginItem : public Q3CheckListItem {
  public:
    PluginItem( Q3ListView *parent, KService::Ptr service ) :
      Q3CheckListItem( parent, service->name(), Q3CheckListItem::CheckBox ), mService( service )
    {}
    KService::Ptr service() { return mService; }
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

  mListView = new Q3ListView( topFrame );
  mListView->addColumn( i18n("Name") );
  mListView->setResizeMode( Q3ListView::LastColumn );
  topLayout->addWidget( mListView );

  mDescription = new QLabel( topFrame );
  mDescription->setAlignment( Qt::AlignVCenter );
  mDescription->setWordWrap( true );
  mDescription->setFrameShape( QLabel::Panel );
  mDescription->setFrameShadow( QLabel::Sunken );
  mDescription->setMinimumSize( QSize( 0, 55 ) );
  QSizePolicy policy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0 );
  policy.setHorizontalStretch( 0 );
  policy.setVerticalStretch( 0 );
  policy.setHeightForWidth( mDescription->sizePolicy().hasHeightForWidth() );
  mDescription->setSizePolicy( policy );
  topLayout->addWidget( mDescription );


  QWidget *buttonRow = new QWidget( topFrame );
  QBoxLayout *buttonRowLayout = new QHBoxLayout( buttonRow );
  mConfigureButton = new KPushButton( KGuiItem( i18n("Configure &Plugin..."),
      "configure", QString(), i18n("This button allows you to configure"
      " the plugin that you have selected in the list above") ), buttonRow );

  buttonRowLayout->addWidget( mConfigureButton );
  buttonRowLayout->addItem( new QSpacerItem(1, 1,  QSizePolicy::Expanding) );
  topLayout->addWidget( buttonRow );
  connect( mConfigureButton, SIGNAL( clicked() ), SLOT( configure() ) );

  connect( mListView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
           SLOT( selectionChanged( Q3ListViewItem* ) ) );
  connect( mListView, SIGNAL( clicked( Q3ListViewItem* ) ),
           SLOT( slotWidChanged() ) );

  load();

  selectionChanged( 0 );
}

void KOPrefsDialogPlugins::usrReadConfig()
{
  mListView->clear();
  KService::List plugins = KOCore::self()->availablePlugins();
  plugins += KOCore::self()->availableParts();

  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  KService::List::ConstIterator it;
  for( it = plugins.begin(); it != plugins.end(); ++it ) {
    Q3CheckListItem *item = new PluginItem( mListView, *it );
    if ( selectedPlugins.contains( (*it)->desktopEntryName() ) ) {
      item->setOn( true );
    }
  }
}

void KOPrefsDialogPlugins::usrWriteConfig()
{
  QStringList selectedPlugins;

  PluginItem *item = static_cast<PluginItem *>( mListView->firstChild() );
  while( item ) {
    if( item->isOn() ) {
      selectedPlugins.append( item->service()->desktopEntryName() );
    }
    item = static_cast<PluginItem *>( item->nextSibling() );
  }
  KOPrefs::instance()->mSelectedPlugins = selectedPlugins;
}

void KOPrefsDialogPlugins::configure()
{
  PluginItem *item = static_cast<PluginItem *>( mListView->selectedItem() );
  if ( !item ) return;

  KOrg::Plugin *plugin = KOCore::self()->loadPlugin( item->service() );

  if ( plugin ) {
    plugin->configure( this );
    delete plugin;
  } else {
    KMessageBox::sorry( this, i18n( "Unable to configure this plugin" ),
                        "PluginConfigUnable" );
  }
}

void KOPrefsDialogPlugins::selectionChanged( Q3ListViewItem *i )
{
  PluginItem *item = dynamic_cast<PluginItem*>( i );
  if ( !item ) {
    mConfigureButton->setEnabled( false );
    mDescription->setText( QString() );
    return;
  }

  QVariant variant = item->service()->property( "X-KDE-KOrganizer-HasSettings" );

  bool hasSettings = true;
  if ( variant.isValid() )
    hasSettings = variant.toBool();

  mDescription->setText( item->service()->comment() );
  mConfigureButton->setEnabled( hasSettings );

  slotWidChanged();
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigplugins( QWidget *parent, const char * )
  {
    return new KOPrefsDialogPlugins( KOGlobals::self()->componentData(), parent /*,
                                     "kcmkorganizerplugins"*/ );
  }
}


extern "C"
{
  KDE_EXPORT KCModule *create_korgdesignerfields( QWidget *parent, const char * ) {
    return new KOPrefsDesignerFields( KOGlobals::self()->componentData(), parent /*, "kcmkorgdesignerfields"*/ );
  }
}

KOPrefsDesignerFields::KOPrefsDesignerFields( const KComponentData &inst, QWidget *parent )
  : KCMDesignerFields( inst, parent )
{
}

QString KOPrefsDesignerFields::localUiDir()
{
  QString dir = KStandardDirs::locateLocal( "data", "korganizer/designer/event/");
  kDebug() << "KOPrefsDesignerFields::localUiDir(): " << dir << endl;
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
