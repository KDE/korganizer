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
    KOrg::Part *create(CalendarView *view,QObject *parent, const char *name)
    {
      return new WebExport(view,parent,name);
    }
};

extern "C" {
  void *init_libkorg_webexport()
  {
    return (new WebExportFactory);
  }
}


WebExport::WebExport(CalendarView *view,QObject *parent, const char *name) :
  KOrg::Part(view,parent,name)
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
  ExportWebDialog *dlg = new ExportWebDialog(view()->calendar());
  dlg->show();
}
