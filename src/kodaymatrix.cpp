/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>
  Parts of the source code have been copied from kdpdatebutton.cpp

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kodaymatrix.h"
#include "koglobals.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/Utils>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <QIcon>
#include <QMenu>
#include <QUrl>

#include <KLocalizedString>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

// ============================================================================
//  K O D A Y M A T R I X
// ============================================================================

const int KODayMatrix::NOSELECTION = -1000;
const int KODayMatrix::NUMDAYS = 42;

KODayMatrix::KODayMatrix(QWidget *parent)
    : QFrame(parent)
{
    // initialize dynamic arrays
    mDays = new QDate[NUMDAYS];
    mDayLabels = new QString[NUMDAYS];

    mTodayMarginWidth = 2;
    mSelEnd = mSelStart = NOSELECTION;

    recalculateToday();

    mHighlightEvents = true;
    mHighlightTodos = false;
    mHighlightJournals = false;
}

void KODayMatrix::addCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    calendar->registerObserver(this);
    mCalendars.push_back(calendar);

    setAcceptDrops(true);
    updateIncidences();
}

void KODayMatrix::removeCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    calendar->unregisterObserver(this);
    mCalendars.removeOne(calendar);

    setAcceptDrops(!mCalendars.empty());
    updateIncidences();
}

QColor KODayMatrix::getShadedColor(const QColor &color) const
{
    QColor shaded;
    int h = 0;
    int s = 0;
    int v = 0;
    color.getHsv(&h, &s, &v);
    s = s / 4;
    v = 192 + v / 4;
    shaded.setHsv(h, s, v);

    return shaded;
}

KODayMatrix::~KODayMatrix()
{
    for (const auto &calendar : std::as_const(mCalendars)) {
        calendar->unregisterObserver(this);
    }

    delete[] mDays;
    delete[] mDayLabels;
}

void KODayMatrix::setSelectedDaysFrom(QDate start, QDate end)
{
    if (mStartDate.isValid()) {
        mSelStart = mStartDate.daysTo(start);
        mSelEnd = mStartDate.daysTo(end);
    }
}

void KODayMatrix::clearSelection()
{
    mSelEnd = mSelStart = NOSELECTION;
}

void KODayMatrix::recalculateToday()
{
    if (!mStartDate.isValid()) {
        return;
    }

    mToday = -1;
    for (int i = 0; i < NUMDAYS; ++i) {
        mDays[i] = mStartDate.addDays(i);
        mDayLabels[i] = QString::number(mDays[i].day());

        // if today is in the currently displayed month, highlight today
        if (mDays[i].year() == QDate::currentDate().year() && mDays[i].month() == QDate::currentDate().month()
            && mDays[i].day() == QDate::currentDate().day()) {
            mToday = i;
        }
    }
}

void KODayMatrix::updateView()
{
    updateView(mStartDate);
}

void KODayMatrix::setUpdateNeeded()
{
    mPendingChanges = true;
}

void KODayMatrix::updateView(QDate actdate)
{
    if (!actdate.isValid() || NUMDAYS < 1) {
        return;
    }
    // flag to indicate if the starting day of the matrix has changed by this call
    bool daychanged = false;

    // if a new startdate is to be set then apply Cornelius's calculation
    // of the first day to be shown
    if (actdate != mStartDate) {
        // reset index of selection according to shift of starting date from
        // startdate to actdate.
        if (mSelStart != NOSELECTION) {
            int const tmp = actdate.daysTo(mStartDate);
            // shift selection if new one would be visible at least partly !
            if (mSelStart + tmp < NUMDAYS && mSelEnd + tmp >= 0) {
                // nested if required for next X display pushed from a different month
                // correction required. otherwise, for month forward and backward,
                // it must be avoided.
                if (mSelStart > NUMDAYS || mSelStart < 0) {
                    mSelStart = mSelStart + tmp;
                }
                if (mSelEnd > NUMDAYS || mSelEnd < 0) {
                    mSelEnd = mSelEnd + tmp;
                }
            }
        }

        mStartDate = actdate;
        daychanged = true;
    }

    if (daychanged) {
        recalculateToday();
    }

    // The calendar has not changed in the meantime and the selected range
    // is still the same so we can save the expensive updateIncidences() call
    if (!daychanged && !mPendingChanges) {
        return;
    }

    // TODO_Recurrence: If we just change the selection, but not the data,
    // there's no need to update the whole list of incidences… This is just a
    // waste of computational power
    updateIncidences();
    QMap<QDate, QStringList> holidaysByDate = KOGlobals::self()->holiday(mDays[0], mDays[NUMDAYS - 1]);
    for (int i = 0; i < NUMDAYS; ++i) {
        // if it is a holy day then draw it red. Sundays are consider holidays, too
        QStringList const holidays = holidaysByDate[mDays[i]];
        QString holiStr;

        if (!holidays.isEmpty()) {
            holiStr = holidays.join(i18nc("delimiter for joining holiday names", ","));
            if (holiStr.isEmpty()) {
                holiStr = QLatin1StringView("");
            }
        }
        mHolidays[i] = holiStr;
    }
}

