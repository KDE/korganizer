#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "testnewstuff.h"
#include "testnewstuff.moc"

void MyWidget::mySlot()
{
  kdDebug() << "CLICK" << endl;

  TestNewStuff *newStuff = new TestNewStuff;
  newStuff->download();
}

static const KCmdLineOptions options[] =
{
  { "upload", "Upload Hot New Stuff.", 0 },
  { "download", "Download Hot New Stuff.", 0 }
};

int main(int argc,char **argv)
{
  KAboutData aboutData("knewstufftest","KNewStuff Test","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  MyWidget wid;
  app.setMainWidget( &wid );
  wid.show();

#if 0
  TestNewStuff newStuff;

  if ( args->isSet( "upload" ) ) {
    newStuff.upload();
  } else if ( args->isSet( "download" ) ) {
    newStuff.download();
  } else {
    kdDebug() << "Please pass --upload or --download option." << endl;
  }
#endif

  app.exec();
}
