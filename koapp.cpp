#include <kstddirs.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>

#include "version.h"
#include "calobject.h"
#include "korganizer.h"

#include "koapp.h"
#include "koapp.moc"


KOrganizerApp::KOrganizerApp() : KUniqueApplication()
{
}

KOrganizerApp::KOrganizerApp(int &argc, char **argv, const QCString &rAppName)
  : KUniqueApplication(argc, argv, rAppName)
{
}

KOrganizerApp::~KOrganizerApp()
{
}

// display items that are imminent over the next several days as listed
void KOrganizerApp::displayImminent(int numdays)
{
  QDate currDate(QDate::currentDate());
  KOEvent *currEvent;

  if (fn.isEmpty()) {
    printf(i18n("No default calendar, and none was specified on the command line.\n"));
    exit(0);
  }

  if (!cal->load(fn)) {
    printf(i18n("Error reading from default calendar.\n"));
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
  QString execStr = locate("exe", "alarmd");

  if (!fn.isEmpty())
    execStr += " " + fn;

  system(execStr.latin1());
}

int KOrganizerApp::newInstance()
{
  qDebug("KOApp::newInstance()");

  cal = new CalObject;

  KGlobal::config()->setGroup("General");
  fn = KGlobal::config()->readEntry("Current Calendar");

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // If a filename was given as argument load this as calendar.
  // We should add the option to load multiple calendars with one command.
  if (args->count() > 0) {
    fn = args->arg(0);
  }

  // process command line options
  if (args->isSet("list")) {
    displayImminent(1);
  } else if (args->isSet("show")) {
    int numDays = args->getOption("show").toInt();
    displayImminent(numDays);
  } else {
    // always try to start the alarm daemon. IT will take care of itself
    // if there is no calendar to work on yet. we may or may not have a
    // calendar filename to work with.
    startAlarmDaemon();

    // Important: fn is a QCString, while the TopWidget constructor takes
    // a QString. There is no QString( const QCString& ) constructor.
    // It compiles, but we get a null-string with a length of MAXINT ;(
    (new KOrganizer(QString::fromLocal8Bit( (const char*) fn),false,
                    "KOrganizer MainWindow"))->show();
  }

  qDebug("KOApp::newInstance() done");
  return 0;
}
