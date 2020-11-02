/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "../alarmdialog.h" //fullpath since incidenceeditors also has an alarmdialog.h

#include <AkonadiCore/Item>

#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>

#include <QDateTime>
#include <QApplication>
#include <QCommandLineParser>

using namespace KCalendarCore;

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
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    auto *e1 = new Event;
    e1->setSummary(QStringLiteral("This is a summary."));
    QDateTime now = QDateTime::currentDateTime();
    e1->setDtStart(now);
    e1->setDtEnd(now.addDays(1));
    Alarm::Ptr a = e1->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
    a->setAudioAlarm(QStringLiteral("/data/kde/share/apps/korganizer/sounds/spinout.wav"));

    Todo *t1 = new Todo;
    t1->setSummary(QStringLiteral("To-do A"));
    t1->setDtDue(now);
    t1->newAlarm();

    auto *e2 = new Event;
    e2->setSummary(QStringLiteral("This is another summary. "
                                  "But it is a very long summary of total sillyness for no good reason"));
    e2->setDtStart(now.addDays(1));
    e2->setDtEnd(now.addDays(2));
    e2->newAlarm();

    auto *e3 = new Event;
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

    auto *e4 = new Event;
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
