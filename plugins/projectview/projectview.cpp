// $Id$

#include <qfile.h>

#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

//#include <korganizer/calendarviewbase.h">

#include "koprojectview.h"

#include "projectview.h"
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
	   
  new KAction(i18n("Project"), 0, this, SLOT(showView()),
              actionCollection(), "view_project");
}

ProjectView::~ProjectView()
{
}

QString ProjectView::info()
{
  return i18n("This plugin provides a gantt diagram as project view.");
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
