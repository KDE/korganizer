/* 
   KOrganizer (c) 1997-1999 Preston Brown
   All code is covered by the GNU Public License

   $Id$	
*/

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <qdir.h>

#include <kstddirs.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "koapp.h"

KOrganizerApp *app;

void signalHandler(int signo)
{
  switch(signo) {
  case SIGSEGV:
   fprintf(stderr, "KOrganizer Crash Handler (signal %d)\n\n", signo);
   fprintf(stderr, "KOrganizer has crashed.  Congratulations,\n"
	   "You have found a bug! :( Please send e-mail to\n"
	   "schumacher@kde.org with the following details:\n\n"
	   "1. What you were doing when the program crashed.\n"
	   "2. What version of KOrganizer you are running.\n"
	   "3. Any other details you feel are relevant.\n\n"
           "Remember: your bug reports help make the next\n"
           "release of KOrganizer more bug-free!\n");
   signal(SIGSEGV, SIG_DFL);
   kill(getpid(), 11); 
   break;
  default:
    break;
  }
  return;
}

static const char *description=I18N_NOOP("A Personal Organizer for KDE");
static const char *version="2.0pre";
static const KCmdLineOptions options[] =
{
        {"l", 0, 0},
        {"list", "List the events for the current day", 0},
	{"s", 0, 0},
	{"show <numdays>", "Show a list of all events for the next <numdays>",
         "1"},
        {"+[calendar]", "A calendar file to load", 0},
	{0,0,0}
};

int main (int argc, char **argv)
{
  KAboutData aboutData("korganizer", I18N_NOOP("korganizer"),
    version, description, KAboutData::License_GPL,
    "(c) 1997-1999, Preston Brown");
  aboutData.addAuthor("Preston Brown",0, "pbrown@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();
  
  if (signal(SIGSEGV, signalHandler) == SIG_ERR)
    debug("warning, can't catch SIGSEGV!");

  if (!KOrganizerApp::start())
    exit(0);

  KOrganizerApp app;
  qDebug("app.exec");
  return app.exec();
  qDebug("~app.exec");

}
