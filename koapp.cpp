#include <kstddirs.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <dcopclient.h>

#include "version.h"
#include "calobject.h"
#include "korganizer.h"

#include "koapp.h"
#include "koapp.moc"


KOrganizerApp::KOrganizerApp() : KUniqueApplication()
{
}

KOrganizerApp::~KOrganizerApp()
{
}

void KOrganizerApp::displayImminent(const QString &file,int numdays)
{
  CalObject *cal = new CalObject;

  QDate currDate(QDate::currentDate());
  KOEvent *currEvent;

  if (!cal->load(file)) {
    printf(i18n("Could not load calendar '%1'.\n").arg(file));
    exit(0);
  }

  for (int i = 1; i <= numdays; i++) {
    QList<KOEvent> tmpList(cal->getEventsForDate(currDate, TRUE));
    printf("%s\n",currDate.toString().data());
    printf("---------------------------------\n");
    if (tmpList.first())
      for (currEvent = tmpList.first(); currEvent; currEvent = tmpList.next())
	currEvent->print(KOEvent::ASCII);
    else
      printf("(none)\n");
    printf("\n");
    currDate = currDate.addDays(1);
  }
}


void KOrganizerApp::startAlarmDaemon()
{
  // Start alarmdaemon. It is a KUniqueApplication, that means it is
  // automatically made sure that there is only one instance of the alarm daemon
  // running.
  QString execStr = locate("exe","alarmd");
  system(execStr.latin1());

  // Force alarm daemon to load active calendar
  if (!dcopClient()->send("alarmd","ad","reloadCal()","")) {
    qDebug("KOrganizerApp::startAlarmDaemon(): dcop send failed");
  }
}


int KOrganizerApp::newInstance()
{
  qDebug("KOApp::newInstance()");

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // process command line options
  int numDays = 0;
  if (args->isSet("list")) {
    numDays = 1;
  } else if (args->isSet("show")) {
    numDays = args->getOption("show").toInt();
  } else {
    startAlarmDaemon();
  }

  // If filenames was given as argument load this as calendars, one per window.
  if (args->count() > 0) {
    int i;
    for(i=0;i<args->count();++i) {
      processCalendar(args->arg(i),numDays);
    }
  } else {
    KGlobal::config()->setGroup("General");
    QString file = KGlobal::config()->readEntry("Active Calendar");

    processCalendar(file,numDays);
  }
  
  qDebug("KOApp::newInstance() done");
  return 0;
}

void KOrganizerApp::processCalendar(const QString & file,int numDays)
{
  if (numDays > 0) {
    displayImminent(file,numDays);
  } else {
    KOrganizer *korg = new KOrganizer("KOrganizer MainWindow");
    korg->show();
    if (!file.isEmpty()) korg->openURL(KURL(file));
  }
}