void KODayMatrix::updateIncidences()
{
    mEvents.clear();

    if (mHighlightEvents) {
        updateEvents();
    }

    if (mHighlightTodos) {
        updateTodos();
    }

    if (mHighlightJournals) {
        updateJournals();
    }

    mPendingChanges = false;
}

void KODayMatrix::updateJournals()
{
    for (const auto &calendar : std::as_const(mCalendars)) {
        const KCalendarCore::Incidence::List incidences = calendar->incidences();
        for (const KCalendarCore::Incidence::Ptr &inc : incidences) {
            Q_ASSERT(inc);
            QDate const d = inc->dtStart().toLocalTime().date();
            if (inc->type() == KCalendarCore::Incidence::TypeJournal && d >= mDays[0] && d <= mDays[NUMDAYS - 1] && !mEvents.contains(d)) {
                mEvents.append(d);
            }
            if (mEvents.count() == NUMDAYS) {
                // No point in wasting cpu, all days are bold already
                break;
            }
        }
    }
}

/**
 * Although updateTodos() is simpler it has some similarities with updateEvent()
 * but don't bother refactoring them so they share code, there's a bigger fish:
 * Try to refactor updateTodos(), updateEvent(), updateJournals(), monthview,
 * agenda view, timeline view, event list view and todo list view
 * all these 9 places have incidence listing code in common, maybe it could go
 * to kcal. Ah, and then there's kontact's summary view which still uses
 * the old CPU consuming code.
 */
void KODayMatrix::updateTodos()
{
    QDate d;
    for (const auto &calendar : std::as_const(mCalendars)) {
        const KCalendarCore::Todo::List todos = calendar->todos();
        for (const KCalendarCore::Todo::Ptr &t : todos) {
            if (mEvents.count() == NUMDAYS) {
                // No point in wasting cpu, all days are bold already
                break;
            }
            Q_ASSERT(t);
            if (t->hasDueDate()) {
                ushort const recurType = t->recurrenceType();

                if (t->recurs() && !(recurType == KCalendarCore::Recurrence::rDaily && !KOPrefs::instance()->mDailyRecur)
                    && !(recurType == KCalendarCore::Recurrence::rWeekly && !KOPrefs::instance()->mWeeklyRecur)) {
                    // It's a recurring todo, find out in which days it occurs
                    const auto timeDateList = t->recurrence()->timesInInterval(QDateTime(mDays[0], {}, QTimeZone::LocalTime),
                                                                               QDateTime(mDays[NUMDAYS - 1], {}, QTimeZone::LocalTime));

                    for (const QDateTime &dt : timeDateList) {
                        d = dt.toLocalTime().date();
                        if (!mEvents.contains(d)) {
                            mEvents.append(d);
                        }
                    }
                } else {
                    d = t->dtDue().toLocalTime().date();
                    if (d >= mDays[0] && d <= mDays[NUMDAYS - 1] && !mEvents.contains(d)) {
                        mEvents.append(d);
                    }
                }
            }
        }
    }
}

