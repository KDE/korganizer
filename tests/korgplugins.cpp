/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include <KAboutData>

#include <qdebug.h>

#include <calendarsupport/plugin.h>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "kocore.h"

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("korgplugins"), i18n("KOrgPlugins"), QStringLiteral("0.1"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KService::List plugins = KOCore::self()->availablePlugins();
    KService::List::ConstIterator it;
    for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
        qDebug() << "Plugin:" << (*it)->desktopEntryName() << "("
                 << (*it)->name() << ")";
        CalendarSupport::Plugin *p = KOCore::self()->loadPlugin(*it);
        if (!p) {
            qDebug() << "Plugin loading failed.";
        } else {
            qDebug() << "PLUGIN INFO:" << p->info();
        }
    }

    plugins = KOCore::self()->availableParts();
    for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
        qDebug() << "Part:" << (*it)->desktopEntryName() << "("
                 << (*it)->name() << ")";
        KOrg::Part *p = KOCore::self()->loadPart(*it, 0);
        if (!p) {
            qDebug() << "Part loading failed.";
        } else {
            qDebug() << "PART INFO:" << p->info();
        }
    }

    plugins = KOCore::self()->availableCalendarDecorations();
    for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
        qDebug() << "CalendarDecoration:" << (*it)->desktopEntryName() << "("
                 << (*it)->name() << ")";
        EventViews::CalendarDecoration::Decoration *p = KOCore::self()->loadCalendarDecoration(*it);
        if (!p) {
            qDebug() << "Calendar decoration loading failed.";
        } else {
            qDebug() << "CALENDAR DECORATION INFO:" << p->info();
        }
    }
    return 0;
}
