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
#include "kitemiconcheckcombo.h"
#include "kocore.h"
#include "koglobals.h"
#include "koprefs.h"
#include "ui_kogroupwareprefspage.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/categoryconfig.h>

#include <incidenceeditor-ng/globalsettings.h>
#include <widgets/tagwidgets.h>

#include <Akonadi/AgentFilterProxyModel>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentTypeDialog>
#include <Akonadi/CollectionComboBox>
#include <Akonadi/CollectionModel>
#include <akonadi/calendar/calendarsettings.h>

#include <KCalCore/Event>
#include <KCalCore/Journal>

#include <KHolidays/Holidays>

#include <Mailtransport/TransportManagementWidget>

#include <KCalendarSystem>
#include <KColorButton>
#include <KComboBox>
#include <KHBox>
#include <KIntSpinBox>
#include <KMessageBox>
#include <KService>
#include <KStandardDirs>
#include <KTabWidget>
#include <KTimeComboBox>
#include <KUrlRequester>
#include <KWindowSystem>

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QTimeEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

KOPrefsDialogMain::KOPrefsDialogMain( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );
  KTabWidget *tabWidget = new KTabWidget( this );
  topTopLayout->addWidget( tabWidget );

  // Personal Settings
  QWidget *personalFrame = new QWidget( this );
  QVBoxLayout *personalLayout = new QVBoxLayout( personalFrame );
  tabWidget->addTab( personalFrame, KIcon( QLatin1String("preferences-desktop-personal") ),
                     i18nc( "@title:tab personal settings", "Personal" ) );

  KPIM::KPrefsWidBool *emailControlCenter =
    addWidBool( CalendarSupport::KCalPrefs::instance()->emailControlCenterItem(), personalFrame );
  connect( emailControlCenter->checkBox(), SIGNAL(toggled(bool)), SLOT(toggleEmailSettings(bool)) );
  personalLayout->addWidget( emailControlCenter->checkBox() );

  mUserEmailSettings =
    new QGroupBox( i18nc( "@title:group email settings", "Email Settings" ), personalFrame );

  personalLayout->addWidget( mUserEmailSettings );
  QFormLayout *emailSettingsLayout = new QFormLayout( mUserEmailSettings );
  KPIM::KPrefsWidString *s =
    addWidString( CalendarSupport::KCalPrefs::instance()->userNameItem(), mUserEmailSettings );
  emailSettingsLayout->addRow ( s->label(), s->lineEdit() );

  s=addWidString( CalendarSupport::KCalPrefs::instance()->userEmailItem(), mUserEmailSettings );
  emailSettingsLayout->addRow ( s->label(), s->lineEdit() );

  KPIM::KPrefsWidRadios *defaultEmailAttachMethod =
    addWidRadios(
      IncidenceEditorNG::GlobalSettings::self()->defaultEmailAttachMethodItem(), personalFrame );
  personalLayout->addWidget( defaultEmailAttachMethod->groupBox() );
  personalLayout->addStretch( 1 );

  // Save Settings
  QFrame *saveFrame = new QFrame( this );
  tabWidget->addTab( saveFrame, KIcon( QLatin1String("document-save") ),
                     i18nc( "@title:tab", "Save" ) );
  QVBoxLayout *saveLayout = new QVBoxLayout( saveFrame );

  QGroupBox *saveGroupBox =
    new QGroupBox( i18nc( "@title:group", "Exporting Calendar" ), saveFrame );
  saveLayout->addWidget( saveGroupBox );
  QVBoxLayout *saveGroupLayout = new QVBoxLayout;
  saveGroupBox->setLayout( saveGroupLayout );

  KPIM::KPrefsWidBool *autoExportHTML =
    addWidBool( KOPrefs::instance()->autoExportItem(), saveGroupBox );
  saveGroupLayout->addWidget( autoExportHTML->checkBox() );

  QBoxLayout *intervalLayout = new QHBoxLayout;
  saveGroupLayout->addLayout( intervalLayout );

  KPIM::KPrefsWidInt *autoExportInterval =
    addWidInt( KOPrefs::instance()->autoExportIntervalItem(), saveGroupBox );
  connect( autoExportHTML->checkBox(), SIGNAL(toggled(bool)),
           autoExportInterval->label(), SLOT(setEnabled(bool)) );
  connect( autoExportHTML->checkBox(), SIGNAL(toggled(bool)),
           autoExportInterval->spinBox(), SLOT(setEnabled(bool)) );
  intervalLayout->addWidget( autoExportInterval->label() );
  intervalLayout->addWidget( autoExportInterval->spinBox() );

  KPIM::KPrefsWidBool *confirmItem =
    addWidBool( KOPrefs::instance()->confirmItem(), saveFrame );
  saveLayout->addWidget( confirmItem->checkBox() );
  KPIM::KPrefsWidRadios *destinationItem =
    addWidRadios( KOPrefs::instance()->destinationItem(), saveFrame );

  saveLayout->addWidget( destinationItem->groupBox() );
  saveLayout->addStretch( 1 );

  // System Tray Settings
  QFrame *systrayFrame = new QFrame( this );
  QVBoxLayout *systrayLayout = new QVBoxLayout( systrayFrame );
  tabWidget->addTab( systrayFrame, KIcon( QLatin1String("preferences-other") ),
                     i18nc( "@title:tab systray settings", "System Tray" ) );

  QGroupBox *systrayGroupBox =
    new QGroupBox( i18nc( "@title:group", "Show/Hide Options" ), systrayFrame );
  systrayLayout->addWidget( systrayGroupBox );
  QVBoxLayout *systrayGroupLayout = new QVBoxLayout;
  systrayGroupBox->setLayout( systrayGroupLayout );

  KPIM::KPrefsWidBool *showReminderDaemonItem =
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

  //Calendar Account
  QFrame *calendarFrame = new QFrame( this );
  tabWidget->addTab( calendarFrame, KIcon( QLatin1String("office-calendar") ),
                     i18nc( "@title:tab calendar account settings", "Calendars" ) );

  mAccountsCalendar.setupUi( calendarFrame );

  mAccountsCalendar.vlay->setSpacing( KDialog::spacingHint() );
  mAccountsCalendar.vlay->setMargin( KDialog::marginHint() );

  mAccountsCalendar.mAccountList->agentFilterProxyModel()->
    addMimeTypeFilter( QLatin1String("text/calendar") );
  mAccountsCalendar.mAccountList->agentFilterProxyModel()->
    addCapabilityFilter( QLatin1String("Resource") ); // show only resources, no agents
  mAccountsCalendar.mFilterAccount->
    setProxy( mAccountsCalendar.mAccountList->agentFilterProxyModel() );

  connect( mAccountsCalendar.mAccountList->view()->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           SLOT(slotAccountSelected()));
  connect( mAccountsCalendar.mAccountList, SIGNAL(doubleClicked(Akonadi::AgentInstance)),
           this, SLOT(slotModifySelectedAccount()) );

  mAccountsCalendar.hlay->insertWidget( 0, mAccountsCalendar.mAccountList );

  connect( mAccountsCalendar.mAddAccountButton, SIGNAL(clicked()),
           this, SLOT(slotAddAccount()) );

  connect( mAccountsCalendar.mModifyAccountButton, SIGNAL(clicked()),
           this, SLOT(slotModifySelectedAccount()) );

  const bool hasSelection = !mAccountsCalendar.mAccountList->selectedAgentInstances().isEmpty();
  mAccountsCalendar.mModifyAccountButton->setEnabled( hasSelection );
  mAccountsCalendar.mRemoveAccountButton->setEnabled( hasSelection );

  connect( mAccountsCalendar.mRemoveAccountButton, SIGNAL(clicked()),
           this, SLOT(slotRemoveSelectedAccount()) );

  load();
}

