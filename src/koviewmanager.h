/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <EventViews/EventView>

#include <KCalendarCore/IncidenceBase> //for KCalendarCore::DateList typedef

#include <QDate>
#include <QList>
#include <QObject>

class CalendarView;
class KOAgendaView;
class KOJournalView;
class KOListView;
class KOTimelineView;
class KOTodoView;
class KOWhatsNextView;
namespace KOrg
{
class BaseView;
class MultiAgendaView;
class MonthView;
}

namespace Akonadi
{
class Item;
}

class KActionCollection;
class KConfig;
class QTabWidget;

/**
  This class manages the views of the calendar. It owns the objects and handles
  creation and selection.
*/
class KOViewManager : public QObject
{
    Q_OBJECT
public:
    enum RangeMode {
        NO_RANGE,
        DAY_RANGE,
        WORK_WEEK_RANGE,
        WEEK_RANGE,
        NEXTX_RANGE,
        OTHER_RANGE // for example, showing 8 days
    };

    explicit KOViewManager(CalendarView *);
    ~KOViewManager() override;

    /** changes the view to be the currently selected view */
    void showView(KOrg::BaseView *);

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    KOrg::BaseView *currentView();

    void setDocumentId(const QString &);

    void updateView();
    void updateView(QDate start, QDate end, QDate preferredMonth);

    void goMenu(bool enable);
    void raiseCurrentView();

    void connectView(KOrg::BaseView *);
    void addView(KOrg::BaseView *, bool isTab = false);

    [[nodiscard]] Akonadi::Item currentSelection();

    /**
     * If there's a selected incidence, it's date is returned, otherwise
     * an invalid QDate is returned.
     */
    [[nodiscard]] QDate currentSelectionDate();

    KOAgendaView *agendaView() const
    {
        return mAgendaView;
    }

    KOrg::MultiAgendaView *multiAgendaView() const
    {
        return mAgendaSideBySideView;
    }

    KOTodoView *todoView() const
    {
        return mTodoView;
    }

    KOrg::MonthView *monthView() const
    {
        return mMonthView;
    }

    void updateMultiCalendarDisplay();

    /**
     * Returns true if agenda is the current view.
     *
     * Never use the pointer returned by agendaView()
     * to know if agenda is selected, because agenda has other modes
     * (tabbed, side by side). Use this function instead.
     */
    [[nodiscard]] bool agendaIsSelected() const;

    /**
      Return the current range mode:
      week, work week, day or nextX days, etc.
    */
    [[nodiscard]] RangeMode rangeMode() const
    {
        return mRangeMode;
    }

Q_SIGNALS:
    void configChanged();
    void datesSelected(const KCalendarCore::DateList &);

public Q_SLOTS:
    void showWhatsNextView();
    void showListView();
    void showAgendaView();
    void showTodoView();
    void showTimeLineView();
    void showMonthView();
    void showJournalView();
    void showEventView();

    void selectDay();
    void selectWorkWeek();
    void selectWeek();
    void selectNextX();

    void connectTodoView(KOTodoView *todoView);

    void zoomInHorizontally();
    void zoomOutHorizontally();
    void zoomInVertically();
    void zoomOutVertically();

    /**
       Notifies all views that an update is needed. This means that the
       next time CalendarView::updateView() is called, views won't try to be smart
       and ignore the update for performance reasons.
    */
    void addChange(EventViews::EventView::Change change);

private Q_SLOTS:
    void currentAgendaViewTabChanged(int index);
    void addCalendar(const Akonadi::CollectionCalendar::Ptr &calendar);
    void removeCalendar(const Akonadi::CollectionCalendar::Ptr &calendar);

private:
    KActionCollection *getActionCollection();
    [[nodiscard]] bool isAgendaViewAction(QAction *, KActionCollection *);
    [[nodiscard]] QAction *viewToAction(const QString &, RangeMode);
    void viewActionEnable(QObject *);
    void viewActionEnable(QAction *);
    QWidget *widgetForView(KOrg::BaseView *) const;
    QList<KOrg::BaseView *> mViews;
    CalendarView *const mMainView;
    QList<Akonadi::CollectionCalendar::Ptr> mCalendars;

    KOAgendaView *mAgendaView = nullptr;
    KOrg::MultiAgendaView *mAgendaSideBySideView = nullptr;
    KOListView *mListView = nullptr;
    KOTodoView *mTodoView = nullptr;
    KOWhatsNextView *mWhatsNextView = nullptr;
    KOJournalView *mJournalView = nullptr;
    KOTimelineView *mTimelineView = nullptr;
    KOrg::MonthView *mMonthView = nullptr;
    KOrg::BaseView *mCurrentView = nullptr;

    KOrg::BaseView *mLastEventView = nullptr;
    QTabWidget *mAgendaViewTabs = nullptr;
    int mAgendaViewTabIndex = 0;
    QAction *mLastViewAction = nullptr;

    RangeMode mRangeMode = NO_RANGE;
};
