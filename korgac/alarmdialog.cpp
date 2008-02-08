/*
  This file is part of the KOrganizer alarm daemon.

  Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>

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
#include "koeventviewer.h"
#include <korganizer_interface.h>

#include <kcal/event.h>
#include <kconfig.h>
#include <Phonon/MediaObject>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kcombobox.h>
#include <kwindowsystem.h>
#include <klockfile.h>
#include <kpushbutton.h>
#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kvbox.h>

#include <QLabel>
#include <QFile>
#include <QSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QDataStream>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QtDBus/QtDBus>
#include <QProcess>
#include "alarmdialog.moc"

class AlarmListItem : public QTreeWidgetItem
{
  public:
    AlarmListItem( Incidence *incidence, QTreeWidget *parent ) :
      QTreeWidgetItem( parent ),
      mIncidence( incidence->clone() ),
      mNotified( false )
    {}

    ~AlarmListItem()
    {
      delete mIncidence;
    }

    Incidence *mIncidence;
    QDateTime mRemindAt;
    bool mNotified;
};

typedef QList<AlarmListItem *> ItemList;


AlarmDialog::AlarmDialog( QWidget *parent )
  : KDialog( parent/*, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_StaysOnTop |
                     Qt::WStyle_DialogBorder,*/ ),
                 mSuspendTimer(this)
{
  KIconLoader::global()->addAppDir( "kdepim" );

  QWidget *topBox = new QWidget( this);
  setMainWidget( topBox );
  setCaption( i18nc( "@title:window", "Reminder" ) );
  setButtons( Ok | User1 | User2 | User3 );
  setDefaultButton( User1 );
  setButtonText( Ok, i18nc( "@action:button", "Dismiss" ) );
  setButtonText( User1, i18nc( "@action:button", "Dismiss all" ) );
  setButtonText( User2, i18nc( "@action:button", "Edit..." ) );
  setButtonText( User3, i18nc( "@action:button", "Suspend" ) );
  QBoxLayout *topLayout = new QVBoxLayout( topBox );
  topLayout->setSpacing( spacingHint() );

  QLabel *label =
    new QLabel( i18nc( "@label", "The following events triggered reminders:" ), topBox );
  topLayout->addWidget( label );

  mIncidenceListView = new QTreeWidget( topBox );
  mIncidenceListView->setColumnCount( 2 );
  QStringList headerLabels =
    ( QStringList( i18nc( "@title:column reminder summary", "Summary" ) )
      << i18nc( "@title:column reminder due date/time", "Due" ) );
  mIncidenceListView->setHeaderLabels( headerLabels );
  mIncidenceListView->setAllColumnsShowFocus( true );
  mIncidenceListView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  topLayout->addWidget( mIncidenceListView );
  connect( mIncidenceListView, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()) );
  connect( mIncidenceListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
           SLOT(slotUser2()) );
  connect( mIncidenceListView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
           SLOT(showDetails()) );
  connect( mIncidenceListView, SIGNAL(itemSelectionChanged()), SLOT(showDetails()) );

  mDetailView = new KOEventViewer( topBox );
  topLayout->addWidget( mDetailView );

  KHBox *suspendBox = new KHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  topLayout->addWidget( suspendBox );

  QLabel *l = new QLabel( i18nc( "@label:spinbox", "Suspend &duration:"), suspendBox );
  mSuspendSpin = new QSpinBox( suspendBox );
  mSuspendSpin->setRange( 1, 9999 );
  mSuspendSpin->setValue( 5 );  // default suspend duration
  l->setBuddy( mSuspendSpin );

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of minutes", "minute(s)") );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of hours", "hour(s)") );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of days", "day(s)") );
  mSuspendUnit->addItem( i18nc( "@item:inlistbox suspend in terms of weeks", "week(s)") );
  connect( &mSuspendTimer, SIGNAL(timeout()), SLOT(wakeUp()) );

  // showButton( User2/*3*/, false );

  setMinimumSize( 300, 200 );
  connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
  connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));
  connect(this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()));
  connect(this, SIGNAL(user3Clicked()), this, SLOT(slotUser3()));
}

