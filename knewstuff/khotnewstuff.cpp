#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "downloaddialog.h"

static const KCmdLineOptions op[] =
{
	{"type <type>", I18N_NOOP("Display only media of this type"), 0},
	{0, 0, 0}
};

int main(int argc, char **argv)
{
	KAboutData about("khotnewstuff", "KHotNewStuff", "0.1");
	KCmdLineArgs *args;

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(op);
	args = KCmdLineArgs::parsedArgs();

	KApplication i;

	KNS::DownloadDialog d;
	if(args->isSet("type")) d.setType(args->getOption("type"));
	d.load();
	d.exec();

	return 0;
}

