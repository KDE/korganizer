#include <time.h>

#include <qdatetime.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>

#include "koprefs.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("timezone",I18N_NOOP("KOrganizer Timezone Test"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  kdDebug(5850) << "KOrganizer TimezoneId: " << KOPrefs::instance()->mTimeZoneId
            << endl;
  
  time_t ltime;
  ::time( &ltime );
  tm *t = localtime( &ltime );

  kdDebug(5850) << "localtime: " << t->tm_hour << ":" << t->tm_min << endl;

  kdDebug(5850) << "tzname: " << tzname[0] << " " << tzname[1] << endl;
  kdDebug(5850) << "timezone: " << timezone/3600 << endl;
  
  QTime qtime = QTime::currentTime();
  
  kdDebug(5850) << "QDateTime::currentTime(): "
            << qtime.toString( Qt::ISODate ) << endl;

  kdDebug(5850) << "KLocale::formatTime(): "
            << KGlobal::locale()->formatTime( qtime ) << endl;
}
