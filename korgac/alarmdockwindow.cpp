/*
    KDE Panel docking window for KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

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

#include <stdlib.h>

#include <qtooltip.h>
#include <qfile.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "koalarmclient.h"
#include "alarmdaemoniface_stub.h"

#include "alarmdockwindow.h"
#include "alarmdockwindow.moc"


AlarmDockWindow::AlarmDockWindow(KOAlarmClient *client, QWidget *parent,
                                 const char *name)
  : KSystemTray(parent, name),
    mAlarmGui(client)
{
  // Read the GUI autostart status from the config file
  KConfig* config = kapp->config();
  config->setGroup("General");
  bool autostartGui = config->readBoolEntry( "Autostart", true );
  bool alarmsEnabled = config->readBoolEntry( "Enabled", true );

  // Set up GUI icons
  KGlobal::iconLoader()->addAppDir( "kalarmdgui" );
  mPixmapEnabled  = loadIcon( "kalarmdgui" );
  mPixmapDisabled = loadIcon( "kalarmdgui_disabled" );

  setPixmap( alarmsEnabled ? mPixmapEnabled : mPixmapDisabled );

  // Set up the context menu
  mAlarmsEnabledId = contextMenu()->insertItem(i18n("Alarms Enabled"),
                                              this, SLOT(toggleAlarmsEnabled()));
  mAutostartGuiId = contextMenu()->insertItem(i18n("Start Alarm Client at Login"),
                                              this, SLOT(toggleGuiAutostart()));
  contextMenu()->insertItem( i18n("Configure Alarm Client..."), this,
                             SLOT( configureAlarmDaemon() ) );
  
  contextMenu()->setItemChecked(mAutostartGuiId, autostartGui);
  contextMenu()->setItemChecked(mAlarmsEnabledId, alarmsEnabled);
}

AlarmDockWindow::~AlarmDockWindow()
{
}


/*
 * Called when the Alarms Enabled context menu item is selected.
 */
void AlarmDockWindow::toggleAlarmsEnabled()
{
  kdDebug() << "AlarmDockWindow::toggleAlarmsEnabled()" << endl;

  KConfig* config = kapp->config();
  config->setGroup("General");

  bool enabled = !contextMenu()->isItemChecked( mAlarmsEnabledId );
  contextMenu()->setItemChecked( mAlarmsEnabledId, enabled );
  setPixmap( enabled ? mPixmapEnabled : mPixmapDisabled );

  config->writeEntry( "Enabled", enabled );  
  config->sync();
}


/*
 * Set GUI autostart at login on or off, and set the context menu accordingly.
 */
void AlarmDockWindow::setGuiAutostart(bool on)
{
  kdDebug() << "setGuiAutostart()=" << int(on) << endl;

  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry("Autostart", on);
  config->sync();

  contextMenu()->setItemChecked(mAutostartGuiId, on);
}


/*
 * Called when the mouse is clicked over the panel icon.
 */
void AlarmDockWindow::mousePressEvent(QMouseEvent* e)
{
  if ( e->button() == LeftButton ) {
    kapp->startServiceByDesktopName( "korganizer", QString::null );
  } else {
    KSystemTray::mousePressEvent( e );
  }
}


void AlarmDockWindow::closeEvent(QCloseEvent*)
{
  kapp->quit();
}

void AlarmDockWindow::configureAlarmDaemon()
{
  kapp->startServiceByDesktopName( "kcmkded", QString::null );
}
