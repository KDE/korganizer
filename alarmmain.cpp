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
      "1.93",I18N_NOOP("KOrganizer Alarm Daemon"),KAboutData::License_GPL,
      "(c) 1997-1999 Preston Brown\n(c) 2000 Cornelius Schumacher",0,
      "http://devel-home.kde.org/~korganiz");
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
  qDebug("alarmdaemon: app.exec()");
  return app.exec();
  qDebug("alarmdaemon: app.exec() done");
}
