/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)

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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "korganizer.h"

#include "komailclient.h"
#include "calendarview.h"
#include "koviewmanager.h"
#include "kodialogmanager.h"
#include "kowindowlist.h"
#include "koprefs.h"
#include "kocore.h"
#include "konewstuff.h"
#include "actionmanager.h"
#include "koglobals.h"
#include "alarmclient.h"
#include "resourceview.h"
#include "korganizerifaceimpl.h"

#include <korganizer/part.h>

#include <libkdepim/statusbarprogresswidget.h>
#include <libkdepim/progressdialog.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <kprocess.h>
#include <kwin.h>
#include <ktip.h>
#include <kstdguiitem.h>
#include <kstatusbar.h>

#include <qcursor.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>

#include <stdlib.h>

using namespace KParts;
#include "korganizer.moc"
using namespace KOrg;

KOrganizer::KOrganizer( const char *name )
  : KParts::MainWindow( 0, name ),
    KOrg::MainWindow()
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  kdDebug(5850) << "KOrganizer::KOrganizer()" << endl;
  KOCore::self()->addXMLGUIClient( this, this );
//  setMinimumSize(600,400);  // make sure we don't get resized too small...

  mCalendarView = new CalendarView( this, "KOrganizer::CalendarView" );
  setCentralWidget(mCalendarView);

  mActionManager = new ActionManager( this, mCalendarView, this, this, false );
  (void)new KOrganizerIfaceImpl( mActionManager, this, "IfaceImpl" );
}

KOrganizer::~KOrganizer()
{
  delete mActionManager;

  KOCore::self()->removeXMLGUIClient( this );
}

void KOrganizer::init( bool document )
{
  kdDebug(5850) << "KOrganizer::init() "
            << ( document ? "hasDocument" : "resources" ) << endl;

  setHasDocument( document );

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  if ( hasDocument() ) {
    mActionManager->createCalendarLocal();
  } else {
    mActionManager->createCalendarResources();
    setCaption( i18n("Calendar") );
  }

  mActionManager->init();
  connect( mActionManager, SIGNAL( actionNew( const KURL & ) ),
           SLOT( newMainWindow( const KURL & ) ) );

  mActionManager->loadParts();

  initActions();
  readSettings();

  KStatusBar *bar = statusBar();

  bar->insertItem( "", ID_GENERAL, 10 );

  bar->insertItem( i18n(" Incoming messages: %1 ").arg( 0 ), ID_MESSAGES_IN );
  bar->insertItem( i18n(" Outgoing messages: %2 ").arg( 0 ), ID_MESSAGES_OUT );
  bar->setItemAlignment( ID_MESSAGES_IN, AlignRight );
  bar->setItemAlignment( ID_MESSAGES_OUT, AlignRight );
  connect( bar, SIGNAL( pressed( int ) ), SLOT( statusBarPressed( int ) ) );

  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( bar, this );
  progressDialog->hide();

  KPIM::StatusbarProgressWidget *progressWidget;
  progressWidget = new KPIM::StatusbarProgressWidget( progressDialog, bar );
  progressWidget->show();

  bar->addWidget( progressWidget, 0, true );

  connect( mActionManager->view(), SIGNAL( numIncomingChanged( int ) ),
           SLOT( setNumIncoming( int ) ) );
  connect( mActionManager->view(), SIGNAL( numOutgoingChanged( int ) ),
           SLOT( setNumOutgoing( int ) ) );

  connect( mActionManager->view(), SIGNAL( statusMessage( const QString & ) ),
           SLOT( showStatusMessage( const QString & ) ) );

  setStandardToolBarMenuEnabled( true );

  kdDebug(5850) << "KOrganizer::KOrganizer() done" << endl;
}

void KOrganizer::newMainWindow( const KURL &url )
{
  KOrganizer *korg = new KOrganizer();
  if ( url.isValid() || url.isEmpty() ) {
    korg->init( true );
    if ( korg->openURL( url ) || url.isEmpty() ) {
      korg->show();
    } else {
      delete korg;
    }
  } else {
    korg->init( false );
    korg->show();
  }
}

