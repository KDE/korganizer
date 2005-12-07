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

#include <qlayout.h>
#include <qlabel.h>
#include <q3groupbox.h>
#include <q3buttongroup.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qfile.h>
#include <qcombobox.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <q3grid.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <q3strlist.h>
#include <q3listview.h>
#include <qtabwidget.h>

//Added by qt3to4:
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kemailsettings.h>
#include <kcalendarsystem.h>
#include <ktrader.h>
#include <kpushbutton.h>
#include <kocore.h>
#include <libkcal/calendarresources.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kholidays.h>

#if defined(USE_SOLARIS)
#include <sys/param.h>

#define ZONEINFODIR    "/usr/share/lib/zoneinfo"
#define INITFILE       "/etc/default/init"
#endif

#include "koprefs.h"

#include "koprefsdialog.h"
#include "kogroupwareprefspage.h"
#include "ktimeedit.h"
#include "koglobals.h"
#include "stdcalendar.h"
#include <kdepimmacros.h>


KOPrefsDialogMain::KOPrefsDialogMain( KInstance *inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  Q3VBox *topFrame = new Q3VBox( this );
  topTopLayout->addWidget( topFrame );

  topFrame->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *emailControlCenter =
      addWidBool( KOPrefs::instance()->emailControlCenterItem(), topFrame );
  connect(emailControlCenter->checkBox(),SIGNAL(toggled(bool)),
          SLOT(toggleEmailSettings(bool)));

  mUserEmailSettings = new Q3Grid( 2, topFrame );

  addWidString( KOPrefs::instance()->userNameItem(), mUserEmailSettings );
  addWidString( KOPrefs::instance()->userEmailItem(), mUserEmailSettings );

  Q3GroupBox *saveGroup = new Q3GroupBox(1,Qt::Horizontal,i18n("Saving Calendar"),
                                           topFrame);

  addWidBool( KOPrefs::instance()->htmlWithSaveItem(), saveGroup );

  KPrefsWidBool *autoSave = addWidBool( KOPrefs::instance()->autoSaveItem(), saveGroup );

  Q3HBox *intervalBox = new Q3HBox( saveGroup );
  addWidInt( KOPrefs::instance()->autoSaveIntervalItem(), intervalBox );
  connect( autoSave->checkBox(), SIGNAL( toggled( bool ) ),
           intervalBox, SLOT( setEnabled( bool ) ) );
  intervalBox->setSpacing( KDialog::spacingHint() );
  new QWidget( intervalBox );

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
    return new KOPrefsDialogMain( KOGlobals::self()->instance(), parent /*, "kcmkorganizermain"*/ );
  }
}


class KOPrefsDialogTime : public KPrefsModule
{
  public:
    KOPrefsDialogTime( KInstance *inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );

      QWidget *topFrame = new QWidget( this );
      topTopLayout->addWidget( topFrame );

      QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
      topLayout->setSpacing( KDialog::spacingHint() );

      Q3HBox *timeZoneBox = new Q3HBox( topFrame );
      topLayout->addMultiCellWidget( timeZoneBox, 0, 0, 0, 1 );

      QLabel *timeZoneLabel = new QLabel( i18n("Timezone:"), timeZoneBox );
      QString whatsThis = i18n( "Select your timezone from the list of "
                                "locations on this drop down box. If your city "
                                "is not listed, select one which shares the "
                                "same timezone. KOrganizer will automatically "
                                "adjust for daylight savings." );
      timeZoneLabel->setWhatsThis( whatsThis );
      mTimeZoneCombo = new QComboBox( timeZoneBox );

      connect( mTimeZoneCombo, SIGNAL( activated( int ) ),
               SLOT( slotWidChanged() ) );

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
      for ( int i = 0; i < mTimeZoneCombo->count(); ++i )
        {
          if (mTimeZoneCombo->text(i) == sCurrentlySet)
            {
             nCurrentlySet = i;
             break;
            }
        }

      mTimeZoneCombo->setCurrentItem(nCurrentlySet);
      mTimeZoneCombo->setWhatsThis( whatsThis );

