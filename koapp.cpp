#include <kstddirs.h>
#include <kglobal.h>
#include <kcmdlineargs.h>

#include "version.h"
#include "calobject.h"
#include "topwidget.h"
#include "koapp.h"


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

/*
void KOrganizerApp::help()
{
  printf("KOrganizer %s\n\n",korgVersion);
  printf("X11/Qt Options:\n");
  printf("-display <display>\n");
  printf("-geometry <geometry>\n");
  printf("-name <name>\n");
  printf("-title <title>\n");
  printf("-cmap\n\n");
  printf("KOrganizer Options:\n");
  printf("-c or --calendar <calendar>\n");
  printf("-l or --list\n");
  printf("-s or --show <numdays>\n");
  printf("-h or --help\n");
}
*/

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
    (new TopWidget(cal, QString::fromLocal8Bit( (const char*) fn), "TopWidget"))->show();
  }

  return 0;
}

/*
int KOrganizerApp::newInstance(QValueList<QCString> params)
{
  qDebug("KOApp::newInstance(params)");

  cal = new CalObject;

  KGlobal::config()->setGroup("General");

  fn = KGlobal::config()->readEntry("Current Calendar");

#if 0
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  // ...
#else
// if we're not using KCmdLineArgs
  // process command line arguments
  QValueList<QCString>::ConstIterator pIt(params.begin());
  ++pIt;
  for (; pIt != params.end(); ++pIt) {
    if ((*pIt == "-h") ||
	(*pIt == "--help")) {
      help();
      return 0;
    } else if ((*pIt ==  "-c") ||
	       (*pIt == "--calendar")) {
      ++pIt;
      if (!(*pIt).isEmpty())
	fn = *pIt;
    } else if ((*pIt == "-l") ||
	       (*pIt == "--list")) {
      displayImminent(1);
      return 0;
    } else if ((*pIt == "-s") ||
	       (*pIt == "--show")) {
      ++pIt;
      if (!(*pIt).isEmpty())
	displayImminent((*pIt).toInt());
      else
	displayImminent(1);
      return 0;
    } else {
      fn = *pIt;
    }
  }

#endif

  // always try to start the alarm daemon. IT will take care of itself
  // if there is no calendar to work on yet. we may or may not have a
  // calendar filename to work with.
  startAlarmDaemon();

  (new TopWidget(cal, fn, "TopWidget"))->show();
  return 0;
}
*/

#include "koapp.moc"