void KOrganizer::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();

  mActionManager->readSettings();

  config->sync();
}


void KOrganizer::writeSettings()
{
  kdDebug(5850) << "KOrganizer::writeSettings" << endl;

  KConfig *config = KOGlobals::self()->config();

  mActionManager->writeSettings();
  saveMainWindowSettings( config );
  config->sync();
}


void KOrganizer::initActions()
{
  KStdAction::quit( this, SLOT( close() ), actionCollection() );
  mStatusBarAction = KStdAction::showStatusbar( this, SLOT( toggleStatusBar() ),
                                                actionCollection() );

  KStdAction::configureToolbars( this, SLOT( configureToolbars() ),
                                 actionCollection() );

  setInstance( KGlobal::instance() );

  setXMLFile( "korganizerui.rc" );
  createGUI( 0 );

  KConfig *config = KOGlobals::self()->config();

  applyMainWindowSettings( config );

  mStatusBarAction->setChecked( !statusBar()->isHidden() );
}

bool KOrganizer::queryClose()
{
  kdDebug(5850) << "KOrganizer::queryClose()" << endl;

  bool close = mActionManager->queryClose();

  // Write configuration. I don't know if it really makes sense doing it this
  // way, when having opened multiple calendars in different CalendarViews.
  if ( close ) writeSettings();

  return close;
}

bool KOrganizer::queryExit()
{
  // Don't call writeSettings here, because filename isn't valid anymore. It is
  // now called in queryClose.
//  writeSettings();
  return true;
}

void KOrganizer::configureToolbars()
{
  saveMainWindowSettings( KOGlobals::self()->config(), "MainWindow" );

  KEditToolbar dlg( factory() );
  dlg.exec();
}

void KOrganizer::toggleStatusBar()
{
  bool show_statusbar = mStatusBarAction->isChecked();
  if (show_statusbar)
     statusBar()->show();
  else
     statusBar()->hide();
}

void KOrganizer::statusBarPressed( int id )
{
  if ( id == ID_MESSAGES_IN )
    mCalendarView->dialogManager()->showIncomingDialog();
  else if ( id == ID_MESSAGES_OUT )
    mCalendarView->dialogManager()->showOutgoingDialog();
}

void KOrganizer::setNumIncoming( int num )
{
  statusBar()->changeItem( i18n(" Incoming messages: %1 ").arg( num ),
                           ID_MESSAGES_IN);
}

void KOrganizer::setNumOutgoing( int num )
{
  statusBar()->changeItem( i18n(" Outgoing messages: %1 ").arg( num ),
                           ID_MESSAGES_OUT );
}

void KOrganizer::showStatusMessage( const QString &message )
{
  statusBar()->message(message,2000);
}

bool KOrganizer::openURL( const KURL &url, bool merge )
{
  return mActionManager->openURL( url, merge );
}

bool KOrganizer::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizer::saveAsURL( const KURL & kurl )
{
  return mActionManager->saveAsURL( kurl )  ;
}

KURL KOrganizer::getCurrentURL() const
{
  return mActionManager->url();
}

void KOrganizer::saveProperties( KConfig *config )
{
  return mActionManager->saveProperties( config );
}

void KOrganizer::readProperties( KConfig *config )
{
  return mActionManager->readProperties( config );
}

KOrg::CalendarViewBase *KOrganizer::view() const
{
  return mActionManager->view();
}

void KOrganizer::setTitle()
{
//  kdDebug(5850) << "KOrganizer::setTitle" << endl;

  if ( !hasDocument() ) return;

  QString title;

  KURL url = mActionManager->url();

  if ( !url.isEmpty() ) {
    if ( url.isLocalFile() ) title = url.fileName();
    else title = url.prettyURL();
  } else {
    title = i18n("New Calendar");
  }

  if ( mCalendarView->isReadOnly() ) {
    title += " [" + i18n("read-only") + "]";
  }

  setCaption( title, !mCalendarView->isReadOnly() &&
                      mCalendarView->isModified() );
}