void KOPrefsDialogMain::usrWriteConfig()
{
    KPIM::KPrefsModule::usrWriteConfig();
    IncidenceEditorNG::GlobalSettings::self()->writeConfig();
}

void KOPrefsDialogMain::slotAccountSelected()
{
  if ( mAccountsCalendar.mAccountList->selectedAgentInstances().isEmpty() ) {
    mAccountsCalendar.mModifyAccountButton->setEnabled( false );
    mAccountsCalendar.mRemoveAccountButton->setEnabled( false );
  } else {
    Akonadi::AgentInstance selectedAgent =
      mAccountsCalendar.mAccountList->selectedAgentInstances().first();

    mAccountsCalendar.mModifyAccountButton->setEnabled(
      !selectedAgent.type().capabilities().contains( QLatin1String( "NoConfig" ) ) );

    mAccountsCalendar.mRemoveAccountButton->setEnabled( true );
  }
}

void KOPrefsDialogMain::slotAddAccount()
{
  //TODO verify this dialog box. We can see note etc...
  Akonadi::AgentTypeDialog dlg( this );
  Akonadi::AgentFilterProxyModel *filter = dlg.agentFilterProxyModel();
  filter->addMimeTypeFilter( QLatin1String("text/calendar") );
  filter->addCapabilityFilter( QLatin1String("Resource") );
  if ( dlg.exec() ) {
    const Akonadi::AgentType agentType = dlg.agentType();

    if ( agentType.isValid() ) {

      Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( agentType, this );
      job->configure( this );
      job->start();
    }
  }
}

void KOPrefsDialogMain::slotModifySelectedAccount()
{
  Akonadi::AgentInstance instance = mAccountsCalendar.mAccountList->currentAgentInstance();
  if ( instance.isValid() ) {
    KWindowSystem::allowExternalProcessWindowActivation();
    instance.configure( this );
  }
}

void KOPrefsDialogMain::slotRemoveSelectedAccount()
{    
  const Akonadi::AgentInstance instance =  mAccountsCalendar.mAccountList->currentAgentInstance();
  if ( instance.isValid() ) {
    const int rc = KMessageBox::questionYesNo( this,
                                       i18n("Do you want to remove account '%1'?", instance.name()),
                                       i18n("Remove account?"));
    if ( rc == KMessageBox::No ) {
      return;
    }

    Akonadi::AgentManager::self()->removeInstance( instance );
  } else {
    return;
  }

  slotAccountSelected();
}

