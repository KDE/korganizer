/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)

    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>

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
#include "calprinter.h"
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
#include "kogroupware.h"

#include <korganizer/part.h>

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
#include <kkeydialog.h>
#include <ktip.h>
#include <kstdguiitem.h>

#include <qcursor.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qfile.h>
#include <qlabel.h>

#include <stdlib.h>

using namespace KParts;
#include "korganizer.moc"
using namespace KOrg;

KOrganizer::KOrganizer( const char *name )
  : DCOPObject( "KOrganizerIface" ),
    KParts::MainWindow( 0, name ),
    KOrg::MainWindow(),
    mIsClosing( false )
{
  kdDebug(5850) << "KOrganizer::KOrganizer()" << endl;
  KOCore::self()->setXMLGUIClient( this );
//  setMinimumSize(600,400);  // make sure we don't get resized too small...

  mCalendarView = new CalendarView( this, "KOrganizer::CalendarView" );
  setCentralWidget(mCalendarView);

  mActionManager = new ActionManager( this, mCalendarView, this, this, false );
}

KOrganizer::~KOrganizer()
{
  delete mActionManager;
  delete mCalendar;
}

void KOrganizer::init( bool document )
{
  kdDebug() << "KOrganizer::init() "
            << ( document ? "hasDocument" : "resources" ) << endl;

  setHasDocument( document );

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  if ( hasDocument() ) {
    mCalendar = new CalendarLocal(KOPrefs::instance()->mTimeZoneId);
    mCalendarResources = 0;
    mCalendarView->setCalendar( mCalendar );
    mCalendarView->readSettings();
  } else {
    mCalendarResources = new CalendarResources( KOPrefs::instance()->mTimeZoneId );
    mCalendar = mCalendarResources;
    setCaption( i18n("Calendar") );

    CalendarResourceManager *manager = mCalendarResources->resourceManager();

    slotConfigChanged();

    if ( manager->isEmpty() ) {
      KConfig *config = KOGlobals::config();
      config->setGroup("General");
      QString fileName = config->readPathEntry( "Active Calendar" );

      QString resourceName;
      if ( fileName.isEmpty() ) {
        fileName = locateLocal( "appdata", "std.ics" );
        resourceName = i18n("Default KOrganizer resource");
      } else {
        resourceName = i18n("Active Calendar");
      }

      kdDebug(5850) << "Using as default resource: '" << fileName << "'" << endl;

      ResourceCalendar *defaultResource = new ResourceLocal( fileName );
      defaultResource->setResourceName( resourceName );

      manager->add( defaultResource );
      manager->setStandardResource( defaultResource );
    }

    kdDebug(5850) << "CalendarResources used by KOrganizer:" << endl;
    CalendarResourceManager::Iterator it;
    for( it = manager->begin(); it != manager->end(); ++it ) {
      (*it)->dump();
    }

    mCalendarView->setCalendar( mCalendarResources );
    mCalendarView->readSettings();

    // Construct the groupware object
    KOGroupware::create( mCalendarView, mCalendarResources );

    ResourceViewFactory factory( manager, mCalendarView );
    mCalendarView->addExtension( &factory );

    connect( mCalendarResources, SIGNAL( calendarChanged() ),
             mCalendarView, SLOT( slotCalendarChanged() ) );

    connect( mCalendarView, SIGNAL( configChanged() ),
             SLOT( slotConfigChanged() ) );
  }

  mCalendar->setOwner( KOPrefs::instance()->fullName() );
  mCalendar->setEmail( KOPrefs::instance()->email() );
  // setting fullName and email do not really count as modifying the calendar
  mCalendarView->setModified(false);

  mActionManager->init();
  connect( mActionManager, SIGNAL( actionNew( const KURL & ) ),
           SLOT( newMainWindow( const KURL & ) ) );
  connect( mActionManager, SIGNAL( actionKeyBindings() ),
           SLOT( configureKeyBindings() ) );

  initActions();
  readSettings();

//  initViews();

  statusBar()->insertItem( "", ID_GENERAL, 10 );

  statusBar()->insertItem( i18n(" Incoming messages: %1 ").arg( 0 ),
                           ID_MESSAGES_IN );
  statusBar()->insertItem( i18n(" Outgoing messages: %2 ").arg( 0 ),
                           ID_MESSAGES_OUT );
  statusBar()->setItemAlignment( ID_MESSAGES_IN, AlignRight );
  statusBar()->setItemAlignment( ID_MESSAGES_OUT, AlignRight );
  connect( statusBar(), SIGNAL( pressed( int ) ),
           SLOT( statusBarPressed( int ) ) );

  connect( mActionManager->view(), SIGNAL( numIncomingChanged( int ) ),
           SLOT( setNumIncoming( int ) ) );
  connect( mActionManager->view(), SIGNAL( numOutgoingChanged( int ) ),
           SLOT( setNumOutgoing( int ) ) );

  connect( mActionManager->view(), SIGNAL( statusMessage( const QString & ) ),
           SLOT( showStatusMessage( const QString & ) ) );

  mActionManager->loadParts();
  kdDebug(5850) << "KOrganizer::KOrganizer() done" << endl;

  setStandardToolBarMenuEnabled( true );
}

