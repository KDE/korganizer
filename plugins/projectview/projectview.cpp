/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/

// $Id$

#include <qfile.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

//#include <korganizer/calendarviewbase.h">

#include "koprojectview.h"

#include "projectview.h"
using namespace KOrg;
#include "projectview.moc"

class ProjectViewFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrg::MainWindow *parent, const char *name)
    {
      return new ProjectView(parent,name);
    }
};

extern "C" {
  void *init_libkorg_projectview()
  {
    return (new ProjectViewFactory);
  }
}


ProjectView::ProjectView(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name), mView(0)
{
//  KInstance * instance = new KInstance( "korganizer_part" );
//  setInstance( instance );

  setXMLFile("plugins/projectviewui.rc");
	   
  parent->addPluginAction( new KAction(i18n("Project"), 0, this, SLOT(showView()),
                           actionCollection(), "view_project") );
}

ProjectView::~ProjectView()
{
}

QString ProjectView::info()
{
  return i18n("This plugin provides a Gantt diagram as project view.");
}

void ProjectView::showView()
{
  if (!mView) {
    mView = new KOProjectView(mainWindow()->view()->calendar(),
                              mainWindow()->view());
    mainWindow()->view()->addView(mView);
  }
  mainWindow()->view()->showView(mView);
}
