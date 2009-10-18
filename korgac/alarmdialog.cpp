/*
  This file is part of the KOrganizer reminder daemon.

  Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008-2009 Allen Winter <winter@kde.org>
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "alarmdialog.h"
#include "kocore.h"
#include "koeventviewer.h"
#include "komailclient.h"
#include "koprefs.h"
#include "korganizer_interface.h"

#include <KCal/Event>
#include <KCal/Incidence>
#include <KCal/IncidenceFormatter>
#include <KCal/Todo>

#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>

#include <KComboBox>
#include <KDebug>
#include <KHBox>
#include <KLocale>
#include <KMessageBox>
#include <KNotification>
#include <KSharedConfig>
#include <KSystemTimeZone>
#include <KToolInvocation>
#include <KWindowSystem>

#include <QLabel>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <Phonon/MediaObject>

class ReminderListItem : public QTreeWidgetItem
{
  public:
    ReminderListItem( Incidence *incidence, QTreeWidget *parent ) :
      QTreeWidgetItem( parent ),
      mIncidence( incidence->clone() ),
      mNotified( false )
    {}

    ~ReminderListItem()
    {
      delete mIncidence;
    }

    bool operator < ( const QTreeWidgetItem & other ) const;

    QString mDisplayText;
    Incidence *mIncidence;
    QDateTime mRemindAt;
    KDateTime mTrigger;
    KDateTime mHappening;
    bool mNotified;
};

bool ReminderListItem::operator < ( const QTreeWidgetItem &other ) const
{
  switch( treeWidget()->sortColumn() ) {
  case 1: // happening datetime
  {
    const ReminderListItem *item = static_cast<const ReminderListItem *>( &other );
    return mHappening < item->mHappening;
  }
  case 2: // trigger datetime
  {
    const ReminderListItem *item = static_cast<const ReminderListItem *>( &other );
    return mTrigger < item->mTrigger;
  }
  default:
    return QTreeWidgetItem::operator < ( other );
  }
}

typedef QList<ReminderListItem *> ReminderList;

AlarmDialog::AlarmDialog( KCal::Calendar *calendar, QWidget *parent )
  : KDialog( parent, Qt::WindowStaysOnTopHint ),
    mCalendar( calendar ), mSuspendTimer( this )
{
  // User1 => Edit...
  // User2 => Dismiss All
  // User3 => Dismiss Selected
  //    Ok => Suspend

  KIconLoader::global()->addAppDir( "korgac" );

  KSharedConfig::Ptr config = KGlobal::config();
  KConfigGroup generalConfig( config, "General" );
  QPoint pos = generalConfig.readEntry( "Position", QPoint( 0, 0 ) );

  QWidget *topBox = new QWidget( this );
  if ( !pos.isNull() ) {
    mPos = pos;
    topBox->move( mPos );
  }
  setMainWidget( topBox );
  setCaption( i18nc( "@title:window", "Reminders" ) );
  setWindowIcon( KIcon( "korgac" ) );
  setButtons( Ok | User1 | User2 | User3 );
  setDefaultButton( User3 );
  setButtonText( User3, i18nc( "@action:button", "Dismiss Reminder" ) );
  setButtonToolTip( User3, i18nc( "@info:tooltip",
                                  "Dismiss the reminders for the selected incidences" ) );
  setButtonText( User2, i18nc( "@action:button", "Dismiss All" ) );
  setButtonToolTip( User2, i18nc( "@info:tooltip",
                                  "Dismiss the reminders for all listed incidences" ) );
  setButtonText( User1, i18nc( "@action:button", "Edit..." ) );
  setButtonToolTip( User1, i18nc( "@info:tooltip",
                                  "Edit the selected incidence" ) );
  setButtonText( Ok, i18nc( "@action:button", "Suspend" ) );
  setButtonToolTip( Ok, i18nc( "@info:tooltip",
                               "Suspend the reminders for the selected incidences "
                               "by the specified interval" ) );

  // Try to keep the dialog small and non-obtrusive.
  setMinimumWidth( 575 );
  setMinimumHeight( 300 );

  QVBoxLayout *mTopLayout = new QVBoxLayout( topBox );

  QLabel *label = new QLabel(
    i18nc( "@label",
           "Reminders: "
           "Click on a title to toggle the details viewer for that item" ),
    topBox );
  mTopLayout->addWidget( label );

  mIncidenceTree = new QTreeWidget( topBox );
  mIncidenceTree->setColumnCount( 3 );
  mIncidenceTree->setSortingEnabled( true );
  QStringList headerLabels =
    ( QStringList( i18nc( "@title:column reminder title", "Title" ) )
      << i18nc( "@title:column happens at date/time", "Date Time" )
      << i18nc( "@title:column trigger date/time", "Trigger Time" ) );
  mIncidenceTree->setHeaderLabels( headerLabels );
  mIncidenceTree->headerItem()->setToolTip(
    0,
    i18nc( "@info:tooltip", "The event or to-do title" ) );
  mIncidenceTree->headerItem()->setToolTip(
    1,
    i18nc( "@info:tooltip", "The reminder is set for this date/time" ) );
  mIncidenceTree->headerItem()->setToolTip(
    2,
    i18nc( "@info:tooltip", "The date/time the reminder was triggered" ) );

  mIncidenceTree->setWordWrap( true );
  mIncidenceTree->setAllColumnsShowFocus( true );
  mIncidenceTree->setSelectionMode( QAbstractItemView::ExtendedSelection );

  mTopLayout->addWidget( mIncidenceTree );

  connect( mIncidenceTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(update()) );
  connect( mIncidenceTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
           SLOT(update()) );
  connect( mIncidenceTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           SLOT(toggleDetails(QTreeWidgetItem*,int)) );
  connect( mIncidenceTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           SLOT(update()) );
  connect( mIncidenceTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(edit()) );
  connect( mIncidenceTree, SIGNAL(itemSelectionChanged()),
           SLOT(update()) );

  mDetailView = new KOEventViewer( mCalendar, topBox );
  QString s;
  s = i18nc( "@info default incidence details string",
             "<emphasis>Select an event or to-do from the list above "
             "to view its details here.</emphasis>" );
  mDetailView->setDefaultText( s );
  mDetailView->setIncidence( 0, QDate() );
  mTopLayout->addWidget( mDetailView );
  mDetailView->hide();
  mLastItem = 0;

  KHBox *suspendBox = new KHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  mTopLayout->addWidget( suspendBox );

  QLabel *l = new QLabel( i18nc( "@label:spinbox", "Suspend &duration:" ), suspendBox );

  mSuspendSpin = new QSpinBox( suspendBox );
  mSuspendSpin->setRange( 1, 9999 );
  mSuspendSpin->setValue( 5 );  // default suspend duration
  mSuspendSpin->setToolTip(
    i18nc( "@info:tooltip",
           "Suspend the reminders by this amount of time" ) );
  mSuspendSpin->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Each reminder for the selected incidences will be suspended "
           "by this number of time units. You can choose the time units "
           "(typically minutes) in the adjacent selector." ) );

  l->setBuddy( mSuspendSpin );

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of minutes", "minute(s)" ) );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of hours", "hour(s)" ) );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of days", "day(s)" ) );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of weeks", "week(s)" ) );
  mSuspendUnit->setToolTip(
    i18nc( "@info:tooltip",
           "Suspend the reminders using this time unit" ) );
  mSuspendUnit->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Each reminder for the selected incidences will be suspended "
           "using this time unit. You can set the number of time units "
           "in the adjacent number entry input." ) );

  connect( &mSuspendTimer, SIGNAL(timeout()), SLOT(wakeUp()) );

  connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()) );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
  connect( this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()) );
  connect( this, SIGNAL(user3Clicked()), this, SLOT(slotUser3()) );
}

AlarmDialog::~AlarmDialog()
{
  mIncidenceTree->clear();
}

void AlarmDialog::addIncidence( Incidence *incidence,
                                const QDateTime &reminderAt,
                                const QString &displayText )
{
  ReminderListItem *item = new ReminderListItem( incidence, mIncidenceTree );
  kDebug() << "adding incidence " << incidence->summary();
  QString summStr = incidence->summary();
  summStr.truncate( 30 );
  item->setText( 0, summStr );
  item->mRemindAt = reminderAt;
  item->mTrigger = KDateTime::currentLocalDateTime();
  item->mDisplayText = displayText;

  //TODO: this function needs to consider all Display type alarms in each incidence.

  Event *event;
  Todo *todo;
  Alarm *alarm = incidence->alarms().first();
  if ( ( event = dynamic_cast<Event *>( incidence ) ) ) {
    item->setIcon( 0, SmallIcon( "view-calendar-day" ) );
    if ( event->recurs() ) {
      KDateTime nextStart = event->recurrence()->getNextDateTime(
        KDateTime( reminderAt, KDateTime::Spec::LocalZone() ) );
      if ( nextStart.isValid() ) {
        item->mHappening = nextStart.toLocalZone();
        item->setText( 1, KGlobal::locale()->formatDateTime( nextStart.toLocalZone() ) );
      }
    }
    if ( item->text( 1 ).isEmpty() ) {
      KDateTime kdt;
      if ( alarm->hasStartOffset() ) {
        kdt = event->dtStart();
      } else {
        kdt = event->dtEnd();
      }
      item->mHappening = kdt;
      item->setText( 1, IncidenceFormatter::dateTimeToString(
                       kdt, false, true, KDateTime::Spec::LocalZone() ) );
    }
  } else if ( ( todo = dynamic_cast<Todo *>( incidence ) ) ) {
    item->setIcon( 0, SmallIcon( "view-calendar-tasks" ) );
    if ( todo->recurs() ) {
      KDateTime nextStart = todo->recurrence()->getNextDateTime(
        KDateTime( reminderAt, KDateTime::Spec::LocalZone() ) );
      if ( nextStart.isValid() ) {
        item->mHappening = nextStart.toLocalZone();
        item->setText( 1, KGlobal::locale()->formatDateTime( nextStart.toLocalZone() ) );
      }
    }
    if ( item->text( 1 ).isEmpty() ) {
      KDateTime kdt;
      if ( alarm->hasStartOffset() && todo->dtStart().isValid() ) {
        kdt = todo->dtStart();
      } else {
        kdt = todo->dtDue();
      }
      item->mHappening = kdt;
      item->setText( 1, IncidenceFormatter::dateTimeToString(
                       kdt, false, true, KDateTime::Spec::LocalZone() ) );
    }
  }
  item->setText( 2, IncidenceFormatter::dateTimeToString(
                   item->mTrigger, false, true, KDateTime::Spec::LocalZone() ) );

  QString tip = IncidenceFormatter::toolTipStr( mCalendar, incidence,
                                                item->mRemindAt.date(), true,
                                                KDateTime::Spec::LocalZone() );
  if ( !item->mDisplayText.isEmpty() ) {
    tip += "<br>" + item->mDisplayText;
  }
  item->setToolTip( 0, tip );
  item->setToolTip( 1, tip );
  item->setToolTip( 2, tip );
  item->setData( 0, QTreeWidgetItem::UserType, false );

  mIncidenceTree->setCurrentItem( item );
  showDetails();
}

void AlarmDialog::slotOk()
{
  suspend();
}

void AlarmDialog::slotUser1()
{
  edit();
}

void AlarmDialog::slotUser2()
{
  dismissAll();
}

void AlarmDialog::slotUser3()
{
  dismissCurrent();
}

void AlarmDialog::dismissCurrent()
{
  ReminderList selection = selectedItems();
  for ( ReminderList::Iterator it = selection.begin(); it != selection.end(); ++it ) {
    kDebug() << "removing " << ( *it )->mIncidence->summary();
    if ( mIncidenceTree->itemBelow( *it ) ) {
      mIncidenceTree->setCurrentItem( mIncidenceTree->itemBelow( *it ) );
    } else if ( mIncidenceTree->itemAbove( *it ) ) {
      mIncidenceTree->setCurrentItem( mIncidenceTree->itemAbove( *it ) );
    }
    mIncidenceTree->removeItemWidget( *it, 0 );
    delete *it;
  }

  if ( activeCount() == 0 ) {
    accept();
  } else {
    update();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::dismissAll()
{
  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    if ( !(*it)->isDisabled() ) { //do not disable suspended reminders
      delete *it;
    }
    ++it;
  }
  setTimer();
  accept();
  emit reminderCount( activeCount() );
}

void AlarmDialog::edit()
{
  ReminderList selection = selectedItems();
  if ( selection.count() != 1 ) {
    return;
  }
  Incidence *incidence = selection.first()->mIncidence;
  if ( incidence->isReadOnly() ) {
    KMessageBox::sorry(
      this,
      i18nc( "@info",
             "\"%1\" is a read-only item so modifications are not possible.",
             incidence->summary() ) );
    return;
  }

  if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.korganizer" ) ) {
    if ( KToolInvocation::startServiceByDesktopName( "korganizer", QString() ) ) {
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Could not start KOrganizer so editing is not possible." ) );
      return;
    }
  }
  org::kde::korganizer::Korganizer korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );

  kDebug() << "editing incidence " << incidence->summary();
  if ( !korganizer.editIncidence( incidence->uid() ) ) {
    KMessageBox::error(
      this,
      i18nc( "@info",
             "An internal KOrganizer error occurred attempting to modify \"%1\"",
             incidence->summary() ) );
  }

#ifdef Q_WS_X11
  // get desktop # where korganizer (or kontact) runs
  QString object =
    QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kontact" ) ?
    "kontact/mainwindow_1" : "korganizer/MainWindow_1";
  QDBusInterface korganizerObj( "org.kde.korganizer", '/' + object );
  QDBusReply<int> reply = korganizerObj.call( "winId" );
  if ( reply.isValid() ) {
    int window = reply;
    int desktop = KWindowSystem::windowInfo( window, NET::WMDesktop ).desktop();

    if ( KWindowSystem::currentDesktop() == desktop ) {
      KWindowSystem::minimizeWindow( winId(), false );
    } else {
      KWindowSystem::setCurrentDesktop( desktop );
    }
    KWindowSystem::activateWindow( KWindowSystem::transientFor( window ) );
  }
#endif
}

void AlarmDialog::suspend()
{
  if ( !isVisible() ) { //do nothing if the dialog is hidden
    return;
  }

  int unit = 1;
  switch ( mSuspendUnit->currentIndex() ) {
    case 3: // weeks
      unit *=  7;
    case 2: // days
      unit *= 24;
    case 1: // hours
      unit *= 60;
    case 0: // minutes
      unit *= 60;
    default:
      break;
  }

  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    if ( (*it)->isSelected() && !(*it)->isDisabled() ) { //suspend selected, non-suspended reminders
      (*it)->setSelected( false );
      (*it)->setDisabled( true );
      ReminderListItem *item = static_cast<ReminderListItem *>( *it );
      item->mRemindAt = QDateTime::currentDateTime().addSecs( unit * mSuspendSpin->value() );
      item->mHappening = KDateTime( item->mRemindAt, KDateTime::Spec::LocalZone() );
      item->mNotified = false;
      (*it)->setText( 1, KGlobal::locale()->formatDateTime( item->mHappening ) );
    }
    ++it;
  }

  setTimer();
  if ( activeCount() == 0 ) {
    accept();
  } else {
    update();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::setTimer()
{
  int nextReminderAt = -1;

  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    ReminderListItem *item = static_cast<ReminderListItem *>( *it );
    if ( item->mRemindAt > QDateTime::currentDateTime() ) {
      int secs = QDateTime::currentDateTime().secsTo( item->mRemindAt );
      nextReminderAt = nextReminderAt <= 0 ? secs : qMin( nextReminderAt, secs );
    }
    ++it;
  }

  if ( nextReminderAt >= 0 ) {
    mSuspendTimer.stop();
    mSuspendTimer.start( 1000 * ( nextReminderAt + 1 ) );
    mSuspendTimer.setSingleShot( true );
  }
}

void AlarmDialog::show()
{
  mIncidenceTree->resizeColumnToContents( 0 );
  mIncidenceTree->resizeColumnToContents( 1 );
  mIncidenceTree->resizeColumnToContents( 2 );
  mIncidenceTree->sortItems( 1, Qt::AscendingOrder );
  mIncidenceTree->setCurrentItem( mIncidenceTree->topLevelItem( 0 ) );

  KDialog::show();
  if ( !mPos.isNull() ) {
    KDialog::move( mPos );
  }
  KWindowSystem::setState( winId(), NET::KeepAbove );
  KWindowSystem::setOnAllDesktops( winId(), true );

  // Audio, Procedure, and EMail alarms
  eventNotification();
}

void AlarmDialog::suspendAll()
{
  mIncidenceTree->clearSelection();
  QTreeWidgetItemIterator it( mIncidenceTree );

  // first, select all non-suspended reminders
  while ( *it ) {
    if ( !(*it)->isDisabled() ) { //do not suspend suspended reminders
      (*it)->setSelected( true );
    }
    ++it;
  }

  //suspend all selected reminders
  suspend();
}

void AlarmDialog::eventNotification()
{
  bool beeped = false;
  bool found = false;

  ReminderList list;

  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    ReminderListItem *item = static_cast<ReminderListItem *>( *it );
    ++it;
    if ( item->isDisabled() || item->mNotified ) {
      //skip suspended reminders or reminders that have been notified
      continue;
    }
    found = true;
    item->mNotified = true;
    Alarm::List alarms = item->mIncidence->alarms();
    Alarm::List::ConstIterator ait;
    for ( ait = alarms.constBegin(); ait != alarms.constEnd(); ++ait ) {
      Alarm *alarm = *ait;
      // FIXME: Check whether this should be done for all multiple alarms
      if ( alarm->type() == Alarm::Procedure ) {
        // FIXME: Add a message box asking whether the procedure should really be executed
        kDebug() << "Starting program: '" << alarm->programFile() << "'";

        QString program = alarm->programFile();

        // if the program name contains spaces escape it
        if ( program.contains( ' ' )   &&
             !( program.startsWith( '\"' ) && program.endsWith( '\"' ) ) ) {
          program = '\"' + program + '\"';
        }

        QProcess::startDetached( program + ' ' + alarm->programArguments() );
      } else if ( alarm->type() == Alarm::Audio ) {
        beeped = true;
        Phonon::MediaObject *player =
          Phonon::createPlayer( Phonon::NotificationCategory, alarm->audioFile() );
        player->setParent( this );
        connect( player, SIGNAL(finished()), player, SLOT(deleteLater()) );
        player->play();
      } else if ( alarm->type() == Alarm::Email ) {
        QString from = KOPrefs::instance()->email();
        Identity id = KOCore::self()->identityManager()->identityForAddress( from );
        QString to;
        if ( alarm->mailAddresses().isEmpty() ) {
          to = from;
        } else {
          QList<Person> addresses = alarm->mailAddresses();
          QStringList add;
          for ( QList<Person>::ConstIterator it = addresses.constBegin();
                it != addresses.constEnd(); ++it ) {
            add << (*it).fullName();
          }
          to = add.join( ", " );
        }

        QString subject;
        if ( alarm->mailSubject().isEmpty() ) {
          if ( alarm->parent()->summary().isEmpty() ) {
            subject = i18nc( "@title", "Reminder" );
          } else {
            subject = i18nc( "@title", "Reminder: %1", alarm->parent()->summary() );
          }
        } else {
          subject = i18nc( "@title", "Reminder: %1", alarm->mailSubject() );
        }

        QString body;
        if ( alarm->mailText().isEmpty() ) {
          body = IncidenceFormatter::mailBodyStr(
            alarm->parent(), KSystemTimeZones::local() );
        } else {
          body = alarm->mailText();
        }
        //TODO: support attachments
        //TODO: transport
        KOMailClient mailer;
        mailer.send( id, from, to, QString(), subject, body, true );
      }
    }
  }

  if ( !beeped && found ) {
    KNotification::beep();
  }
}

void AlarmDialog::wakeUp()
{
  bool activeReminders = false;
  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    ReminderListItem *item = static_cast<ReminderListItem *>( *it );
    if ( item->mRemindAt <= QDateTime::currentDateTime() ) {
      if ( item->isDisabled() ) { //do not wakeup non-suspended reminders
        item->setDisabled( false );
        item->setSelected( false );
      }
      activeReminders = true;
    } else {
      item->setDisabled( true );
    }
    ++it;
  }

  if ( activeReminders ) {
    show();
  }
  setTimer();
  showDetails();
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotSave()
{
  KSharedConfig::Ptr config = KGlobal::config();
  KConfigGroup generalConfig( config, "General" );
  int numReminders = generalConfig.readEntry( "Reminders", 0 );

  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    ReminderListItem *item = static_cast<ReminderListItem *>( *it );
    KConfigGroup incidenceConfig( config,
                                  QString( "Incidence-%1" ).arg( numReminders + 1 ) );
    incidenceConfig.writeEntry( "UID", item->mIncidence->uid() );
    incidenceConfig.writeEntry( "RemindAt", item->mRemindAt );
    ++numReminders;
    ++it;
  }

  generalConfig.writeEntry( "Reminders", numReminders );
  generalConfig.writeEntry( "Position", pos() );
  config->sync();
}

ReminderList AlarmDialog::selectedItems() const
{
  ReminderList list;

  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    if ( (*it)->isSelected() ) {
      list.append( static_cast<ReminderListItem *>( *it ) );
    }
    ++it;
  }
  return list;
}

int AlarmDialog::activeCount()
{
  int count = 0;
  QTreeWidgetItemIterator it( mIncidenceTree );
  while ( *it ) {
    if ( !(*it)->isDisabled() ) { //suspended reminders are non-active
      ++count;
    }
    ++it;
  }
  kDebug() << "computed " << count << " active reminders";
  return count;
}

void AlarmDialog::updateButtons()
{
  int count = selectedItems().count();
  kDebug() << "selected items=" << count;
  enableButton( User3, count > 0 );  // enable Dismiss, if >1 selected
  enableButton( User1, count == 1 ); // enable Edit, if only 1 selected
  enableButton( Ok, count > 0 );     // enable Suspend, if >1 selected
}

void AlarmDialog::toggleDetails( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column );
  if ( !mDetailView->isHidden() ) {
    if ( mLastItem == item ) {
      resize( size().width(), size().height() - mDetailView->height() - 50 );
      mDetailView->hide();
    }
  } else {
    resize( size().width(), size().height() + mDetailView->height() + 50 );
    mDetailView->show();
  }
  mLastItem = item;
}

void AlarmDialog::showDetails()
{
  mDetailView->clearEvents( true );
  mDetailView->clear();
  ReminderListItem *item = dynamic_cast<ReminderListItem *>( mIncidenceTree->currentItem() );
  if ( !item ) {
    mDetailView->setIncidence( 0, QDate() );
  } else {
    if ( !item->mDisplayText.isEmpty() ) {
      QString txt = "<qt><p><b>" + item->mDisplayText + "</b></p></qt>";
      mDetailView->addText( txt );
    }
    mDetailView->appendIncidence( item->mIncidence, item->mRemindAt.date() );
  }
}

void AlarmDialog::update()
{
  updateButtons();
  showDetails();
}

void AlarmDialog::accept()
{
  if ( activeCount() == 0 ) {
    mPos = pos();
    hide();
  }
}

#include "alarmdialog.moc"