void KOPrefsDialogMain::toggleEmailSettings( bool on )
{
  mUserEmailSettings->setEnabled( !on );
/*  if (on) {
    KEMailSettings settings;
    mNameEdit->setText( settings.getSetting(KEMailSettings::RealName) );
    mEmailEdit->setText( settings.getSetting(KEMailSettings::EmailAddress) );
  } else {
    mNameEdit->setText( CalendarSupport::KCalPrefs::instance()->mName );
    mEmailEdit->setText( CalendarSupport::KCalPrefs::instance()->mEmail );
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

class KOPrefsDialogTime : public KPIM::KPrefsModule
{
  public:
    KOPrefsDialogTime( const KComponentData &inst, QWidget *parent )
      : KPIM::KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QVBoxLayout *layout = new QVBoxLayout( this );
      KTabWidget *tabWidget = new KTabWidget( this );
      layout->addWidget( tabWidget );

      QFrame *regionalPage = new QFrame( parent );
      tabWidget->addTab( regionalPage, KIcon( QLatin1String("flag") ),
                         i18nc( "@title:tab", "Regional" ) );

      QGridLayout *regionalLayout = new QGridLayout( regionalPage );
      regionalLayout->setSpacing( KDialog::spacingHint() );

      QGroupBox *datetimeGroupBox =
        new QGroupBox( i18nc( "@title:group", "General Time and Date" ), regionalPage );
      regionalLayout->addWidget( datetimeGroupBox, 0, 0 );

      QGridLayout *datetimeLayout = new QGridLayout( datetimeGroupBox );

      KPIM::KPrefsWidTime *dayBegins =
        addWidTime( KOPrefs::instance()->dayBeginsItem(), regionalPage );
      datetimeLayout->addWidget( dayBegins->label(), 1, 0 );
      datetimeLayout->addWidget( dayBegins->timeEdit(), 1, 1 );

      QGroupBox *holidaysGroupBox =
        new QGroupBox( i18nc( "@title:group", "Holidays" ), regionalPage );
      regionalLayout->addWidget( holidaysGroupBox, 1, 0 );

      QGridLayout *holidaysLayout = new QGridLayout( holidaysGroupBox );

      // holiday region selection
      KHBox *holidayRegBox = new KHBox( regionalPage );
      holidaysLayout->addWidget( holidayRegBox, 1, 0, 1, 2 );

      QLabel *holidayLabel = new QLabel(
        i18nc( "@label", "Use holiday region:" ), holidayRegBox );
      holidayLabel->setWhatsThis( KOPrefs::instance()->holidaysItem()->whatsThis() );

      mHolidayCombo = new KComboBox( holidayRegBox );
      connect( mHolidayCombo, SIGNAL(activated(int)), SLOT(slotWidChanged()) );

      mHolidayCombo->setWhatsThis( KOPrefs::instance()->holidaysItem()->whatsThis() );

      QStringList regions = KHolidays::HolidayRegion::regionCodes();
      QMap<QString, QString> regionsMap;

      foreach ( const QString & regionCode, regions ) {
        QString name = KHolidays::HolidayRegion::name( regionCode );
        QString languageName =
          KGlobal::locale()->languageCodeToName(
            KHolidays::HolidayRegion::languageCode( regionCode ) );
        QString label;
        if ( languageName.isEmpty() ) {
          label = name;
        } else {
          label = i18nc( "Holday region, region language", "%1 (%2)", name, languageName );
        }
        regionsMap.insert( label, regionCode );
      }

      mHolidayCombo->addItem( i18nc( "No holiday region", "None" ), QString() );
      QMapIterator<QString, QString> i( regionsMap );
      while ( i.hasNext() ) {
        i.next();
        mHolidayCombo->addItem( i.key(), i.value() );
      }

      if ( KOGlobals::self()->holidays() && KOGlobals::self()->holidays()->isValid() ) {
        mHolidayCombo->setCurrentIndex(
          mHolidayCombo->findData( KOGlobals::self()->holidays()->regionCode() ) );
      } else {
        mHolidayCombo->setCurrentIndex( 0 );
      }

      QGroupBox *workingHoursGroupBox =
        new QGroupBox( i18nc( "@title:group", "Working Period" ), regionalPage );
      regionalLayout->addWidget( workingHoursGroupBox, 2, 0 );

      QBoxLayout *workingHoursLayout = new QVBoxLayout( workingHoursGroupBox );

      QBoxLayout *workDaysLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workDaysLayout );

      // Respect start of week setting
      int weekStart = KGlobal::locale()->weekStartDay();
      for ( int i=0; i < 7; ++i ) {
        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
        QString weekDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                                   KCalendarSystem::ShortDayName );
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

      KPIM::KPrefsWidTime *workStart =
        addWidTime( KOPrefs::instance()->workingHoursStartItem() );

      QHBoxLayout *workStartLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workStartLayout );

      workStartLayout->addWidget( workStart->label() );
      workStartLayout->addWidget( workStart->timeEdit() );

      KPIM::KPrefsWidTime *workEnd =
        addWidTime( KOPrefs::instance()->workingHoursEndItem() );

      QHBoxLayout *workEndLayout = new QHBoxLayout;
      workingHoursLayout->addLayout( workEndLayout );

      workEndLayout->addWidget( workEnd->label() );
      workEndLayout->addWidget( workEnd->timeEdit() );

      KPIM::KPrefsWidBool *excludeHolidays =
        addWidBool( KOPrefs::instance()->excludeHolidaysItem() );

      workingHoursLayout->addWidget( excludeHolidays->checkBox() );

      regionalLayout->setRowStretch( 4, 1 );

      QFrame *defaultPage = new QFrame( parent );
      tabWidget->addTab( defaultPage, KIcon( QLatin1String("draw-eraser") ),
                         i18nc( "@title:tab", "Default Values" ) );
      QGridLayout *defaultLayout = new QGridLayout( defaultPage );
      defaultLayout->setSpacing( KDialog::spacingHint() );

      QGroupBox *timesGroupBox =
        new QGroupBox( i18nc( "@title:group", "Appointments" ), defaultPage );
      defaultLayout->addWidget( timesGroupBox, 0, 0 );

      QGridLayout *timesLayout = new QGridLayout( timesGroupBox );

      KPIM::KPrefsWidTime *defaultTime =
        addWidTime( CalendarSupport::KCalPrefs::instance()->startTimeItem(), defaultPage );
      timesLayout->addWidget( defaultTime->label(), 0, 0 );
      timesLayout->addWidget( defaultTime->timeEdit(), 0, 1 );

      KPIM::KPrefsWidDuration *defaultDuration =
        addWidDuration( CalendarSupport::KCalPrefs::instance()->defaultDurationItem(),
                        QLatin1String("hh:mm"), defaultPage );

      timesLayout->addWidget( defaultDuration->label(), 1, 0 );
      timesLayout->addWidget( defaultDuration->timeEdit(), 1, 1 );

      QGroupBox *remindersGroupBox =
        new QGroupBox( i18nc( "@title:group", "Reminders" ), defaultPage );
      defaultLayout->addWidget( remindersGroupBox, 1, 0 );

      QGridLayout *remindersLayout = new QGridLayout( remindersGroupBox );

      QLabel *reminderLabel =
        new QLabel( i18nc( "@label", "Default reminder time:" ), defaultPage );
      remindersLayout->addWidget( reminderLabel, 0, 0 );
      reminderLabel->setWhatsThis(
        CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis() );
      mReminderTimeSpin  = new KIntSpinBox( defaultPage );
      mReminderTimeSpin->setWhatsThis(
        CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis() );
      mReminderTimeSpin->setToolTip(
        CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->toolTip() );
      connect( mReminderTimeSpin, SIGNAL(valueChanged(int)), SLOT(slotWidChanged()) );
      remindersLayout->addWidget( mReminderTimeSpin, 0, 1 );

      mReminderUnitsCombo = new KComboBox( defaultPage );
      mReminderUnitsCombo->setToolTip(
        CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->toolTip() );
      mReminderUnitsCombo->setWhatsThis(
        CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->whatsThis() );
      connect( mReminderUnitsCombo, SIGNAL(activated(int)), SLOT(slotWidChanged()) );
      mReminderUnitsCombo->addItem(
        i18nc( "@item:inlistbox reminder units in minutes", "minute(s)" ) );
      mReminderUnitsCombo->addItem(
        i18nc( "@item:inlistbox reminder time units in hours", "hour(s)" ) );
      mReminderUnitsCombo->addItem(
        i18nc( "@item:inlistbox reminder time units in days", "day(s)" ) );
      remindersLayout->addWidget( mReminderUnitsCombo, 0, 2 );

      QCheckBox *cb =
        addWidBool(
          CalendarSupport::KCalPrefs::instance()->defaultAudioFileRemindersItem() )->checkBox();
      cb->setText( QString() );

      if ( CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->value().isEmpty() ) {
        const QString defAudioFile = KGlobal::dirs()->locate( "sound", QLatin1String("KDE-Sys-Warning.ogg") );
        CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->setValue(defAudioFile);
      }
      QString filter =
        i18n( "*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra|"
              "Audio Files (*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra)" );
      KUrlRequester *rq = addWidPath( CalendarSupport::KCalPrefs::instance()->audioFilePathItem(),
                                      0, filter )->urlRequester();
      rq->setEnabled( cb->isChecked() );

      connect( cb, SIGNAL(toggled(bool)), rq, SLOT(setEnabled(bool)) );

      QHBoxLayout *audioFileRemindersBox = new QHBoxLayout( remindersGroupBox );
      audioFileRemindersBox->addWidget( cb );
      audioFileRemindersBox->addWidget( rq );

      remindersLayout->addLayout( audioFileRemindersBox, 1, 0 );
      remindersLayout->addWidget(
        addWidBool(
          CalendarSupport::KCalPrefs::instance()->defaultEventRemindersItem() )->checkBox(), 2, 0 );
      remindersLayout->addWidget(
        addWidBool(
          CalendarSupport::KCalPrefs::instance()->defaultTodoRemindersItem() )->checkBox(), 3, 0 );

      defaultLayout->setRowStretch( 3, 1 );
      load();
    }

  protected:
    void usrReadConfig()
    {
      mReminderTimeSpin->setValue( CalendarSupport::KCalPrefs::instance()->mReminderTime );
      mReminderUnitsCombo->setCurrentIndex(
        CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits );
      for ( int i = 0; i < 7; ++i ) {
        mWorkDays[i]->setChecked( ( 1 << i ) & ( KOPrefs::instance()->mWorkWeekMask ) );
      }
    }

    void usrWriteConfig()
    {
      KOPrefs::instance()->mHolidays =
        mHolidayCombo->itemData( mHolidayCombo->currentIndex() ).toString();

      CalendarSupport::KCalPrefs::instance()->mReminderTime =
        mReminderTimeSpin->value();
      CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits =
        mReminderUnitsCombo->currentIndex();

      int mask = 0;
      for ( int i = 0; i < 7; ++i ) {
        if ( mWorkDays[i]->isChecked() ) {
          mask = mask | ( 1 << i );
        }
      }
      KOPrefs::instance()->mWorkWeekMask = mask;
      KOPrefs::instance()->writeConfig();
      CalendarSupport::KCalPrefs::instance()->writeConfig();
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
    QStringList   tzonenames;
    KComboBox    *mHolidayCombo;
    KIntSpinBox  *mReminderTimeSpin;
    KComboBox    *mReminderUnitsCombo;
    QCheckBox    *mWorkDays[7];
};

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigtime( QWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalog( QLatin1String("timezones4") );
    return new KOPrefsDialogTime( KOGlobals::self()->componentData(), parent );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class KOPrefsDialogViews : public KPIM::KPrefsModule
{
  public:
    KOPrefsDialogViews( const KComponentData &inst, QWidget *parent )
      : KPIM::KPrefsModule( KOPrefs::instance(), inst, parent ),
        mMonthIconComboBox( new KItemIconCheckCombo( KItemIconCheckCombo::MonthType, this ) ),
        mAgendaIconComboBox( new KItemIconCheckCombo( KItemIconCheckCombo::AgendaType, this ) )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );
      KTabWidget *tabWidget = new KTabWidget( this );
      topTopLayout->addWidget( tabWidget );

      connect( mMonthIconComboBox, SIGNAL(checkedItemsChanged(QStringList)),
               SLOT(slotWidChanged()) );
      connect( mAgendaIconComboBox, SIGNAL(checkedItemsChanged(QStringList)),
               SLOT(slotWidChanged()) );

      // Tab: Views->General
      QFrame *generalFrame = new QFrame( this );
      tabWidget->addTab( generalFrame, KIcon( QLatin1String("view-choose") ),
                         i18nc( "@title:tab general settings", "General" ) );

      QBoxLayout *generalLayout = new QVBoxLayout( generalFrame );

      // GroupBox: Views->General->Display Options
      QVBoxLayout *gdisplayLayout = new QVBoxLayout;
      QGroupBox *gdisplayBox = new QGroupBox( i18nc( "@title:group", "Display Options" ) );

      QBoxLayout *nextDaysLayout = new QHBoxLayout;
      gdisplayLayout->addLayout( nextDaysLayout );

      KPIM::KPrefsWidInt *nextDays =
        addWidInt( KOPrefs::instance()->nextXDaysItem() );
      nextDays->spinBox()->setSuffix(
        i18nc( "@label suffix in the N days spin box", " days" ) );

      nextDaysLayout->addWidget( nextDays->label() );
      nextDaysLayout->addWidget( nextDays->spinBox() );
      nextDaysLayout->addStretch( 1 );

      gdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableToolTipsItem() )->checkBox() );
      gdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->todosUseCategoryColorsItem() )->checkBox() );
      gdisplayBox->setLayout( gdisplayLayout );
      generalLayout->addWidget( gdisplayBox );

      // GroupBox: Views->General->Date Navigator
      QVBoxLayout *datenavLayout = new QVBoxLayout;
      QGroupBox *datenavBox = new QGroupBox( i18nc( "@title:group", "Date Navigator" ) );
      datenavLayout->addWidget(
        addWidBool( KOPrefs::instance()->dailyRecurItem() )->checkBox() );
      datenavLayout->addWidget(
        addWidBool( KOPrefs::instance()->weeklyRecurItem() )->checkBox() );
      datenavLayout->addWidget(
        addWidBool( KOPrefs::instance()->highlightTodosItem() )->checkBox() );
      datenavLayout->addWidget(
        addWidBool( KOPrefs::instance()->highlightJournalsItem() )->checkBox() );
      datenavLayout->addWidget(
        addWidBool( KOPrefs::instance()->weekNumbersShowWorkItem() )->checkBox() );
      datenavBox->setLayout( datenavLayout );
      generalLayout->addWidget( datenavBox );
      generalLayout->addStretch( 1 );

      // Tab: Views->Agenda View
      QFrame *agendaFrame = new QFrame( this );
      tabWidget->addTab( agendaFrame, KIcon( QLatin1String("view-calendar-workweek") ),
                         i18nc( "@title:tab", "Agenda View" ) );

      QBoxLayout *agendaLayout = new QVBoxLayout( agendaFrame );

      // GroupBox: Views->Agenda View->Display Options
      QVBoxLayout *adisplayLayout = new QVBoxLayout;
      QGroupBox *adisplayBox = new QGroupBox( i18nc( "@title:group", "Display Options" ) );

      QHBoxLayout *hourSizeLayout = new QHBoxLayout;
      adisplayLayout->addLayout( hourSizeLayout );

      KPIM::KPrefsWidInt *hourSize =
        addWidInt( KOPrefs::instance()->hourSizeItem() );
      hourSize->spinBox()->setSuffix(
        i18nc( "@label suffix in the hour size spin box", " pixels" ) );

      hourSizeLayout->addWidget( hourSize->label() );
      hourSizeLayout->addWidget( hourSize->spinBox() );
      hourSizeLayout->addStretch( 1 );

      adisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableAgendaItemIconsItem() )->checkBox() );
      adisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->showTodosAgendaViewItem() )->checkBox() );
      KPIM::KPrefsWidBool *marcusBainsEnabled =
        addWidBool( KOPrefs::instance()->marcusBainsEnabledItem() );
      adisplayLayout->addWidget( marcusBainsEnabled->checkBox() );

      KPIM::KPrefsWidBool *marcusBainsShowSeconds =
        addWidBool( KOPrefs::instance()->marcusBainsShowSecondsItem() );
      connect( marcusBainsEnabled->checkBox(), SIGNAL(toggled(bool)),
               marcusBainsShowSeconds->checkBox(), SLOT(setEnabled(bool)) );

      adisplayLayout->addWidget( marcusBainsShowSeconds->checkBox() );
      adisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->selectionStartsEditorItem() )->checkBox() );
      mAgendaIconComboBox->setCheckedIcons(
        KOPrefs::instance()->eventViewsPreferences()->agendaViewIcons() );
      adisplayLayout->addWidget( mAgendaIconComboBox );
      adisplayBox->setLayout( adisplayLayout );
      agendaLayout->addWidget( adisplayBox );

      // GroupBox: Views->Agenda View->Color Usage
      agendaLayout->addWidget(
        addWidRadios( KOPrefs::instance()->agendaViewColorsItem() )->groupBox() );

      agendaLayout->addWidget(
        addWidBool( KOPrefs::instance()->colorBusyDaysEnabledItem() )->checkBox() );

      // GroupBox: Views->Agenda View->Multiple Calendars
      agendaLayout->addWidget(
        addWidRadios( KOPrefs::instance()->agendaViewCalendarDisplayItem() )->groupBox() );

      agendaLayout->addStretch( 1 );

      // Tab: Views->Month View
      QFrame *monthFrame = new QFrame( this );
      tabWidget->addTab( monthFrame, KIcon( QLatin1String("view-calendar-month") ),
                         i18nc( "@title:tab", "Month View" ) );

      QBoxLayout *monthLayout = new QVBoxLayout( monthFrame );

      // GroupBox: Views->Month View->Display Options
      QVBoxLayout *mdisplayLayout = new QVBoxLayout;
      QGroupBox *mdisplayBox = new QGroupBox( i18nc( "@title:group", "Display Options" ) );
      /*mdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableMonthScrollItem() )->checkBox() );*/
      mdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->showTimeInMonthViewItem() )->checkBox() );
      mdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->enableMonthItemIconsItem() )->checkBox() );
      mdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->showTodosMonthViewItem() )->checkBox() );
      mdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->showJournalsMonthViewItem() )->checkBox() );
      mdisplayBox->setLayout( mdisplayLayout );

      mMonthIconComboBox->setCheckedIcons(
        KOPrefs::instance()->eventViewsPreferences()->monthViewIcons() );
      mdisplayLayout->addWidget( mMonthIconComboBox );

      monthLayout->addWidget( mdisplayBox );

      monthLayout->addWidget(
        addWidBool( KOPrefs::instance()->colorMonthBusyDaysEnabledItem() )->checkBox() );

      // GroupBox: Views->Month View->Color Usage
      monthLayout->addWidget(
        addWidRadios( KOPrefs::instance()->monthViewColorsItem() )->groupBox() );
      monthLayout->addStretch( 1 );

      // Tab: Views->Todo View
      QFrame *todoFrame = new QFrame( this );
      tabWidget->addTab( todoFrame, KIcon( QLatin1String("view-calendar-tasks") ),
                         i18nc( "@title:tab", "Todo View" ) );

      QBoxLayout *todoLayout = new QVBoxLayout( todoFrame );

      // GroupBox: Views->Todo View->Display Options
      QVBoxLayout *tdisplayLayout = new QVBoxLayout;
      QGroupBox *tdisplayBox = new QGroupBox( i18nc( "@title:group", "Display Options" ) );
      tdisplayLayout->addWidget(
        addWidBool( KOPrefs::instance()->sortCompletedTodosSeparatelyItem() )->checkBox() );
      tdisplayBox->setLayout( tdisplayLayout );
      todoLayout->addWidget( tdisplayBox );

      // GroupBox: Views->Todo View->Other
      QVBoxLayout *otherLayout = new QVBoxLayout;
      QGroupBox *otherBox = new QGroupBox( i18nc( "@title:group", "Other Options" ) );
      otherLayout->addWidget(
        addWidBool( KOPrefs::instance()->recordTodosInJournalsItem() )->checkBox() );
      otherBox->setLayout( otherLayout );
      todoLayout->addWidget( otherBox );
      todoLayout->addStretch( 1 );

      load();
    }
  protected:
    void usrWriteConfig()
    {
      KOPrefs::instance()->eventViewsPreferences()->setAgendaViewIcons(
        mAgendaIconComboBox->checkedIcons() );
      KOPrefs::instance()->eventViewsPreferences()->setMonthViewIcons(
        mMonthIconComboBox->checkedIcons() );
    }
  private:
    KItemIconCheckCombo *mMonthIconComboBox;
    KItemIconCheckCombo *mAgendaIconComboBox;
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
  : KPIM::KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );
  KTabWidget *tabWidget = new KTabWidget( this );
  topTopLayout->addWidget( tabWidget );

  QWidget *colorFrame = new QWidget( this );
  topTopLayout->addWidget( colorFrame );
  QGridLayout *colorLayout = new QGridLayout(colorFrame);
  colorLayout->setSpacing( KDialog::spacingHint() );
  tabWidget->addTab( colorFrame, KIcon( QLatin1String("preferences-desktop-color") ),
                     i18nc( "@title:tab", "Colors" ) );

  // Holiday Color
  KPIM::KPrefsWidColor *holidayColor =
    addWidColor( KOPrefs::instance()->agendaHolidaysBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( holidayColor->label(), 0, 0 );
  colorLayout->addWidget( holidayColor->button(), 0, 1 );

  // agenda view background color
  KPIM::KPrefsWidColor *agendaBgColor =
    addWidColor( KOPrefs::instance()->agendaGridBackgroundColorItem(), colorFrame );
  colorLayout->addWidget( agendaBgColor->label(), 3, 0 );
  colorLayout->addWidget( agendaBgColor->button(), 3, 1 );

  // agenda view Marcus Bains line color
  KPIM::KPrefsWidColor *mblColor =
    addWidColor( KOPrefs::instance()->agendaMarcusBainsLineLineColorItem(), colorFrame );
  colorLayout->addWidget( mblColor->label(), 5, 0 );
  colorLayout->addWidget( mblColor->button(), 5, 1 );

  KPIM::KPrefsWidColor *viewBgBusyColor =
      addWidColor( KOPrefs::instance()->viewBgBusyColorItem(), colorFrame );
  colorLayout->addWidget( viewBgBusyColor->label(), 4, 0 );
  colorLayout->addWidget( viewBgBusyColor->button(), 4, 1 );

  // working hours color
  KPIM::KPrefsWidColor *agendaGridWorkHoursBackgroundColor =
    addWidColor( KOPrefs::instance()->workingHoursColorItem(), colorFrame );
  colorLayout->addWidget( agendaGridWorkHoursBackgroundColor->label(), 6, 0 );
  colorLayout->addWidget( agendaGridWorkHoursBackgroundColor->button(), 6, 1 );

  // Todo due today color
  KPIM::KPrefsWidColor *todoDueTodayColor =
    addWidColor(
      KOPrefs::instance()->todoDueTodayColorItem(), colorFrame );
  colorLayout->addWidget( todoDueTodayColor->label(), 7, 0 );
  colorLayout->addWidget( todoDueTodayColor->button(), 7, 1 );

  // Todo overdue color
  KPIM::KPrefsWidColor *todoOverdueColor =
    addWidColor(
      KOPrefs::instance()->todoOverdueColorItem(), colorFrame );
  colorLayout->addWidget( todoOverdueColor->label(), 8, 0 );
  colorLayout->addWidget( todoOverdueColor->button(), 8, 1 );

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox( i18nc( "@title:group", "Categories" ), colorFrame );
  colorLayout->addWidget( categoryGroup, 9, 0, 1, 2 );

  QGridLayout *categoryLayout = new QGridLayout;
  categoryGroup->setLayout( categoryLayout );

  KPIM::KPrefsWidColor *unsetCategoryColor =
    addWidColor( CalendarSupport::KCalPrefs::instance()->unsetCategoryColorItem(), categoryGroup );
  categoryLayout->addWidget( unsetCategoryColor->label(), 0, 0 );
  categoryLayout->addWidget( unsetCategoryColor->button(), 0, 1 );
  unsetCategoryColor->label()->setWhatsThis( unsetCategoryColor->button()->whatsThis() );
  unsetCategoryColor->label()->setToolTip( unsetCategoryColor->button()->toolTip() );

  mCategoryCombo = new KPIM::TagCombo( categoryGroup );
  mCategoryCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select here the event category you want to modify. "
           "You can change the selected category color using "
           "the button below." ) );
  connect( mCategoryCombo, SIGNAL(activated(int)), SLOT(updateCategoryColor()) );
  categoryLayout->addWidget( mCategoryCombo, 1, 0 );

  mCategoryButton = new KColorButton( categoryGroup );
  mCategoryButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose here the color of the event category selected "
           "using the combo box above." ) );
  connect( mCategoryButton, SIGNAL(changed(QColor)), SLOT(setCategoryColor()) );
  categoryLayout->addWidget( mCategoryButton, 1, 1 );

  updateCategoryColor();

  // resources colors
  QGroupBox *resourceGroup = new QGroupBox( i18nc( "@title:group", "Resources" ), colorFrame );
  colorLayout->addWidget( resourceGroup, 10, 0, 1, 2 );

  QBoxLayout *resourceLayout = new QHBoxLayout;
  resourceGroup->setLayout( resourceLayout );

  mResourceCombo = new Akonadi::CollectionComboBox( resourceGroup );
  //mResourceCombo->addExcludedSpecialResources(Akonadi::Collection::SearchResource);
  QStringList mimetypes;
  mimetypes << KCalCore::Todo::todoMimeType();
  mimetypes << KCalCore::Journal::journalMimeType();
  mimetypes << KCalCore::Event::eventMimeType();

  mResourceCombo->setMimeTypeFilter( mimetypes );
  mResourceCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select the calendar you want to modify. "
           "You can change the selected calendar color using "
           "the button below." ) );
  connect( mResourceCombo, SIGNAL(activated(int)), SLOT(updateResourceColor()) );
  resourceLayout->addWidget( mResourceCombo );

  mResourceButton = new KColorButton( resourceGroup );
  mResourceButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose here the color of the calendar selected "
           "using the combo box above." ) );
  connect( mResourceButton, SIGNAL(changed(QColor)), SLOT(setResourceColor()) );
  resourceLayout->addWidget( mResourceButton );

  colorLayout->setRowStretch( 11, 1 );

  QWidget *fontFrame = new QWidget( this );
  tabWidget->addTab( fontFrame, KIcon( QLatin1String("preferences-desktop-font") ),
                     i18nc( "@title:tab", "Fonts" ) );

  QGridLayout *fontLayout = new QGridLayout( fontFrame );
  fontLayout->setSpacing( KDialog::spacingHint() );

  KPIM::KPrefsWidFont *timeBarFont =
     addWidFont( KOPrefs::instance()->agendaTimeLabelsFontItem(), fontFrame,
                KGlobal::locale()->formatTime( QTime( 12, 34 ) ) );
  fontLayout->addWidget( timeBarFont->label(), 0, 0 );
  fontLayout->addWidget( timeBarFont->preview(), 0, 1 );
  fontLayout->addWidget( timeBarFont->button(), 0, 2 );

  KPIM::KPrefsWidFont *monthViewFont =
    addWidFont( KOPrefs::instance()->monthViewFontItem(), fontFrame,
                KGlobal::locale()->formatTime( QTime( 12, 34 ) ) + QLatin1Char(' ') +
                i18nc( "@label", "Event text" ) );

  fontLayout->addWidget( monthViewFont->label(), 1, 0 );
  fontLayout->addWidget( monthViewFont->preview(), 1, 1 );
  fontLayout->addWidget( monthViewFont->button(), 1, 2 );

  KPIM::KPrefsWidFont *agendaViewFont =
    addWidFont( KOPrefs::instance()->agendaViewFontItem(), fontFrame,
                i18nc( "@label", "Event text" ) );
  fontLayout->addWidget( agendaViewFont->label(), 2, 0 );
  fontLayout->addWidget( agendaViewFont->preview(), 2, 1 );
  fontLayout->addWidget( agendaViewFont->button(), 2, 2 );

  KPIM::KPrefsWidFont *marcusBainsFont =
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
    CalendarSupport::KCalPrefs::instance()->setCategoryColor( i.key(), i.value() );
    ++i;
  }

  i = mResourceDict.constBegin();
  while ( i != mResourceDict.constEnd() ) {
    KOPrefs::instance()->setResourceColor( i.key(), i.value() );
    ++i;
  }

  //mCalendarViewsPrefs->writeConfig();
}

