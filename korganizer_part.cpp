/*
    This file is part of KOrganizer.

    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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

#include "korganizer_part.h"

#include "calendarview.h"
#include "actionmanager.h"
#include "koglobals.h"
#include "koprefs.h"
#include "resourceview.h"
#include "aboutdata.h"
#include "kocore.h"
#include "korganizerifaceimpl.h"
#include "stdcalendar.h"
#include "alarmclient.h"

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>

#include <kpopupmenu.h>
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kstatusbar.h>
#include <kparts/genericfactory.h>

#include <kparts/statusbarextension.h>

#include <sidebarextension.h>
#include <infoextension.h>

#include <qapplication.h>
#include <qfile.h>
#include <qtimer.h>
#include <qlayout.h>

typedef KParts::GenericFactory< KOrganizerPart > KOrganizerFactory;
K_EXPORT_COMPONENT_FACTORY( libkorganizerpart, KOrganizerFactory )

KOrganizerPart::KOrganizerPart( QWidget *parentWidget, const char *widgetName,
                                QObject *parent, const char *name,
                                const QStringList & ) :
  KPIM::Part(parent, name), mTopLevelWidget( parentWidget->topLevelWidget() )
{
  KOCore::self()->addXMLGUIClient( mTopLevelWidget, this );

  QString pname( name );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget, widgetName );
  canvas->setFocusPolicy( QWidget::ClickFocus );
  setWidget( canvas );
  mView = new CalendarView( canvas );

  mActionManager = new ActionManager( this, mView, this, this, true );
  (void)new KOrganizerIfaceImpl( mActionManager, this, "IfaceImpl" );

  if ( pname == "kontact" ) {
    mActionManager->createCalendarResources();
    setHasDocument( false );
    KOrg::StdCalendar::self()->load();
    mView->updateCategories();
  } else {
    mActionManager->createCalendarLocal();
    setHasDocument( true );
  }

  mBrowserExtension = new KOrganizerBrowserExtension( this );
  mStatusBarExtension = new KParts::StatusBarExtension( this );

  setInstance( KOrganizerFactory::instance() );

  QVBoxLayout *topLayout = new QVBoxLayout( canvas );
  topLayout->addWidget( mView );

  KGlobal::iconLoader()->addAppDir( "korganizer" );

  new KParts::SideBarExtension( mView->leftFrame(), this, "SBE" );

  KParts::InfoExtension *ie = new KParts::InfoExtension( this,
                                                         "KOrganizerInfo" );
  connect( mView, SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( slotChangeInfo( Incidence * ) ) );
  connect( this, SIGNAL( textChanged( const QString & ) ),
           ie, SIGNAL( textChanged( const QString & ) ) );

  mActionManager->init();
  mActionManager->readSettings();

  setXMLFile( "korganizer_part.rc" );
  mActionManager->loadParts();
  // If korganizer is run as part inside kontact, the alarmdaemon
  // is not started by KOrganizerApp, so we have to start it here.
  KOGlobals::self()->alarmClient()->startDaemon();
}

KOrganizerPart::~KOrganizerPart()
{
  mActionManager->saveCalendar();
  mActionManager->writeSettings();

  delete mActionManager;
  mActionManager = 0;

  closeURL();

  KOCore::self()->removeXMLGUIClient( mTopLevelWidget );
}

KAboutData *KOrganizerPart::createAboutData()
{
  return KOrg::AboutData::self();
}

void KOrganizerPart::startCompleted( KProcess *process )
{
  delete process;
}

void KOrganizerPart::slotChangeInfo( Incidence *incidence )
{
  if ( incidence ) {
    emit textChanged( incidence->summary() + " / " +
                      incidence->dtStartTimeStr() );
  } else {
    emit textChanged( QString::null );
  }
}

QWidget *KOrganizerPart::topLevelWidget()
{
  return mView->topLevelWidget();
}

ActionManager *KOrganizerPart::actionManager()
{
  return mActionManager;
}

void KOrganizerPart::showStatusMessage( const QString &message )
{
  KStatusBar *statusBar = mStatusBarExtension->statusBar();
  if ( statusBar ) statusBar->message( message );
}

KOrg::CalendarViewBase *KOrganizerPart::view() const
{
  return mView;
}

bool KOrganizerPart::openURL( const KURL &url, bool merge )
{
  return mActionManager->openURL( url, merge );
}

bool KOrganizerPart::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizerPart::saveAsURL( const KURL &kurl )
{
  return mActionManager->saveAsURL( kurl );
}

KURL KOrganizerPart::getCurrentURL() const
{
  return mActionManager->url();
}

bool KOrganizerPart::openFile()
{
  mView->openCalendar( m_file );
  mView->show();
  return true;
}

KOrganizerBrowserExtension::KOrganizerBrowserExtension(KOrganizerPart *parent) :
  KParts::BrowserExtension(parent, "KOrganizerBrowserExtension")
{
}

KOrganizerBrowserExtension::~KOrganizerBrowserExtension()
{
}

using namespace KParts;

#include "korganizer_part.moc"
