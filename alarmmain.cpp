// $Id$
//
// KOrganizer alarm daemon main program
//

/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
*/

#include <stdlib.h>
#include <kdebug.h>

//#include <kglobal.h>
//#include <kstddirs.h>
//#include <ksimpleconfig.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "alarmapp.h"

AlarmApp *app;

static const KCmdLineOptions options[] =
{
        {"+[calendar]", I18N_NOOP("A calendar file to load"), 0},
	{0,0,0}
};


int main(int argc, char **argv)
{
  KLocale::setMainCatalogue("korganizer");
  KAboutData aboutData("alarmd",I18N_NOOP("AlarmDaemon"),
      "2.1beta",I18N_NOOP("KOrganizer Alarm Daemon"),KAboutData::License_GPL,
      "(c) 1997-1999 Preston Brown\n(c) 2000-2001 Cornelius Schumacher",0,
      "http://korganizer.kde.org");
  aboutData.addAuthor("Cornelius Schumacher",I18N_NOOP("Maintainer"),
                      "schumacher@kde.org");
  aboutData.addAuthor("Preston Brown",I18N_NOOP("Original Author"),
                      "pbrown@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions(options);
  KUniqueApplication::addCmdLineOptions();
  
  if (!AlarmApp::start())
    exit(0);

  AlarmApp app;
  kdDebug() << "alarmdaemon: app.exec()" << endl;
  return app.exec();
  kdDebug() << "alarmdaemon: app.exec() done" << endl;
}
