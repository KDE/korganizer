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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qapplication.h>
#include <qfile.h>
#include <kpopupmenu.h>
#include <qtimer.h>
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <kconfig.h>
#include <kprocess.h>
#include <ktempfile.h>

#include "calendarview.h"
#include "actionmanager.h"
#include "koapp.h" // for static methods
#include "korganizer.h"
#include "koglobals.h"
#include "kalarmd/alarmdaemoniface_stub.h"

#include "korganizer_part.h"

extern "C"
{
  /**
   * This function is the 'main' function of this part.  It takes
   * the form 'void *init_lib<library name>()  It always returns a
   * new factory object
   */
  void *init_libkorganizer()
  {
    return new KOrganizerFactory;
  }
};

/**
* We need one static instance of the factory for our C 'main'
* function
*/
KInstance *KOrganizerFactory::s_instance = 0L;
KAboutData *KOrganizerFactory::s_about = 0L;

KOrganizerFactory::KOrganizerFactory()
{
}

KOrganizerFactory::~KOrganizerFactory()
{
  delete s_instance;
  s_instance = 0;
  delete s_about;
}

KParts::Part *KOrganizerFactory::createPartObject(QWidget *parentWidget, const char *widgetName,
                                   QObject *parent, const char *name,
                                   const char*,const QStringList& )
{
  KParts::Part *obj = new KOrganizerPart(parentWidget, widgetName, parent, name );
  return obj;
}

KInstance *KOrganizerFactory::instance()
{
  if ( !s_instance ) {
    s_about = new KAboutData("korganizer", I18N_NOOP("KOrganizer"),"1.99");
    s_instance = new KInstance(s_about);
  }
  
  kdDebug() << "KOrganizerFactory::instance(): Name: " <<
               s_instance->instanceName() << endl;
  
  return s_instance;
}

KOrganizerPart::KOrganizerPart(QWidget *parentWidget, const char *widgetName,
                               QObject *parent, const char *name) :
  KParts::ReadOnlyPart(parent, name)
{
  setInstance(KOrganizerFactory::instance());

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parentWidget, widgetName);
  canvas->setFocusPolicy(QWidget::ClickFocus);
  setWidget(canvas);

  m_extension = new KOrganizerBrowserExtension(this);

  QVBoxLayout *topLayout = new QVBoxLayout(canvas);

  KGlobal::iconLoader()->addAppDir("korganizer");

  widget = new CalendarView(canvas);
  topLayout->addWidget(widget);

  widget->show();

  mam = new ActionManager( this, widget, this, this );
  mam->init();
  mam->readSettings();

  KConfig *config = KOGlobals::config();
  config->setGroup("General");
  QString urlString = config->readEntry("Active Calendar");

  // Force alarm daemon to load active calendar
  if (!urlString.isEmpty()) {
    KOrg::MainWindow *korg=ActionManager::findInstance(urlString);
    if ((0 == korg) && (!urlString.isEmpty()))
      mam->openURL(urlString);
  } else {
    QString location = locateLocal( "data", "korganizer/kontact.ics" );
    mam->saveAsURL( location );
    mam->makeActive();
  }

  setXMLFile( "korganizer_part.rc" );
  QTimer::singleShot(0, mam, SLOT(loadParts()));
}

void KOrganizerPart::startCompleted( KProcess* process ) {
    delete process;
}

void KOrganizerPart::saveCalendar()
{
  QPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
  KMainWindow *window = 0;
  while ((window = it.current()) != 0) {
    ++it;
    if (window->inherits("KOrganizer")) {
      KOrganizer *korg = dynamic_cast<KOrganizer*>(window);
      if (!korg->actionManager()->view()->isModified())
	continue;
      if (korg->actionManager()->getCurrentURL().isEmpty()) {
	KTempFile tmp( locateLocal( "data", "korganizer/" ));
	korg->actionManager()->saveAsURL( tmp.name() );
      } else {
	korg->actionManager()->saveURL();
      }
      window->close(true);
    }
  }

  if (mam->view()->isModified()) {
    if (!mam->getCurrentURL().isEmpty()) {
      mam->saveURL();
    } else {
      QString location = locateLocal( "data", "korganizer/kontact.ics" );
      mam->saveAsURL( location );
    }
  }
  mam->writeSettings();
  delete mam;
}

QWidget* KOrganizerPart::topLevelWidget()
{
  return widget->topLevelWidget();
}

ActionManager *KOrganizerPart::actionManager()
{
  return mam;
}

void KOrganizerPart::addPluginAction( KAction* action)
{
  action->plug( mam->pluginMenu()->popupMenu() );
}

void KOrganizerPart::setActive(bool active)
{
  mam->setActive(active);
}

KOrg::CalendarViewBase *KOrganizerPart::view() const
{
  return widget;
}

bool KOrganizerPart::openURL(const KURL &url, bool merge)
{
  return mam->openURL( url, merge );
}

bool KOrganizerPart::saveURL()
{
  return mam->saveURL();
}

bool KOrganizerPart::saveAsURL(const KURL & kurl)
{
  return mam->saveAsURL( kurl );
}

KURL KOrganizerPart::getCurrentURL() const
{
  return mam->getCurrentURL();
}

void KOrganizerPart::showStatusMessage(const QString& message)
{
  if (!parent() || !parent()->parent())
    return;
  KMainWindow *mainWin = dynamic_cast<KMainWindow*>(parent()->parent()); //yuck
  if (mainWin && mainWin->statusBar())
      mainWin->statusBar()->message( message );
}


KOrganizerPart::~KOrganizerPart()
{
  saveCalendar();
  closeURL();
}

bool KOrganizerPart::openFile()
{
  widget->openCalendar(m_file);
  widget->show();
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


