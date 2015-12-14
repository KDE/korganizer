/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"
#include "kdepim-version.h"

#include <KLocalizedString>

using namespace KOrg;

AboutData::AboutData()
    : KAboutData(QStringLiteral("korganizer"),
                 i18n("KOrganizer"),
                 QStringLiteral(KDEPIM_VERSION),
                 i18n("A Personal Organizer"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 1997–1999 Preston Brown\n"
                      "Copyright © 2000–2004, 2007 Cornelius Schumacher\n"
                      "Copyright © 2004–2005 Reinhold Kainhofer\n"
                      "Copyright © 2006–2012 Allen Winter\n"
                      "Copyright © 2012–2015 KOrganizer authors"),
                 QString(),
                 QStringLiteral("https://userbase.kde.org/KOrganizer"))
{
    addAuthor(i18n("Allen Winter"), i18n("Maintainer"),
              QStringLiteral("winter@kde.org"));
    addAuthor(i18n("Reinhold Kainhofer"), i18n("Former Maintainer"),
              QStringLiteral("reinhold@kainhofer.com"));
    addAuthor(i18n("Cornelius Schumacher"), i18n("Former Maintainer"),
              QStringLiteral("schumacher@kde.org"));
    addAuthor(i18n("Preston Brown"), i18n("Original Author"),
              QStringLiteral("pbrown@kde.org"));
    addAuthor(i18n("Laurent Montel"), i18n("Developer"),
              QStringLiteral("montel@kde.org"));
    addCredit(i18n("Richard Apodaca"));
    addCredit(i18n("Björn Balazs"));
    addCredit(i18n("Jan-Pascal van Best"));
    addCredit(i18n("Bertjan Broeksema"));
    addCredit(i18n("Laszlo Boloni"));
    addCredit(i18n("Barry Benowitz"));
    addCredit(i18n("Christopher Beard"));
    addCredit(i18n("Kalle Dalheimer"));
    addCredit(i18n("Ian Dawes"));
    addCredit(i18n("Thomas Eitzenberger"));
    addCredit(i18n("Neil Hart"));
    addCredit(i18n("Declan Houlihan"));
    addCredit(i18n("Hans-Jürgen Husel"));
    addCredit(i18n("Tim Jansen"));
    addCredit(i18n("Christian Kirsch"));
    addCredit(i18n("Tobias König"));
    addCredit(i18n("Martin Koller"));
    addCredit(i18n("Uwe Koloska"));
    addCredit(i18n("Sergio Luis Martins"));
    addCredit(i18n("Mike McQuaid"));
    addCredit(i18n("Glen Parker"));
    addCredit(i18n("Dan Pilone"));
    addCredit(i18n("Roman Rohr"));
    addCredit(i18n("Rafał Rzepecki"),
              i18n("Part of work sponsored by Google with Summer of Code 2005"));
    addCredit(i18n("Don Sanders"));
    addCredit(i18n("Bram Schoenmakers"));
    addCredit(i18n("Günter Schwann"));
    addCredit(i18n("Herwin Jan Steehouwer"));
    addCredit(i18n("Mario Teijeiro"));
    addCredit(i18n("Nick Thompson"));
    addCredit(i18n("Bo Thorsen"));
    addCredit(i18n("Larry Wright"));
    addCredit(i18n("Thomas Zander"));
    addCredit(i18n("Fester Zigterman"));
}
