#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <calendar/plugin.h>

#include "kocore.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("korgplugins",I18N_NOOP("KOrgPlugins"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  
  KTrader::OfferList plugins = KOCore::self()->availablePlugins("Calendar/Plugin");
  KTrader::OfferList::ConstIterator it;
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug() << "Plugin: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Plugin *p = KOCore::self()->loadPlugin(*it);
    if (!p) {
      kdDebug() << "Plugin loading failed." << endl;
    } else {
      kdDebug() << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  plugins = KOCore::self()->availablePlugins("KOrganizer/Part");
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug() << "Part: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Part *p = KOCore::self()->loadPart(*it,0,0);
    if (!p) {
      kdDebug() << "Plugin loading failed." << endl;
    } else {
      kdDebug() << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  KOrg::TextDecoration::List tdl = KOCore::self()->textDecorations();
  KOrg::TextDecoration *td = tdl.first();
  while(td) {
    kdDebug() << "TEXT DECORATION INFO: " << td->info() << endl;
    td = tdl.next();
  }

  KOrg::WidgetDecoration *moon = KOCore::self()->loadWidgetDecoration("moon");
  if (moon) {
    QWidget *wid = moon->daySmall(0,QDate::currentDate());
    app.setMainWidget(wid);
    wid->show();
    app.exec();
  }
}
