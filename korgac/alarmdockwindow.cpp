/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "alarmdockwindow.h"
#include "koalarmclient.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kstdaction.h>

#include <qtooltip.h>
#include <qfile.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <stdlib.h>
#include <ktoolinvocation.h>
#include <kglobal.h>

AlarmDockWindow::AlarmDockWindow()
  : KSystemTray( 0 )
{
  // Read the autostart status from the config file
  KConfig *config = KGlobal::config();
  config->setGroup("General");
  bool autostart = config->readEntry( "Autostart", true );
  bool alarmsEnabled = config->readEntry( "Enabled", true );

  mName = i18n( "KOrganizer Reminder Daemon" );
  setWindowTitle( mName );

  // Set up icons
  KGlobal::iconLoader()->addAppDir( "korgac" );
  mPixmapEnabled  = loadIcon( "korgac" );
  mPixmapDisabled = loadIcon( "korgac_disabled" );

  setPixmap( alarmsEnabled ? mPixmapEnabled : mPixmapDisabled );

  // Set up the context menu
  mSuspendAll = contextMenu()->addAction( i18n("Suspend All"), this, SLOT( slotSuspendAll() ) );
  mDismissAll = contextMenu()->addAction( i18n("Dismiss All"), this, SLOT( slotDismissAll() ) );
  mSuspendAll->setEnabled( false );
  mDismissAll->setEnabled( false );

  contextMenu()->addSeparator();
  mAlarmsEnabled = contextMenu()->addAction( i18n("Reminders Enabled"), this,
                                                SLOT( toggleAlarmsEnabled() ) );
  mAutostart = contextMenu()->addAction( i18n("Start Reminder Daemon at Login"), this,
                                                SLOT( toggleAutostart() ) );
  mAutostart->setEnabled( autostart );
  mAlarmsEnabled->setEnabled( alarmsEnabled );

  // Disable standard quit behaviour. We have to intercept the quit even, if the
  // main window is hidden.
  KActionCollection *ac = actionCollection();
  const char *quitName = KStdAction::name( KStdAction::Quit );
  KAction *quit = ac->action( quitName );
  if ( !quit ) {
    kDebug(5890) << "No Quit standard action." << endl;
  } else {
    quit->disconnect( SIGNAL( activated() ), this,
                      SLOT( maybeQuit() ) );
    connect( quit, SIGNAL( activated() ), SLOT( slotQuit() ) );
  }

  setToolTip( mName );
}

AlarmDockWindow::~AlarmDockWindow()
{
}

void AlarmDockWindow::slotUpdate( int reminders )
{
  mSuspendAll->setEnabled( reminders > 0 );
  mDismissAll->setEnabled( reminders > 0 );
  if ( reminders > 0 )
  {
    setToolTip( i18np( "There is 1 active reminder.",
                   "There are %n active reminders.", reminders ) );
  } else {
    setToolTip( mName );
  }
}

void AlarmDockWindow::toggleAlarmsEnabled()
{
  kDebug(5890) << "AlarmDockWindow::toggleAlarmsEnabled()" << endl;

  KConfig *config = KGlobal::config();
  config->setGroup( "General" );

  bool enabled = !mAlarmsEnabled->isChecked();
  mAlarmsEnabled->setChecked( enabled );
  setPixmap( enabled ? mPixmapEnabled : mPixmapDisabled );

  config->writeEntry( "Enabled", enabled );
  config->sync();
}

void AlarmDockWindow::toggleAutostart()
{
  bool autostart = !mAutostart->isChecked();
  enableAutostart( autostart );
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
  KConfig *config = KGlobal::config();
  config->setGroup( "General" );
  config->writeEntry( "Autostart", enable );
  config->sync();

  mAutostart->setChecked( enable );
}

void AlarmDockWindow::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton ) {
    KToolInvocation::startServiceByDesktopName( "korganizer", QString() );
  } else {
    KSystemTray::mousePressEvent( e );
  }
}

//void AlarmDockWindow::closeEvent( QCloseEvent * )
void AlarmDockWindow::slotQuit()
{
  int result = KMessageBox::questionYesNoCancel( this,
      i18n("Do you want to start the KOrganizer reminder daemon at login "
           "(note that you will not get reminders whilst the daemon is not running)?"),
      i18n("Close KOrganizer Reminder Daemon"),
      i18n("Start"), i18n("Do Not Start"),
      QString::fromLatin1("AskForStartAtLogin")
      );

  bool autostart = true;
  if ( result == KMessageBox::No ) autostart = false;
  enableAutostart( autostart );

  if ( result != KMessageBox::Cancel )
    emit quitSignal();
}

#include "alarmdockwindow.moc"