AlarmDialog::~AlarmDialog()
{
  mIncidenceListView->clear();
}

void AlarmDialog::addIncidence( Incidence *incidence, const QDateTime &reminderAt )
{
  AlarmListItem *item = new AlarmListItem( incidence, mIncidenceListView );
  item->setText( 0, incidence->summary() );
  item->mRemindAt = reminderAt;
  Todo *todo;
  if ( dynamic_cast<Event*>( incidence ) ) {
    item->setIcon( 0, SmallIcon( "view-calendar-day" ) );
    item->setText( 1, incidence->dtStartStr() );
  } else if ( (todo = dynamic_cast<Todo*>( incidence )) ) {
    item->setIcon( 0, SmallIcon( "view-calendar-tasks" ) );
    item->setText( 1, todo->dtDueStr() );
  }
  if ( activeCount() == 1 ) {// previously empty
    mIncidenceListView->clearSelection();
    item->setSelected( true );
  }
  showDetails();
}

void AlarmDialog::slotOk()
{
  ItemList selection = selectedItems();
  for ( ItemList::Iterator it = selection.begin(); it != selection.end(); ++it )
    delete *it;
  if ( activeCount() == 0 )
    accept();
  else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotUser1()
{
  dismissAll();
}

void AlarmDialog::suspend()
{
  if ( !isVisible() )
    return;

  int unit=1;
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

  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->isSelected() && !item->isHidden() ) {
      item->setHidden( true );
      item->setSelected( false );
      item->mRemindAt = QDateTime::currentDateTime().addSecs( unit * mSuspendSpin->value() );
      item->mNotified = false;
    }
  }

  setTimer();
  if ( activeCount() == 0 )
    accept();
  else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::setTimer()
{
  int nextReminderAt = -1;
  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->mRemindAt > QDateTime::currentDateTime() ) {
      int secs = QDateTime::currentDateTime().secsTo( item->mRemindAt );
      nextReminderAt = nextReminderAt <= 0 ? secs : qMin( nextReminderAt, secs );
    }
  }

  if ( nextReminderAt >= 0 ) {
    mSuspendTimer.stop();
    mSuspendTimer.start( 1000 * (nextReminderAt + 1), true );
  }
}

void AlarmDialog::slotUser2()
{
  if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.korganizer" ) ) {
    if ( KToolInvocation::startServiceByDesktopName( "korganizer", QString() ) )
      KMessageBox::error( 0, i18nc( "@info", "Could not start KOrganizer." ) );
  }
  org::kde::korganizer::Korganizer korganizer( "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  ItemList selection = selectedItems();
  if ( selection.count() != 1 )
    return;
  Incidence *incidence = selection.first()->mIncidence;
  korganizer.editIncidence( incidence->uid() );

#ifdef Q_WS_X11
#ifdef __GNUC__
#warning "kde4: verify it when kontact will not crash"
#endif
  // get desktop # where korganizer (or kontact) runs
  QString object = QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kontact" ) ?
           "kontact/mainwindow_1" : "korganizer/MainWindow_1";
  QDBusInterface korganizerObj("org.kde.korganizer", '/'+object);
  QDBusReply<int> reply = korganizerObj.call( "winId" );
  if ( reply.isValid() ) {
    int window = reply;
    int desktop = KWindowSystem::windowInfo( window, NET::WMDesktop ).desktop();

    if ( KWindowSystem::currentDesktop() == desktop ) {
      KWindowSystem::minimizeWindow( winId(), false );
    }
    else
      KWindowSystem::setCurrentDesktop( desktop );

    KWindowSystem::activateWindow( KWindowSystem::transientFor( window ) );
  }
#endif
}

void AlarmDialog::slotUser3()
{
  suspend();
}

