// $Id$
//
// KOrganizer alarm daemon main program
//

#include <stdlib.h>
#include <kdebug.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "alarmapp.h"

static const KCmdLineOptions options[] =
{
        {"+[calendar]", I18N_NOOP("A calendar file to load"), 0},
	{0,0,0}
};

int main(int argc, char **argv)
{
  KLocale::setMainCatalogue("korganizer");
  KAboutData aboutData("alarmd",I18N_NOOP("AlarmDaemon"),
      "2.2pre",I18N_NOOP("KOrganizer Alarm Daemon"),KAboutData::License_GPL,
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

  return app.exec();
}