void KOrganizer::newMainWindow( const KURL &url )
{
  KOrganizer *korg = new KOrganizer();
  if ( url.isValid() && !url.isEmpty() ) {
    korg->init( true );
    if ( korg->openURL( url ) ) {
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

  KConfig *config = KOGlobals::config();

  config->setGroup( "KOrganizer Geometry" );

  int windowWidth = config->readNumEntry( "Width", 600 );
  int windowHeight = config->readNumEntry( "Height", 400 );

  resize( windowWidth, windowHeight );

  mActionManager->readSettings();

  config->sync();
}


void KOrganizer::writeSettings()
{
  kdDebug(5850) << "KOrganizer::writeSettings" << endl;

  KConfig *config = KOGlobals::config();

  config->setGroup( "KOrganizer Geometry" );
  config->writeEntry( "Width",width() );
  config->writeEntry( "Height",height() );

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

  KConfig *config = KOGlobals::config();

  applyMainWindowSettings( config );

  mStatusBarAction->setChecked( !statusBar()->isHidden() );
}

#if 0
void KOrganizer::initViews()
{
  kdDebug(5850) << "KOrganizer::initViews()" << endl;

  // TODO: get calendar pointer from somewhere
  KOrg::View::List views = KOCore::self()->views( this );
  KOrg::View *it;
  for( it = views.first(); it; it = views.next() ) {
    guiFactory()->addClient( it );
  }
}
#endif

bool KOrganizer::queryClose()
{
  kdDebug(5850) << "KOrganizer::queryClose()" << endl;

  bool close = true;

  if ( hasDocument() ) {
    close = mActionManager->saveModifiedURL();
  } else {
    if ( !mIsClosing ) {
      kdDebug(5850) << "!mIsClosing" << endl;
      mCalendar->save();
      // TODO: Put main window into a state indicating final saving.
      mIsClosing = true;
      connect( mCalendar, SIGNAL( calendarSaved() ), SLOT( close() ) );
    }
    if ( mCalendar->isSaving() ) {
      kdDebug(5850) << "KOrganizer::queryClose(): isSaving" << endl;
      close = false;
    } else {
      kdDebug(5850) << "KOrganizer::queryClose(): close = true" << endl;
      close = true;
    }
  }

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
  saveMainWindowSettings( KOGlobals::config(), "MainWindow" );

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

bool KOrganizer::deleteEvent( QString uid )
{
  return mActionManager->deleteEvent( uid );
}

bool KOrganizer::eventRequest( QString request, QCString receiver,
                               QString ical )
{
  return mActionManager->eventRequest( request, receiver, ical );
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

QString KOrganizer::getCurrentURLasString() const
{
  return mActionManager->getCurrentURLasString();
}

void KOrganizer::closeURL()
{
  return mActionManager->closeURL();
}

bool KOrganizer::openURL( QString url )
{
  return mActionManager->openURL( url );
}

bool KOrganizer::mergeURL( QString url )
{
  return mActionManager->mergeURL( url );
}

bool KOrganizer::saveAsURL( QString url )
{
  return mActionManager->saveAsURL( url );
}

void KOrganizer::slotConfigChanged()
{
  if ( mCalendarResources ) {
    if ( KOPrefs::instance()->mDestination == KOPrefs::askDestination )
      mCalendarResources->setAskDestinationPolicy();
    else
      mCalendarResources->setStandardDestinationPolicy();
  }
}

void KOrganizer::configureKeyBindings()
{
  KKeyDialog::configure( actionCollection(), this );
}
