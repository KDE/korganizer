#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "koprefs.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("timezone",I18N_NOOP("KOrganizer Timezone Test"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  kdDebug() << "TimezoneId: " << KOPrefs::instance()->mTimeZoneId << endl;
}
