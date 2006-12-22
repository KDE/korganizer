/*
    This file is part of KOrganizer.

    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

#include <kcal/calendarlocal.h>
#include <kcal/calendarresources.h>
#include <kcal/resourcecalendar.h>

#include <kmenu.h>
#include <kinstance.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kprocess.h>
#include <ktemporaryfile.h>
#include <kstatusbar.h>
#include <kparts/genericfactory.h>

#include <kparts/statusbarextension.h>

#include <QApplication>
#include <QFile>
#include <QTimer>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>

typedef KParts::GenericFactory< KOrganizerPart > KOrganizerFactory;
K_EXPORT_COMPONENT_FACTORY( libkorganizerpart, KOrganizerFactory )

KOrganizerPart::KOrganizerPart( QWidget *parentWidget, QObject *parent,
                                const QStringList & ) :
  KParts::ReadOnlyPart(parent), mTopLevelWidget( parentWidget->topLevelWidget() )
{
  KGlobal::locale()->insertCatalog( "libkcal" );
  KGlobal::locale()->insertCatalog( "libkdepim" );
  KGlobal::locale()->insertCatalog( "kdgantt" );

  KOCore::self()->addXMLGUIClient( mTopLevelWidget, this );

#warning Port me!
//  QString pname( name );

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget );
  canvas->setFocusPolicy( Qt::ClickFocus );
  setWidget( canvas );
  mView = new CalendarView( canvas );

  mActionManager = new ActionManager( this, mView, this, this, true );
  (void)new KOrganizerIfaceImpl( mActionManager, this, "IfaceImpl" );

#warning Port me!
//  if ( pname == QLatin1String("kontact") ) {
    mActionManager->createCalendarResources();
    setHasDocument( false );
    KOrg::StdCalendar::self()->load();
    mView->updateCategories();
//  } else {
//    mActionManager->createCalendarLocal();
//    setHasDocument( true );
//  }

  mStatusBarExtension = new KParts::StatusBarExtension( this );

  setInstance( KOrganizerFactory::instance() );

  QVBoxLayout *topLayout = new QVBoxLayout( canvas );
  topLayout->addWidget( mView );

  KGlobal::iconLoader()->addAppDir( "korganizer" );

  connect( mView, SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( slotChangeInfo( Incidence * ) ) );

  mActionManager->init();
  mActionManager->readSettings();

  setXMLFile( "korganizer_part.rc" );
  mActionManager->loadParts();
  setTitle();
}

KOrganizerPart::~KOrganizerPart()
{
  mActionManager->saveCalendar();
  mActionManager->writeSettings();

  delete mActionManager;
  mActionManager = 0;

  closeUrl();

  KOCore::self()->removeXMLGUIClient( mTopLevelWidget );
}

KAboutData *KOrganizerPart::createAboutData()
{
  return new KOrg::AboutData;
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
    emit textChanged( QString() );
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
  if ( statusBar ) statusBar->showMessage( message );
}

KOrg::CalendarViewBase *KOrganizerPart::view() const
{
  return mView;
}

bool KOrganizerPart::openURL( const KUrl &url, bool merge )
{
  return mActionManager->openURL( url, merge );
}

bool KOrganizerPart::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizerPart::saveAsURL( const KUrl &kurl )
{
  return mActionManager->saveAsURL( kurl );
}

KUrl KOrganizerPart::getCurrentURL() const
{
  return mActionManager->url();
}

bool KOrganizerPart::openFile()
{
  mView->openCalendar( m_file );
  mView->show();
  return true;
}

// FIXME: This is copied verbatim from the KOrganizer class. Move it to the common base class!
void KOrganizerPart::setTitle()
{
//  kDebug(5850) << "KOrganizer::setTitle" << endl;
// FIXME: Inside kontact we want to have different titles depending on the
//        type of view (calendar, to-do, journal). How can I add the filter
//        name in that case?
/*
  QString title;
  if ( !hasDocument() ) {
    title = i18n("Calendar");
  } else {
    KUrl url = mActionManager->url();

    if ( !url.isEmpty() ) {
      if ( url.isLocalFile() ) title = url.fileName();
      else title = url.prettyUrl();
    } else {
      title = i18n("New Calendar");
    }

    if ( mView->isReadOnly() ) {
      title += " [" + i18n("read-only") + ']';
    }
  }

  title += " - <" + mView->currentFilterName() + "> ";

  emit setWindowCaption( title );*/
}

#include "korganizer_part.moc"
