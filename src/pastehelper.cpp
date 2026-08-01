/*
  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pastehelper.h"

#include <KCalendarCore/MemoryCalendar>
#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(6, 29, 0)
#include <KCalendarCore/MimeData>
#else
#include <KCalUtils/DndFactory>
#endif

#include <QClipboard>
#include <QDate>
#include <QGuiApplication>
#include <QMimeData>
#include <QTimeZone>

using namespace KCalendarCore;

static QDateTime copyTimeSpec(const QDateTime &dt, const QDateTime &source)
{
    switch (source.timeSpec()) {
    case Qt::TimeZone:
    case Qt::LocalTime:
    case Qt::UTC:
        return dt.toTimeZone(source.timeZone());
    case Qt::OffsetFromUTC:
        return dt.toOffsetFromUtc(source.offsetFromUtc());
    }

    Q_UNREACHABLE();
}

static Incidence::Ptr pasteIncidence(const Incidence::Ptr &incidence, QDateTime newDateTime, PasteHelper::PasteFlags pasteOptions)
{
    Incidence::Ptr inc(incidence);

    if (inc) {
        inc = Incidence::Ptr(inc->clone());
        inc->recreate();
    }

    if (inc && newDateTime.isValid()) {
        if (inc->type() == Incidence::TypeEvent) {
            Event::Ptr const event = inc.staticCast<Event>();
            if (pasteOptions & PasteHelper::FlagPasteAtOriginalTime) {
                // Set date and preserve time and timezone stuff
                const QDate date = newDateTime.date();
                newDateTime = event->dtStart();
                newDateTime.setDate(date);
            }

            // in seconds
            const qint64 durationInSeconds = event->dtStart().secsTo(event->dtEnd());
            const qint64 durationInDays = event->dtStart().daysTo(event->dtEnd());

            if (incidence->allDay()) {
                event->setDtStart(QDateTime(newDateTime.date(), {}));
                event->setDtEnd(newDateTime.addDays(durationInDays));
            } else {
                event->setDtStart(copyTimeSpec(newDateTime, event->dtStart()));
                event->setDtEnd(copyTimeSpec(newDateTime.addSecs(durationInSeconds), event->dtEnd()));
            }
        } else if (inc->type() == Incidence::TypeTodo) {
            Todo::Ptr const aTodo = inc.staticCast<Todo>();
            const bool pasteAtDtStart = (pasteOptions & PasteHelper::FlagTodosPasteAtDtStart);
            if (pasteOptions & PasteHelper::FlagPasteAtOriginalTime) {
                // Set date and preserve time and timezone stuff
                const QDate date = newDateTime.date();
                newDateTime = pasteAtDtStart ? aTodo->dtStart() : aTodo->dtDue();
                newDateTime.setDate(date);
            }
            if (pasteAtDtStart) {
                aTodo->setDtStart(copyTimeSpec(newDateTime, aTodo->dtStart()));
            } else {
                aTodo->setDtDue(copyTimeSpec(newDateTime, aTodo->dtDue()));
            }
        } else if (inc->type() == Incidence::TypeJournal) {
            if (pasteOptions & PasteHelper::FlagPasteAtOriginalTime) {
                // Set date and preserve time and timezone stuff
                const QDate date = newDateTime.date();
                newDateTime = inc->dtStart();
                newDateTime.setDate(date);
            }
            inc->setDtStart(copyTimeSpec(newDateTime, inc->dtStart()));
        } else {
            qWarning() << "Trying to paste unknown incidence of type" << inc->type();
        }
    }

    return inc;
}

Incidence::List PasteHelper::pasteIncidences(const QDateTime &newDateTime, PasteFlags pasteOptions)
{
    QClipboard const *clipboard = QGuiApplication::clipboard();
    Q_ASSERT(clipboard);
#if KCALENDARCORE_VERSION < QT_VERSION_CHECK(6, 29, 0)
    Calendar::Ptr const calendar(KCalUtils::DndFactory::createDropCalendar(clipboard->mimeData()));
#else
    Calendar::Ptr const calendar(KCalendarCore::MimeData::decodeCalendar(clipboard->mimeData()));
#endif
    Incidence::List list;

    if (!calendar) {
        qWarning() << "Can't parse clipboard";
        return list;
    }

    // All pasted incidences get new uids, must keep track of old uids,
    // so we can update child's parents
    QHash<QString, Incidence::Ptr> oldUidToNewInc;

    Incidence::List::ConstIterator it;
    const Incidence::List incidences = calendar->incidences();
    Incidence::List::ConstIterator end(incidences.constEnd());
    for (it = incidences.constBegin(); it != end; ++it) {
        Incidence::Ptr const incidence = pasteIncidence(*it, newDateTime, pasteOptions);
        if (incidence) {
            list.append(incidence);
            oldUidToNewInc[(*it)->uid()] = *it;
        }
    }

    // update relations
    end = list.constEnd();
    for (it = list.constBegin(); it != end; ++it) {
        const Incidence::Ptr &incidence = *it;
        if (oldUidToNewInc.contains(incidence->relatedTo())) {
            Incidence::Ptr const parentInc = oldUidToNewInc[incidence->relatedTo()];
            incidence->setRelatedTo(parentInc->uid());
        } else {
            // not related to anything in the clipboard
            incidence->setRelatedTo(QString());
        }
    }

    return list;
}
