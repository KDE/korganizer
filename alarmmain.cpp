#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "alarmdaemon.h"
#include "alarmapp.moc"

int AlarmApp::newInstance(QValueList<QCString> params)
{
  AlarmDaemon *ad;

  // if no filename is supplied, read from config file.
  if (params.count() < 2) {
    KSimpleConfig config("korganizerrc", true);
    
    config.setGroup("General");
    QString newFileName = config.readEntry("Current Calendar");

    // this is the docking widget
    ad = new AlarmDaemon(newFileName.data(), 0, "ad");
  } else
    ad = new AlarmDaemon(params[1], 0, "ad");
}

int main(int argc, char **argv)
{
  if (!KUniqueApplication::start(argc, argv, "alarmd"))
    exit(0);

  KUniqueApplication app(argc, argv, "alarmd");
  app.setTopWidget(new QWidget);
  return app.exec();
}
