/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998, 1999  Preston Brown <preston.brown@yale.edu>
  Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  Ian Dawes <iadawes@globalserve.net>
  Laszlo Boloni <boloni@cs.purdue.edu>

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

#include "korganizer.h"
#include "komailclient.h"
#include "calendarview.h"
#include "koviewmanager.h"
#include "kodialogmanager.h"
#include "kowindowlist.h"
#include "koprefs.h"
#include "kocore.h"
#include "actionmanager.h"
#include "koglobals.h"
#include "resourceview.h"
#include "korganizerifaceimpl.h"

#include <korganizer/part.h>

#include <libkdepim/statusbarprogresswidget.h>
#include <libkdepim/progressdialog.h>

#include <kcal/calendarlocal.h>
#include <kcal/calendarresources.h>
#include <kcal/resourcecalendar.h>

#include <kio/netaccess.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kedittoolbar.h>
#include <ktemporaryfile.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <ktip.h>
#include <KStandardGuiItem>
#include <kstatusbar.h>
#include <kshortcutsdialog.h>
#include <krecentfilesaction.h>

#include <QCursor>
#include <QTimer>
#include <QFile>
#include <QLabel>
#include <QLayout>

#include <stdlib.h>

using namespace KParts;
#include "korganizer.moc"
using namespace KOrg;

KOrganizer::KOrganizer() : KParts::MainWindow(), KOrg::MainWindow()
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

  kDebug();
  KOCore::self()->addXMLGUIClient( this, this );
//  setMinimumSize(600,400);  // make sure we don't get resized too small...

  mCalendarView = new CalendarView( this );
  mCalendarView->setObjectName( "KOrganizer::CalendarView" );
  setCentralWidget( mCalendarView );

  mActionManager = new ActionManager( this, mCalendarView, this, this, false, menuBar() );
  (void)new KOrganizerIfaceImpl( mActionManager, this, "IfaceImpl" );
}

KOrganizer::~KOrganizer()
{
  delete mActionManager;

  KOCore::self()->removeXMLGUIClient( this );
}

void KOrganizer::init( bool document )
{
  kDebug() << ( document ? "hasDocument" : "resources" );

  setHasDocument( document );

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  if ( hasDocument() ) {
    mActionManager->createCalendarLocal();
  } else {
    mActionManager->createCalendarResources();
  }

  setComponentData( KGlobal::mainComponent() );

  mActionManager->init();
  connect( mActionManager, SIGNAL(actionNew(const KUrl&)),
           SLOT(newMainWindow(const KUrl&)) );

  mActionManager->loadParts();

  initActions();
  readSettings();

  KStatusBar *bar = statusBar();

  bar->insertItem( "", ID_GENERAL, 10 );
  connect( bar, SIGNAL(pressed(int)), SLOT(statusBarPressed(int)) );

  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( bar, this );
  progressDialog->hide();

  KPIM::StatusbarProgressWidget *progressWidget;
  progressWidget = new KPIM::StatusbarProgressWidget( progressDialog, bar );
  progressWidget->show();

  bar->addPermanentWidget( progressWidget );

  connect( mActionManager->view(), SIGNAL(statusMessage(const QString&)),
           SLOT(showStatusMessage(const QString&)) );

  setStandardToolBarMenuEnabled( true );
  setTitle();

  kDebug() << "done";
}

void KOrganizer::newMainWindow( const KUrl &url )
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
  kDebug();

  KConfig *config = KOGlobals::self()->config();

  mActionManager->writeSettings();
  config->sync();
}

void KOrganizer::initActions()
{
  setStandardToolBarMenuEnabled( true );
  createStandardStatusBarAction();

  KStandardAction::keyBindings( this, SLOT(slotEditKeys()), actionCollection() );
  KStandardAction::configureToolbars( this, SLOT(configureToolbars()), actionCollection() );
  KStandardAction::quit( this, SLOT(close()), actionCollection() );

  setXMLFile( "korganizerui.rc", true );
  createGUI( 0 );

  setAutoSaveSettings();
}

void KOrganizer::slotEditKeys()
{
  KShortcutsDialog::configure( actionCollection(),
  KShortcutsEditor::LetterShortcutsAllowed );
}

bool KOrganizer::queryClose()
{
  kDebug();

  bool close = mActionManager->queryClose();

  // Write configuration. I don't know if it really makes sense doing it this
  // way, when having opened multiple calendars in different CalendarViews.
  if ( close ) {
    writeSettings();
  }

  return close;
}

bool KOrganizer::queryExit()
{
  // Don't call writeSettings here, because filename isn't valid anymore. It is
  // now called in queryClose.
//  writeSettings();
  return true;
}

void KOrganizer::statusBarPressed( int id )
{
  Q_UNUSED( id );
}

void KOrganizer::showStatusMessage( const QString &message )
{
  statusBar()->showMessage( message, 2000 );
}

bool KOrganizer::openURL( const KUrl &url, bool merge )
{
  return mActionManager->openURL( url, merge );
}

bool KOrganizer::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizer::saveAsURL( const KUrl & kurl )
{
  return mActionManager->saveAsURL( kurl )  ;
}

KUrl KOrganizer::getCurrentURL() const
{
  return mActionManager->url();
}

void KOrganizer::saveProperties( KConfigGroup &config )
{
  return mActionManager->saveProperties( config );
}

void KOrganizer::readProperties( const KConfigGroup &config )
{
  return mActionManager->readProperties( config );
}

KOrg::CalendarViewBase *KOrganizer::view() const
{
  return mActionManager->view();
}

void KOrganizer::setTitle()
{
  QString title;
  if ( !hasDocument() ) {
    title = i18n( "Calendar" );
  } else {
    KUrl url = mActionManager->url();

    if ( !url.isEmpty() ) {
      if ( url.isLocalFile() ) {
        title = url.fileName();
      } else {
        title = url.prettyUrl();
      }
    } else {
      title = i18n( "New Calendar" );
    }

    if ( mCalendarView->isReadOnly() ) {
      title += " [" + i18nc( "the calendar is read-only", "read-only" ) + ']';
    }
  }
  if ( mCalendarView->isFiltered() ) {
    title += " - <" + mCalendarView->currentFilterName() + "> ";
  }

  setCaption( title, !mCalendarView->isReadOnly() &&
                      mCalendarView->isModified() );
}
