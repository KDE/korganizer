// $Id$

#include <qfile.h>

#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

#include "exportwebdialog.h"
#include "calendarview.h"

#include "webexport.h"
#include "webexport.moc"

class WebExportFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrganizer *parent, const char *name)
    {
      return new WebExport(parent,name);
    }
};

extern "C" {
  void *init_libkorg_webexport()
  {
    return (new WebExportFactory);
  }
}


WebExport::WebExport(KOrganizer *parent, const char *name) :
  KOrg::Part(parent,name)
{
//  KInstance * instance = new KInstance( "korganizer_part" );
//  setInstance( instance );

  setXMLFile("plugins/webexportui.rc");
	   
  new KAction(i18n("Export Web Page.."), 0, this, SLOT(exportWeb()),
              actionCollection(), "export_web");
}

WebExport::~WebExport()
{
}

QString WebExport::info()
{
  return i18n("This plugin provides export of calendars as web pages.");
}

void WebExport::exportWeb()
{
  ExportWebDialog *dlg = new ExportWebDialog(mainWindow()->view()->calendar());
  dlg->show();
}