void KODayMatrix::updateEvents()
{
    if (mEvents.count() == NUMDAYS) {
        mPendingChanges = false;
        // No point in wasting cpu, all days are bold already
        return;
    }

    for (const auto &calendar : std::as_const(mCalendars)) {
        const auto eventlist = calendar->events(mDays[0], mDays[NUMDAYS - 1], calendar->timeZone());
        for (const KCalendarCore::Event::Ptr &ev : eventlist) {
            if (mEvents.count() == NUMDAYS) {
                // No point in wasting cpu, all days are bold already
                break;
            }

            Q_ASSERT(ev);
            const ushort recurType = ev->recurrenceType();
            const QDateTime dtStart = ev->dtStart().toLocalTime();

            // timed incidences occur in
            //   [dtStart(), dtEnd()[. All-day incidences occur in [dtStart(), dtEnd()]
            // so we subtract 1 second in the timed case
            const int secsToAdd = ev->allDay() ? 0 : -1;
            const QDateTime dtEnd = ev->dtEnd().toLocalTime().addSecs(secsToAdd);

            if (!(recurType == KCalendarCore::Recurrence::rDaily && !KOPrefs::instance()->mDailyRecur)
                && !(recurType == KCalendarCore::Recurrence::rWeekly && !KOPrefs::instance()->mWeeklyRecur)) {
                KCalendarCore::DateTimeList timeDateList;
                const bool isRecurrent = ev->recurs();
                const int eventDuration = dtStart.daysTo(dtEnd);

                if (isRecurrent) {
                    // Its a recurring event, find out in which days it occurs
                    timeDateList = ev->recurrence()->timesInInterval(QDateTime(mDays[0], {}, QTimeZone::LocalTime),
                                                                     QDateTime(mDays[NUMDAYS - 1], {}, QTimeZone::LocalTime));
                } else {
                    if (dtStart.date() >= mDays[0]) {
                        timeDateList.append(dtStart);
                    } else {
                        // The event starts in another month (not visible))
                        timeDateList.append(QDateTime(mDays[0], {}, QTimeZone::LocalTime));
                    }
                }

                for (auto t = timeDateList.begin(); t != timeDateList.end(); ++t) {
                    // This could be a multiday event, so iterate from dtStart() to dtEnd()
                    QDate d = t->toLocalTime().date();
                    int j = 0;

                    QDate occurrenceEnd;
                    if (isRecurrent) {
                        occurrenceEnd = d.addDays(eventDuration);
                    } else {
                        occurrenceEnd = dtEnd.date();
                    }

                    do {
                        mEvents.append(d);
                        ++j;
                        d = d.addDays(1);
                    } while (d <= occurrenceEnd && j < NUMDAYS);
                }
            }
        }
        mPendingChanges = false;
    }
}

const QDate &KODayMatrix::getDate(int offset) const
{
    if (offset < 0 || offset > NUMDAYS - 1) {
        return mDays[0];
    }
    return mDays[offset];
}

QString KODayMatrix::getHolidayLabel(int offset) const
{
    if (offset < 0 || offset > NUMDAYS - 1) {
        return {};
    }
    return mHolidays[offset];
}

int KODayMatrix::getDayIndexFrom(int x, int y) const
{
    return 7 * (y / mDaySize.height()) + (KOGlobals::self()->reverseLayout() ? 6 - x / mDaySize.width() : x / mDaySize.width());
}

void KODayMatrix::calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence)
    mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence)
    mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *calendar)
{
    Q_UNUSED(incidence)
    Q_UNUSED(calendar)
    mPendingChanges = true;
}

void KODayMatrix::setHighlightMode(bool highlightEvents, bool highlightTodos, bool highlightJournals)
{
    // don't set mPendingChanges to true if nothing changed
    if (highlightTodos != mHighlightTodos || highlightEvents != mHighlightEvents || highlightJournals != mHighlightJournals) {
        mHighlightEvents = highlightEvents;
        mHighlightTodos = highlightTodos;
        mHighlightJournals = highlightJournals;
        mPendingChanges = true;
    }
}

void KODayMatrix::resourcesChanged()
{
    mPendingChanges = true;
}

// ----------------------------------------------------------------------------
//  M O U S E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

bool KODayMatrix::event(QEvent *event)
{
    if (KOPrefs::instance()->mEnableToolTips && event->type() == QEvent::ToolTip) {
        auto helpEvent = static_cast<QHelpEvent *>(event);

        // calculate which cell of the matrix the mouse is in
        QRect const sz = frameRect();
        int const dheight = sz.height() * 7 / 42;
        int const dwidth = sz.width() / 7;
        int const row = helpEvent->pos().y() / dheight;
        int const col = helpEvent->pos().x() / dwidth;

        // show holiday names only
        QString const tipText = getHolidayLabel(col + row * 7);
        if (!tipText.isEmpty()) {
            QToolTip::showText(helpEvent->globalPos(), tipText);
        } else {
            QToolTip::hideText();
        }
    }
    return QFrame::event(event);
}

