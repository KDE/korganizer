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

#include "kalarmd/alarmdaemoniface_stub.h"

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
#include <kkeydialog.h>
#include <kparts/genericfactory.h>

#include <sidebarextension.h>
#include <infoextension.h>

#include <qapplication.h>
#include <qfile.h>
#include <qtimer.h>

typedef KParts::GenericFactory< KOrganizerPart > KOrganizerFactory;
K_EXPORT_COMPONENT_FACTORY( libkorganizerpart, KOrganizerFactory )

KOrganizerPart::KOrganizerPart( QWidget *parentWidget, const char *widgetName,
                                QObject *parent, const char *name,
                                const QStringList & ) :
  KParts::ReadOnlyPart(parent, name)
{
  KOCore::self()->setXMLGUIClient( this );

  QString pname( name );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parentWidget, widgetName);
  canvas->setFocusPolicy(QWidget::ClickFocus);
  setWidget(canvas);
  CalendarView *view;

  if ( pname!="kontact" ) {
    mCalendar = new CalendarLocal(KOPrefs::instance()->mTimeZoneId);
    mCalendarResources = 0;
    view = new CalendarView( canvas );
    view->setCalendar( mCalendar );
    view->readSettings();
  } else {
    mCalendarResources = new CalendarResources( KOPrefs::instance()->mTimeZoneId );
    mCalendar = mCalendarResources;
    CalendarResourceManager *manager = mCalendarResources->resourceManager();
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
    view = new CalendarView( canvas );
    view->setCalendar( mCalendarResources );
    view->readSettings();
    ResourceViewFactory factory( manager, view );
    view->addExtension( &factory );
    connect( mCalendarResources, SIGNAL( calendarChanged() ),
             view, SLOT( updateView() ) );
    connect( mCalendarResources, SIGNAL( calendarChanged() ),
             view, SLOT( slotCalendarChanged() ) );

    connect( view, SIGNAL( configChanged() ),
             SLOT( slotConfigChanged() ) );
    if ( KOPrefs::instance()->mDestination==KOPrefs::askDestination )
      mCalendarResources->setAskDestinationPolicy();
    else
      mCalendarResources->setStandardDestinationPolicy();
  }

  mBrowserExtension = new KOrganizerBrowserExtension(this);
  mStatusBarExtension = new KParts::StatusBarExtension(this);

  setInstance(KOrganizerFactory::instance());


  QVBoxLayout *topLayout = new QVBoxLayout(canvas);

  KGlobal::iconLoader()->addAppDir("korganizer");

  mWidget = view;
  topLayout->addWidget( mWidget );

  new KParts::SideBarExtension(view->leftFrame(), this, "SBE");

  KParts::InfoExtension *ie = new KParts::InfoExtension( this,
                                                          "KOrganizerInfo" );
  connect( mWidget, SIGNAL( incidenceSelected( Incidence * ) ),
            SLOT( slotChangeInfo( Incidence * ) ) );
  connect( this, SIGNAL( textChanged( const QString & ) ),
            ie, SIGNAL( textChanged( const QString& ) ) );

  mWidget->show();

  mActionManager = new ActionManager( this, mWidget, this, this, true );
  mActionManager->init();
  mActionManager->readSettings();
  connect( mActionManager, SIGNAL( actionKeyBindings() ),
            SLOT( configureKeyBindings() ) );

#if 0
  KConfig *config = KOGlobals::config();
  config->setGroup("General");
  QString urlString = config->readPathEntry("Active Calendar");

  // Force alarm daemon to load active calendar
  if (!urlString.isEmpty()) {
    KOrg::MainWindow *korg=ActionManager::findInstance(urlString);
    if ((0 == korg) && (!urlString.isEmpty()))
      mActionManager->openURL(urlString);
  } else {
    QString location = locateLocal( "data", "korganizer/kontact.ics" );
    mActionManager->saveAsURL( location );
    mActionManager->makeActive();
  }
#endif

  setXMLFile( "korganizer_part.rc" );
  QTimer::singleShot(0, mActionManager, SLOT(loadParts()));
}

KAboutData *KOrganizerPart::createAboutData()
{
  return KOrg::AboutData::self();
}

void KOrganizerPart::startCompleted( KProcess* process )
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

void KOrganizerPart::saveCalendar()
{
  if ( mActionManager->view()->isModified() ) {
    if ( !mActionManager->url().isEmpty() ) {
      mActionManager->saveURL();
    } else {
      QString location = locateLocal( "data", "korganizer/kontact.ics" );
      mActionManager->saveAsURL( location );
    }
  }
  mActionManager->writeSettings();
  delete mActionManager;
  mActionManager = 0;
}

QWidget *KOrganizerPart::topLevelWidget()
{
  return mWidget->topLevelWidget();
}

ActionManager *KOrganizerPart::actionManager()
{
  return mActionManager;
}

void KOrganizerPart::addPluginAction( KAction *action )
{
  action->plug( mActionManager->pluginMenu()->popupMenu() );
}

void KOrganizerPart::showStatusMessage( const QString &message )
{
  KStatusBar *statusBar = mStatusBarExtension->statusBar();
  if ( statusBar ) statusBar->message( message );
}

KOrg::CalendarViewBase *KOrganizerPart::view() const
{
  return mWidget;
}

bool KOrganizerPart::openURL(const KURL &url, bool merge)
{
  return mActionManager->openURL( url, merge );
}

bool KOrganizerPart::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizerPart::saveAsURL(const KURL & kurl)
{
  return mActionManager->saveAsURL( kurl );
}

KURL KOrganizerPart::getCurrentURL() const
{
  return mActionManager->url();
}

KOrganizerPart::~KOrganizerPart()
{
  saveCalendar();
  closeURL();
}

bool KOrganizerPart::openFile()
{
  mWidget->openCalendar(m_file);
  mWidget->show();
  return true;
}

void KOrganizerPart::configureKeyBindings()
{
  KKeyDialog::configure( actionCollection(), true );
}

void KOrganizerPart::slotConfigChanged()
{
  if ( mCalendarResources ) {
    if ( KOPrefs::instance()->mDestination==KOPrefs::askDestination )
      mCalendarResources->setAskDestinationPolicy();
    else
      mCalendarResources->setStandardDestinationPolicy();
  }
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


