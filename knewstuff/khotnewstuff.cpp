/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "downloaddialog.h"

static const KCmdLineOptions op[] =
{
	{"type <type>", I18N_NOOP("Display only media of this type"), 0},
	KCmdLineLastOption
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