void KODayMatrix::mousePressEvent(QMouseEvent *e)
{
    mSelStart = getDayIndexFrom(e->position().toPoint().x(), e->position().toPoint().y());
    if (e->button() == Qt::RightButton) {
        popupMenu(mDays[mSelStart]);
    } else if (e->button() == Qt::LeftButton) {
        if (mSelStart > NUMDAYS - 1) {
            mSelStart = NUMDAYS - 1;
        }
        mSelInit = mSelStart;
    }
}
void KODayMatrix::popupMenu(const QDate &date)
{
    QMenu popup(this);
    popup.setTitle(date.toString());
    const QAction *newEventAction = popup.addAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18n("New E&vent…"));
    const QAction *newTodoAction = popup.addAction(QIcon::fromTheme(QStringLiteral("task-new")), i18n("New &To-do…"));
    const QAction *newJournalAction = popup.addAction(QIcon::fromTheme(QStringLiteral("journal-new")), i18n("New &Journal…"));
    const QAction *ret = popup.exec(QCursor::pos());
    if (ret == newEventAction) {
        Q_EMIT newEventSignal(date);
    } else if (ret == newTodoAction) {
        Q_EMIT newTodoSignal(date);
    } else if (ret == newJournalAction) {
        Q_EMIT newJournalSignal(date);
    }
}

void KODayMatrix::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }
    int tmp = getDayIndexFrom(e->position().toPoint().x(), e->position().toPoint().y());
    if (tmp > NUMDAYS - 1) {
        tmp = NUMDAYS - 1;
    }

    if (mSelInit > tmp) {
        mSelEnd = mSelInit;
        if (tmp != mSelStart) {
            mSelStart = tmp;
            update();
        }
    } else {
        mSelStart = mSelInit;

        // repaint only if selection has changed
        if (tmp != mSelEnd) {
            mSelEnd = tmp;
            update();
        }
    }

    KCalendarCore::DateList daylist;
    if (mSelStart < 0) {
        mSelStart = 0;
    }
    daylist.reserve(mSelEnd - mSelStart + 1);
    for (int i = mSelStart; i <= mSelEnd; ++i) {
        daylist.append(mDays[i]);
    }
    Q_EMIT selected(static_cast<const KCalendarCore::DateList>(daylist));
}

void KODayMatrix::mouseMoveEvent(QMouseEvent *e)
{
    int tmp = getDayIndexFrom(e->position().toPoint().x(), e->position().toPoint().y());
    if (tmp > NUMDAYS - 1) {
        tmp = NUMDAYS - 1;
    }

    if (mSelInit > tmp) {
        mSelEnd = mSelInit;
        if (tmp != mSelStart) {
            mSelStart = tmp;
            update();
        }
    } else {
        mSelStart = mSelInit;

        // repaint only if selection has changed
        if (tmp != mSelEnd) {
            mSelEnd = tmp;
            update();
        }
    }
}

// ----------------------------------------------------------------------------
//  D R A G ' N   D R O P   H A N D L I N G
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Drag and Drop handling -- based on the Troll Tech dirview example

enum {
    DRAG_COPY = 0,
    DRAG_MOVE = 1,
    DRAG_CANCEL = 2
};

void KODayMatrix::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
    const QMimeData *md = e->mimeData();
    if (!CalendarSupport::canDecode(md)) {
        e->ignore();
        return;
    }

    // some visual feedback
    //  oldPalette = palette();
    //  setPalette(my_HilitePalette);
    //  update();
}

void KODayMatrix::dragMoveEvent(QDragMoveEvent *e)
{
    const QMimeData *md = e->mimeData();
    if (!CalendarSupport::canDecode(md)) {
        e->ignore();
        return;
    }
    e->accept();
}

void KODayMatrix::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_UNUSED(e)
    //  setPalette(oldPalette);
    //  update();
}

