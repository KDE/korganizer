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

// $Id$

#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>

#include "calendarview.h"

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

  (void)new KAction(i18n("&List"), "list", 0,
                    widget, SLOT(showListView()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    widget, SLOT(showDayView()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    widget, SLOT(showWorkWeekView()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    widget, SLOT(showWeekView()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), "month", 0,
                    widget, SLOT(showMonthView()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-Do list"), "todo", 0,
                    widget, SLOT(view_todolist()),
                    actionCollection(), "view_todo");

  setXMLFile( "korganizer_part.rc" );
}

KOrganizerPart::~KOrganizerPart()
{
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
