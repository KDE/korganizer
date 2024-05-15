/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "korganizer-version.h"

#include <KLocalizedString>

using namespace KOrg;

AboutData::AboutData()
    : KAboutData(QStringLiteral("korganizer"),
                 i18n("KOrganizer"),
                 QStringLiteral(KORGANIZER_VERSION),
                 i18n("A Personal Organizer"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 1997–1999 Preston Brown\n"
                      "Copyright © 2000–2004, 2007 Cornelius Schumacher\n"
                      "Copyright © 2004–2005 Reinhold Kainhofer\n"
                      "Copyright © 2006–2012 Allen Winter\n"
                      "Copyright © 2012–2024 KOrganizer authors"),
                 QString(),
                 QStringLiteral("https://userbase.kde.org/KOrganizer"))
{
    addAuthor(i18nc("@info:credit", "Allen Winter"), i18n("Maintainer"), QStringLiteral("winter@kde.org"));
    addAuthor(i18nc("@info:credit", "Reinhold Kainhofer"), i18n("Former Maintainer"), QStringLiteral("reinhold@kainhofer.com"));
    addAuthor(i18nc("@info:credit", "Cornelius Schumacher"), i18n("Former Maintainer"), QStringLiteral("schumacher@kde.org"));
    addAuthor(i18nc("@info:credit", "Preston Brown"), i18n("Original Author"), QStringLiteral("pbrown@kde.org"));
    addAuthor(i18nc("@info:credit", "Laurent Montel"), i18n("Developer"), QStringLiteral("montel@kde.org"));
    addCredit(i18nc("@info:credit", "Richard Apodaca"));
    addCredit(i18nc("@info:credit", "Björn Balazs"));
    addCredit(i18nc("@info:credit", "Jan-Pascal van Best"));
    addCredit(i18nc("@info:credit", "Bertjan Broeksema"));
    addCredit(i18nc("@info:credit", "Laszlo Boloni"));
    addCredit(i18nc("@info:credit", "Barry Benowitz"));
    addCredit(i18nc("@info:credit", "Christopher Beard"));
    addCredit(i18nc("@info:credit", "Kalle Dalheimer"));
    addCredit(i18nc("@info:credit", "Ian Dawes"));
    addCredit(i18nc("@info:credit", "Thomas Eitzenberger"));
    addCredit(i18nc("@info:credit", "Neil Hart"));
    addCredit(i18nc("@info:credit", "Declan Houlihan"));
    addCredit(i18nc("@info:credit", "Hans-Jürgen Husel"));
    addCredit(i18nc("@info:credit", "Tim Jansen"));
    addCredit(i18nc("@info:credit", "Christian Kirsch"));
    addCredit(i18nc("@info:credit", "Tobias König"));
    addCredit(i18nc("@info:credit", "Martin Koller"));
    addCredit(i18nc("@info:credit", "Uwe Koloska"));
    addCredit(i18nc("@info:credit", "Sergio Luis Martins"));
    addCredit(i18nc("@info:credit", "Mike McQuaid"));
    addCredit(i18nc("@info:credit", "Glen Parker"));
    addCredit(i18nc("@info:credit", "Dan Pilone"));
    addCredit(i18nc("@info:credit", "Roman Rohr"));
    addCredit(i18nc("@info:credit", "Rafał Rzepecki"), i18n("Part of work sponsored by Google with Summer of Code 2005"));
    addCredit(i18nc("@info:credit", "Don Sanders"));
    addCredit(i18nc("@info:credit", "Bram Schoenmakers"));
    addCredit(i18nc("@info:credit", "Günter Schwann"));
    addCredit(i18nc("@info:credit", "Herwin Jan Steehouwer"));
    addCredit(i18nc("@info:credit", "Mario Teijeiro"));
    addCredit(i18nc("@info:credit", "Nick Thompson"));
    addCredit(i18nc("@info:credit", "Bo Thorsen"));
    addCredit(i18nc("@info:credit", "Larry Wright"));
    addCredit(i18nc("@info:credit", "Thomas Zander"));
    addCredit(i18nc("@info:credit", "Fester Zigterman"));
}