void KODayMatrix::dropEvent(QDropEvent *e)
{
    QList<QUrl> const urls = (e->mimeData()->urls());
    // qCDebug(KORGANIZER_LOG)<<" urls :"<<urls;
    if (urls.isEmpty()) {
        e->ignore();
        return;
    }
    // For the moment support 1 url
    if (urls.count() >= 1) {
        QUrl const &res = urls.at(0);

        auto job = new Akonadi::ItemFetchJob(Akonadi::Item::fromUrl(res));
        job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
        job->fetchScope().fetchFullPayload();
        Akonadi::Item::List items;
        if (job->exec()) {
            items = job->items();
        }
        bool const exist = items.at(0).isValid();
        int action = DRAG_CANCEL;
        Qt::KeyboardModifiers const keyboardModifiers = e->modifiers();
        if (keyboardModifiers & Qt::ControlModifier) {
            action = DRAG_COPY;
        } else if (keyboardModifiers & Qt::ShiftModifier) {
            action = DRAG_MOVE;
        } else {
            const QAction *copy = nullptr;
            const QAction *move = nullptr;
            auto menu = new QMenu(this);
            if (exist) {
                move = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("&Move"));
                if (/*existingEvent*/ true) {
                    copy = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("&Copy"));
                }
            } else {
                move = menu->addAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("&Add"));
            }
            menu->addSeparator();
            /*QAction *cancel =*/
            menu->addAction(QIcon::fromTheme(QStringLiteral("process-stop")), i18n("&Cancel"));
            const QAction *a = menu->exec(QCursor::pos());
            if (a == copy) {
                action = DRAG_COPY;
            } else if (a == move) {
                action = DRAG_MOVE;
            }
            delete menu;
        }

        if (action == DRAG_COPY || action == DRAG_MOVE) {
            e->accept();
            int const idx = getDayIndexFrom(e->position().toPoint().x(), e->position().toPoint().y());

            if (action == DRAG_COPY) {
                Q_EMIT incidenceDropped(items.at(0), mDays[idx]);
            } else if (action == DRAG_MOVE) {
                Q_EMIT incidenceDroppedMove(items.at(0), mDays[idx]);
            }
        }
    }
}

