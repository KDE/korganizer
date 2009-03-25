/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008-2009 Allen Winter <winter@kde.org>

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

#include "alarmdockwindow.h"

#include <KAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <KDebug>
#include <KIconEffect>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KMenu>
#include <KStandardAction>
#include <KToolInvocation>

AlarmDockWindow::AlarmDockWindow()
  : KSystemTrayIcon( 0 )
{
  // Read the autostart status from the config file
  KConfigGroup config( KGlobal::config(), "General" );
  bool autostartSet = config.hasKey( "Autostart" );
  bool autostart = config.readEntry( "Autostart", true );
  bool alarmsEnabled = config.readEntry( "Enabled", true );

  mName = i18nc( "@title:window", "KOrganizer Reminder Daemon" );
  setToolTip( mName );

  // Set up icons
  KIconLoader::global()->addAppDir( "korgac" );
  KIconLoader::global()->addAppDir( "kdepim" );
  QString iconPath = KIconLoader::global()->iconPath( "korgac", KIconLoader::Panel );
  mIconEnabled  = loadIcon( iconPath );
  if ( mIconEnabled.isNull() ) {
    KMessageBox::sorry( parentWidget(),
                        i18nc( "@info", "Cannot load system tray icon." ) );
  } else {
    KIconLoader loader;
    QImage iconDisabled =
      mIconEnabled.pixmap( loader.currentSize( KIconLoader::Panel ) ).toImage();
    KIconEffect::toGray( iconDisabled, 1.0 );
    mIconDisabled = QIcon( QPixmap::fromImage( iconDisabled ) );
  }

  setIcon( alarmsEnabled ? mIconEnabled : mIconDisabled );

  // Set up the context menu
  mSuspendAll =
    contextMenu()->addAction( i18nc( "@action:inmenu", "Suspend All Reminders" ), this,
                              SLOT(slotSuspendAll()) );
  mDismissAll =
    contextMenu()->addAction( i18nc( "@action:inmenu", "Dismiss All Reminders" ), this,
                              SLOT(slotDismissAll()) );
  mSuspendAll->setEnabled( false );
  mDismissAll->setEnabled( false );

  contextMenu()->addSeparator();
  mAlarmsEnabled =
    contextMenu()->addAction( i18nc( "@action:inmenu", "Enable Reminders" ) );
  connect( mAlarmsEnabled, SIGNAL(toggled(bool)), SLOT(toggleAlarmsEnabled(bool)) );
  mAlarmsEnabled->setCheckable( true );

  mAutostart =
    contextMenu()->addAction( i18nc( "@action:inmenu", "Start Reminder Daemon at Login" ) );
  connect( mAutostart, SIGNAL(toggled(bool )), SLOT(toggleAutostart(bool)) );
  mAutostart->setCheckable( true );

  mAlarmsEnabled->setChecked( alarmsEnabled );
  mAutostart->setChecked( autostart );

  // Disable standard quit behaviour. We have to intercept the quit even,
  // if the main window is hidden.
  KActionCollection *ac = actionCollection();
  const char *quitName = KStandardAction::name( KStandardAction::Quit );
  QAction *quit = ac->action( quitName );
  if ( !quit ) {
    kDebug() << "No Quit standard action.";
  } else {
    quit->disconnect( SIGNAL(triggered(bool)), this, SLOT(maybeQuit()) );
    connect( quit, SIGNAL(activated()), SLOT(slotQuit()) );
  }

  connect( this, SIGNAL(activated( QSystemTrayIcon::ActivationReason )),
           SLOT(slotActivated( QSystemTrayIcon::ActivationReason )) );

  setToolTip( mName );
  mAutostartSet = autostartSet;
}

AlarmDockWindow::~AlarmDockWindow()
{
}

void AlarmDockWindow::slotUpdate( int reminders )
{
  bool actif = (reminders > 0);
  mSuspendAll->setEnabled( actif );
  mDismissAll->setEnabled( actif );
  if ( actif ) {
    setToolTip( i18ncp( "@info:tooltip",
                        "There is 1 active reminder.",
                        "There are %1 active reminders.", reminders ) );
  } else {
    setToolTip( i18nc( "@info:tooltip", "No active reminders." ) );
  }
}

void AlarmDockWindow::toggleAlarmsEnabled( bool checked )
{
  kDebug();

  setIcon( checked ? mIconEnabled : mIconDisabled );

  KConfigGroup config( KGlobal::config(), "General" );
  config.writeEntry( "Enabled", checked );
  config.sync();
}

void AlarmDockWindow::toggleAutostart( bool checked )
{
  kDebug();
  mAutostartSet = true;
  enableAutostart( checked );
}

void AlarmDockWindow::slotSuspendAll()
{
  emit suspendAllSignal();
}

void AlarmDockWindow::slotDismissAll()
{
  emit dismissAllSignal();
}

void AlarmDockWindow::enableAutostart( bool enable )
{
  KConfigGroup config( KGlobal::config(), "General" );
  config.writeEntry( "Autostart", enable );
  config.sync();
}

void AlarmDockWindow::slotActivated( QSystemTrayIcon::ActivationReason reason )
{
  if ( reason == QSystemTrayIcon::Trigger ) {
    KToolInvocation::startServiceByDesktopName( "korganizer", QString() );
  }
}

void AlarmDockWindow::slotQuit()
{
  if ( mAutostartSet == true ) {
    int result = KMessageBox::questionYesNo(
      parentWidget(),
      i18nc( "@info",
             "Do you want to quit the KOrganizer reminder daemon?<nl/>"
             "<note> you will not get calendar reminders unless the daemon is running.</note>" ),
      i18nc( "@title:window", "Close KOrganizer Reminder Daemon" ) );

    if ( result == KMessageBox::Yes ) {
      emit quitSignal();
    }
  } else {
    int result = KMessageBox::questionYesNoCancel(
      parentWidget(),
      i18nc( "@info",
             "Do you want to start the KOrganizer reminder daemon at login?<nl/>"
             "<note> you will not get calendar reminders unless the daemon is running.</note>" ),
      i18nc( "@title:window", "Close KOrganizer Reminder Daemon" ),
      KGuiItem( i18nc( "@action:button start the reminder daemon", "Start" ) ),
      KGuiItem( i18nc( "@action:button do not start the reminder daemon", "Do Not Start" ) ),
      KStandardGuiItem::cancel(),
      QString::fromLatin1( "AskForStartAtLogin" ) );

    bool autostart = true;
    if ( result == KMessageBox::No ) {
      autostart = false;
    }
    enableAutostart( autostart );

    if ( result != KMessageBox::Cancel ) {
      emit quitSignal();
    }
  }
}

#include "alarmdockwindow.moc"