void AlarmDialog::show()
{
  KDialog::show();
  KWindowSystem::setState( winId(), NET::KeepAbove );
  KWindowSystem::setOnAllDesktops( winId(), true );
  eventNotification();
}

void AlarmDialog::dismissAll()
{
  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->isHidden() )
      continue;
    delete item;
  }
  setTimer();
  accept();
  emit reminderCount( activeCount() );
}

void AlarmDialog::eventNotification()
{
  bool beeped = false;
  bool found = false;

  QList<AlarmListItem *> list;

  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->isHidden() || item->mNotified )
      continue;
    found = true;
    item->mNotified = true;
    Alarm::List alarms = item->mIncidence->alarms();
    Alarm::List::ConstIterator it;
    for ( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
      // FIXME: Check whether this should be done for all multiple alarms
      if (alarm->type() == Alarm::Procedure) {
        // FIXME: Add a message box asking whether the procedure should really be executed
      kDebug(5890) <<"Starting program: '" << alarm->programFile() <<"'";
      QProcess::startDetached( QFile::encodeName(alarm->programFile()));
      }
      else if (alarm->type() == Alarm::Audio) {
        beeped = true;
      Phonon::MediaObject* player = Phonon::createPlayer( Phonon::NotificationCategory, alarm->audioFile() );
      player->setParent( this );
      connect( player, SIGNAL( finished() ), player, SLOT( deleteLater() ) );
      player->play();
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
  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->mRemindAt <= QDateTime::currentDateTime() ) {
      if ( item->isHidden() ) {
        item->setHidden( false );
        item->setSelected( false );
      }
      activeReminders = true;
    } else {
      item->setHidden( true );
    }
  }

  if ( activeReminders )
    show();
  setTimer();
  showDetails();
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotSave()
{
  KSharedConfig::Ptr config = KGlobal::config();
  // KLockFile::Ptr lock = config->lockFile();
  // if ( lock.data()->lock() != KLockFile::LockOK )
  //   return;

  KConfigGroup generalConfig( config, "General" );
  int numReminders = generalConfig.readEntry("Reminders", 0);

  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    KConfigGroup incidenceConfig( config,
                                  QString("Incidence-%1").arg(numReminders) );
    incidenceConfig.writeEntry( "UID", item->mIncidence->uid() );
    incidenceConfig.writeEntry( "RemindAt", item->mRemindAt );
    ++numReminders;
  }

  generalConfig.writeEntry( "Reminders", numReminders );
  config->sync();
  // lock.data()->unlock();
}

void AlarmDialog::updateButtons()
{
  ItemList selection = selectedItems();
  enableButton( User2, selection.count() == 1 );
  enableButton( Ok, selection.count() > 0 );
  enableButton( User3, selection.count() > 0 );
}

QList<AlarmListItem *> AlarmDialog::selectedItems() const
{
  QList<AlarmListItem *> list;

  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( item->isSelected() )
      list.append( item );
  }
  return list;
}

int AlarmDialog::activeCount()
{
  int count = 0;
  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    AlarmListItem *item = static_cast<AlarmListItem *>( root->child( i ) );
    if ( !item->isHidden() )
      count++;
  }
  return count;
}

void AlarmDialog::suspendAll()
{
  mIncidenceListView->clearSelection();
  QTreeWidgetItem *const root = mIncidenceListView->invisibleRootItem();
  for ( int i = 0 ; i < root->childCount() ; i++ ) {
    QTreeWidgetItem *item = root->child( i );
    if ( !item->isHidden() )
      item->setSelected( true );
  }
  suspend();
}

void AlarmDialog::showDetails()
{
  mDetailView->clearEvents( true );
  mDetailView->clear();
  AlarmListItem *item = dynamic_cast<AlarmListItem *>( mIncidenceListView->currentItem() );
  if ( !item || item->isHidden() )
    return;
  mDetailView->appendIncidence( item->mIncidence );
}
