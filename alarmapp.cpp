// $Id$

#include <qstring.h>

#include <ksimpleconfig.h>
#include <kdebug.h>

#include "alarmdaemon.h"

#include "alarmapp.h"
#include "alarmapp.moc"


AlarmApp::AlarmApp() :
  KUniqueApplication(),
  mAd(0)
{
}

AlarmApp::~AlarmApp()
{
}

int AlarmApp::newInstance()
{
  kdDebug() << "AlarmApp::newInstance()" << endl;

  // Check if we already have a running alarm daemon widget
  if (mAd) return 0;

  mAd = new AlarmDaemon(0,"ad");
  mAd->reloadCal();
  
  return 0;
}
