/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include "../alarmdialog.h" //fullpath since incidenceeditors also has an alarmdialog.h

#include <AkonadiCore/Item>

#include <KCalCore/Event>
#include <KCalCore/Todo>

#include <KAboutData>

#include <QDateTime>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

using namespace KCalCore;

static Akonadi::Item::Id id = 1;
template<class T> Akonadi::Item incidenceToItem(T *incidence)
{
    Akonadi::Item item(++id);
    item.setPayload< QSharedPointer<T> >(QSharedPointer<T>(incidence));
    return item;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testkabc"), i18n("TestKabc"), QStringLiteral("0.1"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    Event *e1 = new Event;
    e1->setSummary(QStringLiteral("This is a summary."));
    KDateTime now = KDateTime::currentLocalDateTime();
    e1->setDtStart(now);
    e1->setDtEnd(now.addDays(1));
    Alarm::Ptr a = e1->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
    a->setAudioAlarm(QStringLiteral("/data/kde/share/apps/korganizer/sounds/spinout.wav"));

    Todo *t1 = new Todo;
    t1->setSummary(QStringLiteral("To-do A"));
    t1->setDtDue(now);
    t1->newAlarm();

    Event *e2 = new Event;
    e2->setSummary(QStringLiteral("This is another summary. "
                                  "But it is a very long summary of total sillyness for no good reason"));
    e2->setDtStart(now.addDays(1));
    e2->setDtEnd(now.addDays(2));
    e2->newAlarm();

    Event *e3 = new Event;
    e3->setSummary(QStringLiteral("Meet with Fred"));
    e3->setDtStart(now.addDays(2));
    e3->setDtEnd(now.addDays(3));
    e3->newAlarm();

    Todo *t2 = new Todo;
    t2->setSummary(QStringLiteral("Something big is due today"));
    t2->setDtDue(now);
    t2->newAlarm();

    Todo *t3 = new Todo;
    t3->setSummary(QStringLiteral("Be lazy"));
    t3->setDtDue(now);
    t3->newAlarm();

    Event *e4 = new Event;
    e4->setSummary(QStringLiteral("Watch TV"));
    e4->setDtStart(now.addSecs(120));
    e4->setDtEnd(now.addSecs(180));
    e4->newAlarm();

    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    AlarmDialog dlg(calendar);
    dlg.addIncidence(incidenceToItem(e2), QDateTime::currentDateTime().addSecs(60),
                     QString());
    dlg.addIncidence(incidenceToItem(t1), QDateTime::currentDateTime().addSecs(300),
                     QStringLiteral("THIS IS DISPLAY TEXT"));
    dlg.addIncidence(incidenceToItem(e4), QDateTime::currentDateTime().addSecs(120),
                     QStringLiteral("Fred and Barney get cloned"));
    dlg.addIncidence(incidenceToItem(e3), QDateTime::currentDateTime().addSecs(240),
                     QString());
    dlg.addIncidence(incidenceToItem(e1), QDateTime::currentDateTime().addSecs(180),
                     QString());
    dlg.addIncidence(incidenceToItem(t2), QDateTime::currentDateTime().addSecs(600),
                     QStringLiteral("THIS IS DISPLAY TEXT"));
    dlg.addIncidence(incidenceToItem(t3), QDateTime::currentDateTime().addSecs(360),
                     QString());
    dlg.show();
    dlg.eventNotification();

    return app.exec();
}
