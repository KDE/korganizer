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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "alarmdockwindow.h"
#include "koalarmclient.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kstdaction.h>

#include <qtooltip.h>
#include <qfile.h>

#include <stdlib.h>

AlarmDockWindow::AlarmDockWindow( const char *name )
  : KSystemTray( 0, name )
{
  // Read the autostart status from the config file
  KConfig *config = kapp->config();
  config->setGroup("General");
  bool autostart = config->readBoolEntry( "Autostart", true );
  bool alarmsEnabled = config->readBoolEntry( "Enabled", true );

  // Set up icons
  KGlobal::iconLoader()->addAppDir( "korgac" );
  mPixmapEnabled  = loadIcon( "korgac" );
  mPixmapDisabled = loadIcon( "korgac_disabled" );

  setPixmap( alarmsEnabled ? mPixmapEnabled : mPixmapDisabled );

  // Set up the context menu
  mSuspendAll = contextMenu()->insertItem( i18n("Suspend All"), this, SLOT( slotSuspendAll() ) );
  mDismissAll = contextMenu()->insertItem( i18n("Dismiss All"), this, SLOT( slotDismissAll() ) );
  contextMenu()->setItemEnabled( mSuspendAll, false );
  contextMenu()->setItemEnabled( mDismissAll, false );

  contextMenu()->insertSeparator();
  mAlarmsEnabledId = contextMenu()->insertItem( i18n("Alarms Enabled"),
                                                this,
                                                SLOT( toggleAlarmsEnabled() ) );
  mAutostartId = contextMenu()->insertItem( i18n("Start Alarm Client at Login"),
                                            this,
                                            SLOT( toggleAutostart() ) );
  contextMenu()->setItemChecked( mAutostartId, autostart );
  contextMenu()->setItemChecked( mAlarmsEnabledId, alarmsEnabled );

  // Disable standard quit behaviour. We have to intercept the quit even, if the
  // main window is hidden.
  KActionCollection *ac = actionCollection();
  const char *quitName = KStdAction::name( KStdAction::Quit );
  KAction *quit = ac->action( quitName );
  if ( !quit ) {
    kdDebug(5890) << "No Quit standard action." << endl;
  } else {
    quit->disconnect( SIGNAL( activated() ), qApp,
                      SLOT( closeAllWindows() ) );
  }

  QToolTip::add(this, i18n( "KOrganizer Alarm Reminder" ) );


  connect( this, SIGNAL( quitSelected() ), SLOT( slotQuit() ) );
}

AlarmDockWindow::~AlarmDockWindow()
{
}

void AlarmDockWindow::slotUpdate( int reminders )
{
  QToolTip::remove( this );
  if ( reminders > 0 )
  {
    QToolTip::add( this, i18n( "There is 1 active reminder.",
                   "There are %n active reminders.", reminders ) );
    contextMenu()->setItemEnabled( mSuspendAll, true );
    contextMenu()->setItemEnabled( mDismissAll, true );
  }
  else
  {
    QToolTip::add( this, i18n( "KOrganizer Alarm Reminder" ) );
    contextMenu()->setItemEnabled( mSuspendAll, false );
    contextMenu()->setItemEnabled( mDismissAll, false );
  }
}

void AlarmDockWindow::toggleAlarmsEnabled()
{
  kdDebug(5890) << "AlarmDockWindow::toggleAlarmsEnabled()" << endl;

  KConfig *config = kapp->config();
  config->setGroup( "General" );

  bool enabled = !contextMenu()->isItemChecked( mAlarmsEnabledId );
  contextMenu()->setItemChecked( mAlarmsEnabledId, enabled );
  setPixmap( enabled ? mPixmapEnabled : mPixmapDisabled );

  config->writeEntry( "Enabled", enabled );  
  config->sync();
}

void AlarmDockWindow::toggleAutostart()
{
  bool autostart = !contextMenu()->isItemChecked( mAutostartId );

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
  KConfig *config = kapp->config();
  config->setGroup( "General" );
  config->writeEntry( "Autostart", enable );
  config->sync();

  contextMenu()->setItemChecked( mAutostartId, enable );
}

void AlarmDockWindow::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == LeftButton ) {
    kapp->startServiceByDesktopName( "korganizer", QString::null );
  } else {
    KSystemTray::mousePressEvent( e );
  }
}

//void AlarmDockWindow::closeEvent( QCloseEvent * )
void AlarmDockWindow::slotQuit()
{
  int result = KMessageBox::questionYesNoCancel( this,
      i18n("Do you want to start the KOrganizer alarm daemon at login "
           "(note that you will not get alarms whilst the daemon is not running)?"),
      i18n("Close KOrganizer Alarm Daemon"),
      KStdGuiItem::yes(), KStdGuiItem::no(), 
      QString::fromLatin1("AskForStartAtLogin")
      );

  bool autostart = true;
  if ( result == KMessageBox::No ) autostart = false;
  enableAutostart( autostart );

  if ( result != KMessageBox::Cancel )
    emit quitSignal();
}

#include "alarmdockwindow.moc"
