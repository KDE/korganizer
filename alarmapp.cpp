// $Id$

#include <qstring.h>

#include <ksimpleconfig.h>
#include <kcmdlineargs.h>

#include "alarmdaemon.h"

#include "alarmapp.h"
#include "alarmapp.moc"


AlarmApp::AlarmApp()
  : KUniqueApplication()
{
  mAd = 0;
}

AlarmApp::~AlarmApp()
{
}

int AlarmApp::newInstance()
{
  qDebug("AlarmApp::newInstance()");

  // Check if we already have a running alarm daemon widget
  if (mAd) return 0;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // If a filename was given as argument load this as calendar.
  // We should add the option to load multiple calendars with one command.
  if (args->count() > 0) {
    const char *fn = args->arg(0);
    mAd = new AlarmDaemon(fn,0,"ad");
  } else {
    KSimpleConfig config("korganizerrc", true);
    
    config.setGroup("General");
    QString newFileName = config.readEntry("Active Calendar");

    // this is the docking widget
    mAd = new AlarmDaemon(newFileName.latin1(), 0, "ad");
  }
  
  return 0;
}