void KOPrefsDialogColorsAndFonts::usrReadConfig()
{
  updateCategories();
  updateResources();
  //mCalendarViewsPrefs->readConfig();
}

void KOPrefsDialogColorsAndFonts::updateCategories()
{
  updateCategoryColor();
}

void KOPrefsDialogColorsAndFonts::setCategoryColor()
{
  mCategoryDict.insert( mCategoryCombo->currentText(), mCategoryButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateCategoryColor()
{
  const QString cat = mCategoryCombo->currentText();
  QColor color = mCategoryDict.value( cat );
  if ( !color.isValid() ) {
    //TODO get this from the tag
    color = CalendarSupport::KCalPrefs::instance()->categoryColor( cat );
  }
  if ( color.isValid() ) {
    mCategoryButton->setColor( color );
  }
}

void KOPrefsDialogColorsAndFonts::updateResources()
{
  updateResourceColor();
}

void KOPrefsDialogColorsAndFonts::setResourceColor()
{
  bool ok;
  const QString id =
    QString::number( mResourceCombo->itemData(
                       mResourceCombo->currentIndex(),
                       Akonadi::CollectionModel::CollectionIdRole ).toLongLong( &ok ) );
  if( ! ok ) {
    return;
  }
  mResourceDict.insert( id, mResourceButton->color() );
  slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateResourceColor()
{
  bool ok;
  const QString id =
    QString::number( mResourceCombo->itemData(
                       mResourceCombo->currentIndex(),
                       Akonadi::CollectionModel::CollectionIdRole ).toLongLong( &ok ) );
  if ( !ok ) {
    return;
  }
  kDebug() << id << mResourceCombo->itemText( mResourceCombo->currentIndex() );

  QColor color = mResourceDict.value( id );
  if ( ! color.isValid() ) {
    color = KOPrefs::instance()->resourceColor( id );
  }
  mResourceButton->setColor( color );
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
  : KPIM::KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout( topFrame );
  topLayout->setSpacing( KDialog::spacingHint() );

  KPIM::KPrefsWidBool *useGroupwareBool =
    addWidBool( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem(), topFrame );
  topLayout->addWidget( useGroupwareBool->checkBox(), 0, 0, 1, 2 );

  KPIM::KPrefsWidBool *bcc =
    addWidBool( Akonadi::CalendarSettings::self()->bccItem(), topFrame );
  topLayout->addWidget( bcc->checkBox(), 1, 0, 1, 2 );

  QLabel *aTransportLabel = new QLabel(
    i18nc( "@label", "Mail transport:" ), topFrame );
  topLayout->addWidget( aTransportLabel, 2, 0, 1, 2 );

  MailTransport::TransportManagementWidget *tmw =
    new MailTransport::TransportManagementWidget( topFrame );
  tmw->layout()->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addWidget( tmw, 3, 0, 1, 2 );

  //topLayout->setRowStretch( 2, 1 );


  load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
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
  : KPrefsModule( CalendarSupport::KCalPrefs::instance(), inst, parent )
{
  mGroupwarePage = new Ui::KOGroupwarePrefsPage();
  QWidget *widget = new QWidget( this );
  widget->setObjectName( QLatin1String("KOGrouparePrefsPage") );

  mGroupwarePage->setupUi( widget );

  mGroupwarePage->groupwareTab->setTabIcon( 0, KIcon( QLatin1String("go-up") ) );
  mGroupwarePage->groupwareTab->setTabIcon( 1, KIcon( QLatin1String("go-down") ) );

  // signals and slots connections

  connect( mGroupwarePage->publishDays, SIGNAL(valueChanged(int)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishUrl, SIGNAL(textChanged(QString)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishUser, SIGNAL(textChanged(QString)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishPassword, SIGNAL(textChanged(QString)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->publishSavePassword, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveEnable, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveUser, SIGNAL(textChanged(QString)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrievePassword, SIGNAL(textChanged(QString)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveSavePassword, SIGNAL(toggled(bool)),
           SLOT(slotWidChanged()) );
  connect( mGroupwarePage->retrieveUrl, SIGNAL(textChanged(QString)),
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
    Akonadi::CalendarSettings::self()->freeBusyPublishAuto() );
  mGroupwarePage->publishDelay->setValue(
    Akonadi::CalendarSettings::self()->freeBusyPublishDelay() );
  mGroupwarePage->publishDays->setValue(
    Akonadi::CalendarSettings::self()->freeBusyPublishDays() );
  mGroupwarePage->publishUrl->setText(
    Akonadi::CalendarSettings::self()->freeBusyPublishUrl() );
  mGroupwarePage->publishUser->setText(
    Akonadi::CalendarSettings::self()->freeBusyPublishUser() );
  mGroupwarePage->publishPassword->setText(
    Akonadi::CalendarSettings::self()->freeBusyPublishPassword() );
  mGroupwarePage->publishSavePassword->setChecked(
    Akonadi::CalendarSettings::self()->freeBusyPublishSavePassword() );

  mGroupwarePage->retrieveEnable->setChecked(
    Akonadi::CalendarSettings::self()->freeBusyRetrieveAuto() );
  mGroupwarePage->fullDomainRetrieval->setChecked(
    Akonadi::CalendarSettings::self()->freeBusyFullDomainRetrieval() );
  mGroupwarePage->retrieveUrl->setText(
    Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl() );
  mGroupwarePage->retrieveUser->setText(
    Akonadi::CalendarSettings::self()->freeBusyRetrieveUser() );
    mGroupwarePage->retrievePassword->setText(
    Akonadi::CalendarSettings::self()->freeBusyRetrievePassword() );
  mGroupwarePage->retrieveSavePassword->setChecked(
    Akonadi::CalendarSettings::self()->freeBusyRetrieveSavePassword() );
}

void KOPrefsDialogGroupwareScheduling::usrWriteConfig()
{
  Akonadi::CalendarSettings::self()->setFreeBusyPublishAuto(
    mGroupwarePage->publishEnable->isChecked());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishDelay(mGroupwarePage->publishDelay->value());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishDays(
    mGroupwarePage->publishDays->value());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishUrl(
    mGroupwarePage->publishUrl->text());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishUser(
    mGroupwarePage->publishUser->text());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishPassword(
    mGroupwarePage->publishPassword->text());
  Akonadi::CalendarSettings::self()->setFreeBusyPublishSavePassword(
    mGroupwarePage->publishSavePassword->isChecked());

  Akonadi::CalendarSettings::self()->setFreeBusyRetrieveAuto(
    mGroupwarePage->retrieveEnable->isChecked());
  Akonadi::CalendarSettings::self()->setFreeBusyFullDomainRetrieval(
    mGroupwarePage->fullDomainRetrieval->isChecked());
  Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUrl(
    mGroupwarePage->retrieveUrl->text());
  Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUser(
    mGroupwarePage->retrieveUser->text());
  Akonadi::CalendarSettings::self()->setFreeBusyRetrievePassword(
    mGroupwarePage->retrievePassword->text());
  Akonadi::CalendarSettings::self()->setFreeBusyRetrieveSavePassword(
    mGroupwarePage->retrieveSavePassword->isChecked());

  // clear the url cache for our user
  const QString configFile = KStandardDirs::locateLocal( "data", QLatin1String("korganizer/freebusyurls") );
  KConfig cfg( configFile );
  cfg.deleteGroup( CalendarSupport::KCalPrefs::instance()->email() );


  Akonadi::CalendarSettings::self()->writeConfig();
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
              QLatin1String("configure"), QString(),
              i18nc( "@info:whatsthis",
                     "This button allows you to configure"
                     " the plugin that you have selected in the list above" ) ),
    buttonRow );
  buttonRowLayout->addWidget( mConfigureButton );
  buttonRowLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding ) );
  topLayout->addWidget( buttonRow );

  mPositioningGroupBox = new QGroupBox( i18nc( "@title:group", "Position" ), topFrame );
  //mPositionMonthTop = new QCheckBox(
  //i18nc( "@option:check", "Show in the month view" ), mPositioningGroupBox );
  mPositionAgendaTop = new QRadioButton(
    i18nc( "@option:check", "Show at the top of the agenda views" ), mPositioningGroupBox );
  mPositionAgendaBottom = new QRadioButton(
    i18nc( "@option:check", "Show at the bottom of the agenda views" ), mPositioningGroupBox );
  QVBoxLayout *positioningLayout = new QVBoxLayout( mPositioningGroupBox );
  //positioningLayout->addWidget( mPositionMonthTop );
  positioningLayout->addWidget( mPositionAgendaTop );
  positioningLayout->addWidget( mPositionAgendaBottom );
  positioningLayout->addStretch( 1 );
  topLayout->addWidget( mPositioningGroupBox );

  connect( mConfigureButton, SIGNAL(clicked()), SLOT(configure()) );

  //connect( mPositionMonthTop, SIGNAL(clicked()), SLOT(positioningChanged()) );
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

  EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();

  QStringList selectedPlugins = viewPrefs->selectedPlugins();

  QTreeWidgetItem *decorations =
    new QTreeWidgetItem( mTreeWidget, QStringList(
                           i18nc( "@title:group", "Calendar Decorations" ) ) );
  QTreeWidgetItem *others =
    new QTreeWidgetItem( mTreeWidget, QStringList(
                           i18nc( "@title:group", "Other Plugins" ) ) );

  KService::List::ConstIterator it;
  for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
    QTreeWidgetItem *item = 0;
    if ( (*it)->hasServiceType( EventViews::CalendarDecoration::Decoration::serviceType() ) ) {
      item = new PluginItem( decorations, *it );
    } else if ( !(*it)->hasServiceType( QLatin1String( "KOrganizer/PrintPlugin"  ) ) ) {
      // we specifically skip print plugins since we no longer support them
      item = new PluginItem( others, *it );
    } else {
      continue;
    }
    if ( selectedPlugins.contains( (*it)->desktopEntryName() ) ) {
      item->setCheckState( 0, Qt::Checked );
    } else {
      item->setCheckState( 0, Qt::Unchecked );
    }
  }

  decorations->setExpanded( true );
  others->setExpanded( true );

  mDecorationsAtMonthViewTop = KOPrefs::instance()->decorationsAtMonthViewTop().toSet();
  mDecorationsAtAgendaViewTop = viewPrefs->decorationsAtAgendaViewTop().toSet();
  mDecorationsAtAgendaViewBottom = viewPrefs->decorationsAtAgendaViewBottom().toSet();
}

void KOPrefsDialogPlugins::usrWriteConfig()
{
  QStringList selectedPlugins;

  for ( int i = 0; i < mTreeWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem *serviceTypeGroup = mTreeWidget->topLevelItem( i );
    for ( int j = 0; j < serviceTypeGroup->childCount(); j++) {
      PluginItem *item = static_cast<PluginItem *>( serviceTypeGroup->child( j ) );
      if( item->checkState( 0 ) == Qt::Checked ) {
        selectedPlugins.append( item->service()->desktopEntryName() );
      }
    }
  }
  EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();
  viewPrefs->setSelectedPlugins( selectedPlugins );

  KOPrefs::instance()->setDecorationsAtMonthViewTop( mDecorationsAtMonthViewTop.toList() );
  viewPrefs->setDecorationsAtAgendaViewTop( mDecorationsAtAgendaViewTop.toList() );
  viewPrefs->setDecorationsAtAgendaViewBottom( mDecorationsAtAgendaViewBottom.toList() );
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

  CalendarSupport::Plugin *plugin = KOCore::self()->loadPlugin( item->service() );

  if ( plugin ) {
    plugin->configure( this );
    delete plugin;

    slotWidChanged();
  } else {
    KMessageBox::sorry( this,
                        i18nc( "@info", "Unable to configure this plugin" ),
                        QLatin1String("PluginConfigUnable") );
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

  /*if ( mPositionMonthTop->checkState() == Qt::Checked ) {
    if ( !mDecorationsAtMonthViewTop.contains( decoration ) ) {
      mDecorationsAtMonthViewTop.insert( decoration );
    }
  } else {
    mDecorationsAtMonthViewTop.remove( decoration );
  }*/

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
  //mPositionMonthTop->setChecked( false );
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

  QVariant variant = item->service()->property( QLatin1String("X-KDE-KOrganizer-HasSettings") );

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
  if ( item->service()->hasServiceType(
         EventViews::CalendarDecoration::Decoration::serviceType() ) ) {
    QString decoration = item->service()->desktopEntryName();
    /*if ( mDecorationsAtMonthViewTop.contains( decoration ) ) {
      mPositionMonthTop->setChecked( true );
      hasPosition = true;
    }*/
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
  const QString dir = KStandardDirs::locateLocal( "data", QLatin1String("korganizer/designer/event/") );
  return dir;
}

QString KOPrefsDesignerFields::uiPath()
{
  return QLatin1String("korganizer/designer/event/");
}

void KOPrefsDesignerFields::writeActivePages( const QStringList &activePages )
{
  CalendarSupport::KCalPrefs::instance()->setActiveDesignerFields( activePages );
  CalendarSupport::KCalPrefs::instance()->writeConfig();
}

QStringList KOPrefsDesignerFields::readActivePages()
{
  return CalendarSupport::KCalPrefs::instance()->activeDesignerFields();
}

QString KOPrefsDesignerFields::applicationName()
{
  return QLatin1String("KORGANIZER");
}