// ----------------------------------------------------------------------------
//  P A I N T   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::paintEvent(QPaintEvent *)
{
    QPainter p;
    const QRect rect = frameRect();
    const int dayHeight = mDaySize.height();
    const int dayWidth = mDaySize.width();
    int row;
    int column;
    const bool isRTL = KOGlobals::self()->reverseLayout();

    QPalette const pal = palette();

    p.begin(this);

    // draw background
    p.fillRect(0, 0, rect.width(), rect.height(), QBrush(pal.color(QPalette::Base)));

    // draw topleft frame
    p.setPen(pal.color(QPalette::Mid));
    p.drawRect(0, 0, rect.width() - 1, rect.height() - 1);
    // don't paint over borders
    p.translate(1, 1);

    // draw selected days with highlighted background color
    if (mSelStart != NOSELECTION) {
        row = mSelStart / 7;
        // fix larger selections starting in the previous month
        if (row < 0 && mSelEnd > 0) {
            row = 0;
        }
        column = mSelStart - row * 7;
        const QColor selectionColor = KOPrefs::instance()->agendaGridHighlightColor();

        if (row < 6 && row >= 0) {
            if (row == mSelEnd / 7) {
                // Single row selection
                p.fillRect(isRTL ? (7 - (mSelEnd - mSelStart + 1) - column) * dayWidth : column * dayWidth,
                           row * dayHeight,
                           (mSelEnd - mSelStart + 1) * dayWidth,
                           dayHeight,
                           selectionColor);
            } else {
                // draw first row to the right
                p.fillRect(isRTL ? 0 : column * dayWidth, row * dayHeight, (7 - column) * dayWidth, dayHeight, selectionColor);
                // draw full block till last line
                int selectionHeight = mSelEnd / 7 - row;
                if (selectionHeight + row >= 6) {
                    selectionHeight = 6 - row;
                }
                if (selectionHeight > 1) {
                    p.fillRect(0, (row + 1) * dayHeight, 7 * dayWidth, (selectionHeight - 1) * dayHeight, selectionColor);
                }
                // draw last block from left to mSelEnd
                if (mSelEnd / 7 < 6) {
                    int const selectionWidth = mSelEnd - 7 * (mSelEnd / 7) + 1;
                    p.fillRect(isRTL ? (7 - selectionWidth) * dayWidth : 0,
                               (row + selectionHeight) * dayHeight,
                               selectionWidth * dayWidth,
                               dayHeight,
                               selectionColor);
                }
            }
        }
    }

    // iterate over all days in the matrix and draw the day label in appropriate colors
    const QColor textColor = pal.color(QPalette::Text);
    const QColor textColorShaded = getShadedColor(textColor);
    QColor actcol = textColorShaded;
    p.setPen(actcol);
    QPen tmppen;

    const QList<QDate> workDays = CalendarSupport::workDays(mDays[0], mDays[NUMDAYS - 1]);
    for (int i = 0; i < NUMDAYS; ++i) {
        row = i / 7;
        column = isRTL ? 6 - (i - row * 7) : i - row * 7;

        // if it is the first day of a month switch color from normal to shaded and vice versa
        if (mDays[i].day() == 1) {
            if (actcol == textColorShaded) {
                actcol = textColor;
            } else {
                actcol = textColorShaded;
            }
            p.setPen(actcol);
        }

        // Reset pen color after selected days block
        if (i == mSelEnd + 1) {
            p.setPen(actcol);
        }

        const bool holiday = !workDays.contains(mDays[i]);

        const QColor holidayColorShaded = getShadedColor(KOPrefs::instance()->agendaHolidaysBackgroundColor());

        // if today then draw rectangle around day
        if (mToday == i) {
            tmppen = p.pen();
            QPen todayPen(p.pen());

            todayPen.setWidth(mTodayMarginWidth);
            // draw red rectangle for holidays
            if (holiday) {
                if (actcol == textColor) {
                    todayPen.setColor(KOPrefs::instance()->agendaHolidaysBackgroundColor());
                } else {
                    todayPen.setColor(holidayColorShaded);
                }
            }
            // draw gray rectangle for today if in selection
            if (i >= mSelStart && i <= mSelEnd) {
                const QColor grey(QStringLiteral("grey"));
                todayPen.setColor(grey);
            }
            p.setPen(todayPen);
            p.drawRect(column * dayWidth, row * dayHeight, dayWidth, dayHeight);
            p.setPen(tmppen);
        }

        // if any events are on that day then draw it using a bold font
        if (mEvents.contains(mDays[i])) {
            QFont myFont = font();
            myFont.setBold(true);
            p.setFont(myFont);
        }

        // if it is a holiday then use the default holiday color
        if (holiday) {
            if (actcol == textColor) {
                p.setPen(KOPrefs::instance()->agendaHolidaysBackgroundColor());
            } else {
                p.setPen(holidayColorShaded);
            }
        }

        // draw selected days with special color
        if (i >= mSelStart && i <= mSelEnd && !holiday) {
            p.setPen(Qt::white);
        }

        p.drawText(column * dayWidth, row * dayHeight, dayWidth, dayHeight, Qt::AlignHCenter | Qt::AlignVCenter, mDayLabels[i]);

        // reset color to actual color
        if (holiday) {
            p.setPen(actcol);
        }
        // reset bold font to plain font
        if (mEvents.contains(mDays[i])) {
            QFont myFont = font();
            myFont.setBold(false);
            p.setFont(myFont);
        }
    }
    p.end();
}

// ----------------------------------------------------------------------------
//  R E SI Z E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::resizeEvent(QResizeEvent *)
{
    QRect const sz = frameRect();
    mDaySize.setHeight(sz.height() * 7 / NUMDAYS);
    mDaySize.setWidth(sz.width() / 7);
}

/* static */
QPair<QDate, QDate> KODayMatrix::matrixLimits(QDate month)
{
    QDate d(month.year(), month.month(), 1);

    const int dayOfWeek = d.dayOfWeek();
    const int weekstart = KOGlobals::self()->firstDayOfWeek();

    d = d.addDays(-(7 + dayOfWeek - weekstart) % 7);

    if (dayOfWeek == weekstart) {
        d = d.addDays(-7); // Start on the second line
    }

    return qMakePair(d, d.addDays(NUMDAYS - 1));
}

#include "moc_kodaymatrix.cpp"
