#include <qwidget.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "alarmdialog.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  Event *e = new Event;
  e->setSummary( "This is a summary." );
  e->setDtStart( QDateTime::currentDateTime() );
  e->setDtEnd( QDateTime::currentDateTime().addDays( 1 ) );

  Alarm *a = e->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
  a->setAudioAlarm( "/opt/kde/share/apps/korganizer/sounds/spinout.wav" );

  AlarmDialog dlg;
  app.setMainWidget( &dlg );
  dlg.appendEvent( e );
  dlg.show();
  dlg.eventNotification();
    
  app.exec();
}