      // holiday region selection
      Q3HBox *holidayRegBox = new Q3HBox( topFrame );
      topLayout->addMultiCellWidget( holidayRegBox, 1, 1, 0, 1 );

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
        QString countryFile = locate( "locale",
                                      "l10n/" + (*it) + "/entry.desktop" );
        QString regionName;
        if ( !countryFile.isEmpty() ) {
          KSimpleConfig cfg( countryFile );
          cfg.setGroup( "KCM Locale" );
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

      mHolidayCombo->insertStringList(holidayList);

      for (int i=0; i < mHolidayCombo->count(); ++i) {
        if ( mHolidayCombo->text(i) == currentHolidayName ) {
          mHolidayCombo->setCurrentItem(i);
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
      QLabel *alarmLabel = new QLabel( i18n( "Default reminder time:" ), topFrame);
      topLayout->addWidget( alarmLabel, 5, 0 );
      alarmLabel->setWhatsThis(
                       i18n( "Enter the default reminder time here." ) );
      mAlarmTimeCombo = new QComboBox( topFrame );
      mAlarmTimeCombo->setWhatsThis(
                       i18n( "Enter the default reminder time here." ) );
      connect( mAlarmTimeCombo, SIGNAL( activated( int ) ),
               SLOT( slotWidChanged() ) );
      mAlarmTimeCombo->insertStringList( alarmList );
      topLayout->addWidget( mAlarmTimeCombo, 5, 1 );


      Q3GroupBox *workingHoursGroup = new Q3GroupBox(1,Qt::Horizontal,
                                                   i18n("Working Hours"),
                                                   topFrame);
      topLayout->addMultiCellWidget( workingHoursGroup, 6, 6, 0, 1 );

      Q3HBox *workDaysBox = new Q3HBox( workingHoursGroup );
      // Respect start of week setting
      int weekStart=KGlobal::locale()->weekStartDay();
      for ( int i = 0; i < 7; ++i ) {
        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
        QString weekDayName = calSys->weekDayName( (i + weekStart + 6)%7 + 1, true );
        if ( KOPrefs::instance()->mCompactDialogs ) {
          weekDayName = weekDayName.left( 1 );
        }
        int index = ( i + weekStart + 6 ) % 7;
        mWorkDays[ index ] = new QCheckBox( weekDayName, workDaysBox );
        mWorkDays[ index ]->setWhatsThis(
                         i18n( "Check this box to make KOrganizer mark the "
                               "working hours for this day of the week. "
                               "If this is a work day for you, check "
                               "this box, or the working hours will not be "
                               "marked with color." ) );

        connect( mWorkDays[ index ], SIGNAL( stateChanged( int ) ),
               SLOT( slotWidChanged() ) );
      }

      Q3HBox *workStartBox = new Q3HBox(workingHoursGroup);
      addWidTime( KOPrefs::instance()->workingHoursStartItem(), workStartBox );

      Q3HBox *workEndBox = new Q3HBox(workingHoursGroup);
      addWidTime( KOPrefs::instance()->workingHoursEndItem(), workEndBox );


      addWidBool( KOPrefs::instance()->excludeHolidaysItem(),
                  workingHoursGroup );

      topLayout->setRowStretch(7,1);

      load();
    }

  protected:
    void usrReadConfig()
    {
      setCombo( mTimeZoneCombo,
                i18n( KOPrefs::instance()->mTimeZoneId.utf8() ) );

      mAlarmTimeCombo->setCurrentItem( KOPrefs::instance()->mAlarmTime );
      for ( int i = 0; i < 7; ++i ) {
        mWorkDays[i]->setChecked( (1<<i) & (KOPrefs::instance()->mWorkWeekMask) );
      }
    }

    void usrWriteConfig()
    {
      // Find untranslated selected zone
      QStringList::Iterator tz;
      for ( tz = tzonenames.begin(); tz != tzonenames.end(); ++tz )
        if (mTimeZoneCombo->currentText() == i18n((*tz).utf8()))
          break;
      if (tz != tzonenames.end())
        KOPrefs::instance()->mTimeZoneId = (*tz);
      else
        KOPrefs::instance()->mTimeZoneId = mTimeZoneCombo->currentText();

      KOPrefs::instance()->mHolidays = ( mHolidayCombo->currentItem() == 0 ) ?  // (None)
                                       QString::null :
                                       mRegionMap[mHolidayCombo->currentText()];

      KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentItem();
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
  KCModule *create_korganizerconfigtime( QWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalog( "timezones" );
    return new KOPrefsDialogTime( KOGlobals::self()->instance(), parent /*, "kcmkorganizertime"*/ );
  }
}


class KOPrefsDialogViews : public KPrefsModule
{
  public:
    KOPrefsDialogViews( KInstance *inst, QWidget *parent )
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

      /*** Date Navigator Group ***/
      Q3GroupBox *dateNavGroup = new Q3GroupBox( 1, Qt::Horizontal,
                                               i18n("Date Navigator"),
                                               topFrame );
      addWidBool( KOPrefs::instance()->dailyRecurItem(), dateNavGroup );
      addWidBool( KOPrefs::instance()->weeklyRecurItem(), dateNavGroup );
      topLayout->addWidget( dateNavGroup );


      /*** Agenda View Group ***/
      Q3GroupBox *agendaGroup = new Q3GroupBox( 1, Qt::Horizontal,
                                              i18n("Agenda View"),
                                              topFrame );

      Q3HBox *hourSizeBox = new Q3HBox( agendaGroup );
      KPrefsWidInt *hourSize =
          addWidInt( KOPrefs::instance()->hourSizeItem(), hourSizeBox );
      hourSize->spinBox()->setSuffix(i18n("suffix in the hour size spin box", " pixel"));
      // horizontal spacer:
      new QWidget( hourSizeBox );

      Q3HBox *nextDaysBox = new Q3HBox( agendaGroup );
      KPrefsWidInt *nextDays =
        addWidInt( KOPrefs::instance()->nextXDaysItem(), nextDaysBox );
      nextDays->spinBox()->setSuffix(i18n("suffix in the N days spin box", " days"));
      new QWidget( nextDaysBox );

      KPrefsWidBool *marcusBainsEnabled =
          addWidBool( KOPrefs::instance()->marcusBainsEnabledItem(), agendaGroup );

      KPrefsWidBool *marcusBainsShowSeconds =
          addWidBool( KOPrefs::instance()->marcusBainsShowSecondsItem(), agendaGroup );
      connect( marcusBainsEnabled->checkBox(), SIGNAL( toggled( bool ) ),
               marcusBainsShowSeconds->checkBox(), SLOT( setEnabled( bool ) ) );

      addWidBool( KOPrefs::instance()->selectionStartsEditorItem(), agendaGroup );

      addWidBool( KOPrefs::instance()->agendaViewUsesResourceColorItem(), agendaGroup );

      topLayout->addWidget( agendaGroup );


      /*** Month View Group ***/
      Q3GroupBox *monthGroup = new Q3GroupBox( 1, Qt::Horizontal,
                                             i18n("Month View"),
                                             topFrame );
      addWidBool( KOPrefs::instance()->enableMonthScrollItem(), monthGroup );
      addWidBool( KOPrefs::instance()->fullViewMonthItem(), monthGroup );
      addWidBool( KOPrefs::instance()->monthViewUsesCategoryColorItem(),
                      monthGroup );
      addWidBool( KOPrefs::instance()->monthViewUsesResourceColorItem(), monthGroup );
      topLayout->addWidget( monthGroup );


      /*** Todo View Group ***/
      Q3GroupBox *todoGroup = new Q3GroupBox( 1, Qt::Horizontal,
                                            i18n("To-do View"),
                                            topFrame );
      addWidBool( KOPrefs::instance()->fullViewTodoItem(), todoGroup );
      addWidBool( KOPrefs::instance()->useSplitListViewsItem(), todoGroup );
      addWidBool( KOPrefs::instance()->recordTodosInJournalsItem(), todoGroup );
      topLayout->addWidget( todoGroup );

      topLayout->addStretch( 1 );

      load();
    }
};

extern "C"
{
  KCModule *create_korganizerconfigviews( QWidget *parent, const char * )
  {
    return new KOPrefsDialogViews( KOGlobals::self()->instance(), parent /*, "kcmkorganizerviews"*/ );
  }
}


class KOPrefsDialogFonts : public KPrefsModule
{
  public:
    KOPrefsDialogFonts( KInstance *inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
    {
      QBoxLayout *topTopLayout = new QVBoxLayout( this );

      QWidget *topFrame = new QWidget( this );
      topTopLayout->addWidget( topFrame );

      QGridLayout *topLayout = new QGridLayout(topFrame,5,3);
      topLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidFont *timeBarFont =
          addWidFont( KOPrefs::instance()->timeBarFontItem(), topFrame,
                      KGlobal::locale()->formatTime( QTime( 12, 34 ) ) );
      topLayout->addWidget(timeBarFont->label(),0,0);
      topLayout->addWidget(timeBarFont->preview(),0,1);
      topLayout->addWidget(timeBarFont->button(),0,2);

      KPrefsWidFont *monthViewFont =
          addWidFont( KOPrefs::instance()->monthViewFontItem(), topFrame,
                      KGlobal::locale()->formatTime(QTime(12,34)) + " " +
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

      topLayout->setColStretch(1,1);
      topLayout->setRowStretch(4,1);

      load();
    }
};

extern "C"
{
  KCModule *create_korganizerconfigfonts( QWidget *parent, const char * )
  {
    return new KOPrefsDialogFonts( KOGlobals::self()->instance(), parent /*, "kcmkorganizerfonts"*/ );
  }
}


KOPrefsDialogColors::KOPrefsDialogColors( KInstance *inst, QWidget *parent )
      : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout(topFrame,7,2);
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
  Q3GroupBox *categoryGroup = new Q3GroupBox(1,Qt::Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,7,7,0,1);

  mCategoryCombo = new QComboBox(categoryGroup);
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  mCategoryCombo->setWhatsThis(
                   i18n( "Select here the event category you want to modify. "
                         "You can change the selected category color using "
                         "the button below." ) );
  connect(mCategoryCombo,SIGNAL(activated(int)),SLOT(updateCategoryColor()));

  mCategoryButton = new KColorButton(categoryGroup);
  mCategoryButton->setWhatsThis(
                   i18n( "Choose here the color of the event category selected "
                         "using the combo box above." ) );
  connect(mCategoryButton,SIGNAL(changed(const QColor &)),SLOT(setCategoryColor()));
  updateCategoryColor();

  // resources colors
  Q3GroupBox *resourceGroup = new Q3GroupBox(1,Qt::Horizontal,i18n("Resources"),
                                           topFrame);
  topLayout->addMultiCellWidget(resourceGroup,8,8,0,1);

  mResourceCombo = new QComboBox(resourceGroup);
  mResourceCombo->setWhatsThis(
                   i18n( "Select here resource you want to modify. "
                         "You can change the selected resource color using "
                         "the button below." ) );
  connect(mResourceCombo,SIGNAL(activated(int)),SLOT(updateResourceColor()));

  mResourceButton = new KColorButton(resourceGroup);
  mResourceButton->setWhatsThis(
                   i18n( "Choose here the color of the resource selected "
                         "using the combo box above." ) );
  connect(mResourceButton,SIGNAL(changed(const QColor &)),SLOT(setResourceColor()));
  updateResources();

  topLayout->setRowStretch(9,1);

  load();
}

void KOPrefsDialogColors::usrWriteConfig()
{
  Q3DictIterator<QColor> itCat(mCategoryDict);
  while (itCat.current()) {
    KOPrefs::instance()->setCategoryColor(itCat.currentKey(),*itCat.current());
    ++itCat;
  }

  Q3DictIterator<QColor> itRes(mResourceDict);
  while (itRes.current()) {
    KOPrefs::instance()->setResourceColor(itRes.currentKey(),*itRes.current());
    ++itRes;
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
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  updateCategoryColor();
}

void KOPrefsDialogColors::setCategoryColor()
{
  mCategoryDict.replace(mCategoryCombo->currentText(), new QColor(mCategoryButton->color()));
  slotWidChanged();
}

void KOPrefsDialogColors::updateCategoryColor()
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

void KOPrefsDialogColors::updateResources()
{
  mResourceCombo->clear();
  mResourceIdentifier.clear();
  kdDebug( 5850) << "KOPrefsDialogColors::updateResources()" << endl;

  KCal::CalendarResourceManager *manager = KOrg::StdCalendar::self()->resourceManager();

  kdDebug(5850) << "Loading Calendar resources...:" << endl;
  KCal::CalendarResourceManager::Iterator it;
  for( it = manager->begin(); it != manager->end(); ++it ) {
    if ( !(*it)->subresources().isEmpty() ) {
      QStringList subresources = (*it)->subresources();
      for ( int i = 0; i < subresources.count(); ++i ) {
        QString resource = subresources[ i ];
        if ( (*it)->subresourceActive( resource ) ) {
          mResourceCombo->insertItem( (*it)->labelForSubresource( resource ) );
          mResourceIdentifier.append( resource );
        }
      }
    }

    mResourceCombo->insertItem( (*it)->resourceName() );
    mResourceIdentifier.append( (*it)->identifier() );
  }

  updateResourceColor();
}

void KOPrefsDialogColors::setResourceColor()
{
  kdDebug( 5850) << "KOPrefsDialogColors::setResorceColor()" << endl;

  mResourceDict.replace( mResourceIdentifier[mResourceCombo->currentItem()],
    new QColor( mResourceButton->color() ) );
  slotWidChanged();
}

void KOPrefsDialogColors::updateResourceColor()
{
  kdDebug( 5850 ) << "KOPrefsDialogColors::updateResourceColor()" << endl;
  QString res= mResourceIdentifier[mResourceCombo->currentItem()];
  QColor *color = mCategoryDict.find(res);
  if( !color )  {
    color = KOPrefs::instance()->resourceColor( res );
  }
  if( color ) {
    mResourceButton->setColor(*color);
  }
}
extern "C"
{
  KCModule *create_korganizerconfigcolors( QWidget *parent, const char * )
  {
    return new KOPrefsDialogColors( KOGlobals::self()->instance(), parent /*, "kcmkorganizercolors"*/ );
  }
}


KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling( KInstance *inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  QBoxLayout *topTopLayout = new QVBoxLayout( this );

  QWidget *topFrame = new QWidget( this );
  topTopLayout->addWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *useGroupwareBool =
      addWidBool( KOPrefs::instance()->useGroupwareCommunicationItem(),
      topFrame );
  topLayout->addMultiCellWidget(useGroupwareBool->checkBox(),0,0,0,1);
  // FIXME: This radio button should only be available when KMail is chosen
//   connect(thekmailradiobuttonupthere,SIGNAL(toggled(bool)),
//           useGroupwareBool->checkBox(), SLOT(enabled(bool)));

  KPrefsWidBool *bcc =
      addWidBool( KOPrefs::instance()->bccItem(), topFrame );
  topLayout->addMultiCellWidget(bcc->checkBox(),1,1,0,1);

  KPrefsWidRadios *mailClientGroup =
      addWidRadios( KOPrefs::instance()->mailClientItem(), topFrame );
  topLayout->addMultiCellWidget(mailClientGroup->groupBox(),2,2,0,1);


#if 0
  KPrefsWidRadios *schedulerGroup =
      addWidRadios(i18n("Scheduler Mail Client"),KOPrefs::instance()->mIMIPScheduler,
                   topFrame);
  schedulerGroup->addRadio("Dummy"); // Only for debugging
  schedulerGroup->addRadio(i18n("Mail client"));

  topLayout->addMultiCellWidget(schedulerGroup->groupBox(),0,0,0,1);
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
  topLayout->addMultiCellWidget(aMailsLabel,3,3,0,1);
  mAMails = new Q3ListView(topFrame);
  mAMails->setWhatsThis( whatsThis );

  mAMails->addColumn(i18n("Email"),300);
  topLayout->addMultiCellWidget(mAMails,4,4,0,1);

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

  QPushButton *add = new QPushButton(i18n("New"),topFrame,"new");
  whatsThis = i18n( "Press this button to add a new entry to the "
                    "additional e-mail addresses list. Use the edit "
                    "box above to edit the new entry." );
  add->setWhatsThis( whatsThis );
  topLayout->addWidget(add,6,0);
  QPushButton *del = new QPushButton(i18n("Remove"),topFrame,"remove");
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
  KCModule *create_korganizerconfiggroupscheduling( QWidget *parent,
                                                     const char * )
  {
    return new KOPrefsDialogGroupScheduling( KOGlobals::self()->instance(), parent /*,
                                             "kcmkorganizergroupscheduling"*/ );
  }
}


KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling( KInstance *inst, QWidget *parent )
  : KPrefsModule( KOPrefs::instance(), inst, parent )
{
  mGroupwarePage = new KOGroupwarePrefsPage( this );
  connect( mGroupwarePage, SIGNAL( changed() ), SLOT( slotWidChanged() ) );
  ( new QVBoxLayout( this ) )->addWidget( mGroupwarePage );

  load();
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
  KCModule *create_korganizerconfigfreebusy( QWidget *parent, const char * )
  {
    return new KOPrefsDialogGroupwareScheduling( KOGlobals::self()->instance(), parent /*,
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
KOPrefsDialogPlugins::KOPrefsDialogPlugins( KInstance *inst, QWidget *parent )
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
  mDescription->setAlignment( Qt::TextHideMnemonic | Qt::TextWordWrap | Qt::AlignVCenter );
  mDescription->setFrameShape( QLabel::Panel );
  mDescription->setFrameShadow( QLabel::Sunken );
  mDescription->setMinimumSize( QSize( 0, 55 ) );
  mDescription->setSizePolicy(
         QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0,
                      0, 0, mDescription->sizePolicy().hasHeightForWidth() ) );
  topLayout->addWidget( mDescription );


  QWidget *buttonRow = new QWidget( topFrame );
  QBoxLayout *buttonRowLayout = new QHBoxLayout( buttonRow );
  mConfigureButton = new KPushButton( KGuiItem( i18n("Configure &Plugin..."),
      "configure", QString::null, i18n("This button allows you to configure"
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
//  usrReadConfig();
  selectionChanged( 0 );
}

void KOPrefsDialogPlugins::usrReadConfig()
{
  mListView->clear();
  KTrader::OfferList plugins = KOCore::self()->availablePlugins();
  plugins += KOCore::self()->availableParts();

  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  KTrader::OfferList::ConstIterator it;
  for( it = plugins.begin(); it != plugins.end(); ++it ) {
    Q3CheckListItem *item = new PluginItem( mListView, *it );
    if ( selectedPlugins.find( (*it)->desktopEntryName() ) !=
                               selectedPlugins.end() ) {
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
    mDescription->setText( QString::null );
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
  KCModule *create_korganizerconfigplugins( QWidget *parent, const char * )
  {
    return new KOPrefsDialogPlugins( KOGlobals::self()->instance(), parent /*,
                                     "kcmkorganizerplugins"*/ );
  }
}


extern "C"
{
  KCModule *create_korgdesignerfields( QWidget *parent, const char * ) {
    return new KOPrefsDesignerFields( KOGlobals::self()->instance(), parent /*, "kcmkorgdesignerfields"*/ );
  }
}

KOPrefsDesignerFields::KOPrefsDesignerFields( KInstance *inst, QWidget *parent )
  : KCMDesignerFields( inst, parent )
{
}

QString KOPrefsDesignerFields::localUiDir()
{
  QString dir = locateLocal( "data", "korganizer/designer/event/");
  kdDebug() << "KOPrefsDesignerFields::localUiDir(): " << dir << endl;
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
