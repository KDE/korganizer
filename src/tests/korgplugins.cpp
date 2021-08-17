/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kocore.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

#include <KService>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    KService::List::ConstIterator it;
    KService::List plugins = KOCore::self()->availableCalendarDecorations();
    for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
        qDebug() << "CalendarDecoration:" << (*it)->desktopEntryName() << "(" << (*it)->name() << ")";
        EventViews::CalendarDecoration::Decoration *p = KOCore::self()->loadCalendarDecoration(*it);
        if (!p) {
            qDebug() << "Calendar decoration loading failed.";
        } else {
            qDebug() << "CALENDAR DECORATION INFO:" << p->info();
        }
    }
    return 0;
}
