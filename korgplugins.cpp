#include <kaboutdata.h>
#include <kapplication.h>
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
    kdDebug(5850) << "Plugin: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Plugin *p = KOCore::self()->loadPlugin(*it);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  plugins = KOCore::self()->availablePlugins("KOrganizer/Part");
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug(5850) << "Part: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Part *p = KOCore::self()->loadPart(*it,0);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  plugins = KOCore::self()->availablePlugins("KOrganizer/View");
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug(5850) << "Part: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
#if 0
    KOrg::Part *p = KOCore::self()->loadPart(*it,0,0);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
#endif
  }

#if 0  
  KOrg::TextDecoration::List tdl = KOCore::self()->textDecorations();
  KOrg::TextDecoration *td = tdl.first();
  while(td) {
    kdDebug(5850) << "TEXT DECORATION INFO: " << td->info() << endl;
    td = tdl.next();
  }

  KOrg::WidgetDecoration *moon = KOCore::self()->loadWidgetDecoration("moon");
  if (moon) {
    QWidget *wid = moon->daySmall(0,QDate::currentDate());
    app.setMainWidget(wid);
    wid->show();
    app.exec();
  }
#endif
}
