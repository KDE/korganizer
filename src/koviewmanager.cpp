/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koviewmanager.h"
#include "actionmanager.h"
#include "akonadicollectionview.h"
#include "calendarview.h"
#include "datenavigator.h"
#include "koglobals.h"
#include "mainwindow.h"
#include "prefs/koprefs.h"
#include "views/agendaview/koagendaview.h"
#include "views/journalview/kojournalview.h"
#include "views/listview/kolistview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/timelineview/kotimelineview.h"
#include "views/todoview/kotodoview.h"
#include "views/whatsnextview/kowhatsnextview.h"
#include "widgets/navigatorbar.h"

#include <Akonadi/EntityTreeModel>

#include <KActionCollection>
#include <KMessageBox>
#include <QTabWidget>

#include <KSharedConfig>
#include <QAction>
#include <QStackedWidget>

KOViewManager::KOViewManager(CalendarView *mainView)
    : mMainView(mainView)
{
    connect(mainView, &CalendarView::calendarAdded, this, &KOViewManager::addCalendar);
    connect(mainView, &CalendarView::calendarRemoved, this, &KOViewManager::removeCalendar);
}

KOViewManager::~KOViewManager() = default;

KOrg::BaseView *KOViewManager::currentView()
{
    return mCurrentView;
}

KActionCollection *KOViewManager::getActionCollection()
{
    KActionCollection *collection = nullptr;
    KOrg::MainWindow *w = ActionManager::findInstance(QUrl());
    if (w) {
        collection = w->getActionCollection();
    }
    return collection;
}

QAction *KOViewManager::viewToAction(const QString &view, RangeMode rangeMode)
{
    QAction *action = nullptr;
    KActionCollection *ac = getActionCollection();
    if (!ac) {
        return action;
    }

    if (view == QLatin1StringView("WhatsNext")) {
        action = ac->action(QStringLiteral("view_whatsnext"));
    } else if (view == QLatin1StringView("OldMonth")) {
        // the oldmonth view is gone, so we assume the new month view
        action = ac->action(QStringLiteral("view_month"));
    } else if (view == QLatin1StringView("List")) {
        action = ac->action(QStringLiteral("view_list"));
    } else if (view == QLatin1StringView("Journal")) {
        action = ac->action(QStringLiteral("view_journal"));
    } else if (view == QLatin1StringView("Todo")) {
        action = ac->action(QStringLiteral("view_todo"));
    } else if (view == QLatin1StringView("Timeline")) {
        action = ac->action(QStringLiteral("view_timeline"));
    } else if (view == QLatin1StringView("Month")) {
        action = ac->action(QStringLiteral("view_month"));
    } else if (view == QLatin1StringView("Agenda")) {
        switch (rangeMode) {
        case WORK_WEEK_RANGE:
            action = ac->action(QStringLiteral("select_workweek"));
            break;
        case WEEK_RANGE:
            action = ac->action(QStringLiteral("select_week"));
            break;
        case NEXTX_RANGE:
            action = ac->action(QStringLiteral("select_nextx"));
            break;
        case DAY_RANGE:
            action = ac->action(QStringLiteral("select_day"));
            break;
        case NO_RANGE:
        default:
            break;
        }
    }
    Q_ASSERT_X(action, "no such view", qPrintable(view));
    return action;
}

void KOViewManager::readSettings(KConfig *config)
{
    KConfigGroup const generalConfig(config, QStringLiteral("General"));
    const QString view = generalConfig.readEntry("Current View");

    if (view == QLatin1StringView("WhatsNext")) {
        showWhatsNextView();
    } else if (view == QLatin1StringView("OldMonth")) {
        // the oldmonth view is gone, so we assume the new month view
        showMonthView();
    } else if (view == QLatin1StringView("List")) {
        showListView();
        mListView->readSettings(config);
    } else if (view == QLatin1StringView("Journal")) {
        showJournalView();
    } else if (view == QLatin1StringView("Todo")) {
        showTodoView();
    } else if (view == QLatin1StringView("Timeline")) {
        showTimeLineView();
    } else if (view == QLatin1StringView("Month")) {
        showMonthView();
    } else {
        showAgendaView();
    }

    mRangeMode = RangeMode(generalConfig.readEntry("Range Mode", int(OTHER_RANGE)));

    switch (mRangeMode) {
    case WORK_WEEK_RANGE:
        selectWorkWeek();
        break;
    case WEEK_RANGE:
        selectWeek();
        break;
    case NEXTX_RANGE:
        selectNextX();
        break;
    case DAY_RANGE:
        selectDay();
        break;
    case NO_RANGE:
    default:
        // Someone has been playing with the config file.
        mRangeMode = OTHER_RANGE;
    }
}

void KOViewManager::writeSettings(KConfig *config)
{
    KConfigGroup generalConfig(config, QStringLiteral("General"));
    QString view;
    if (mCurrentView == mWhatsNextView) {
        view = QStringLiteral("WhatsNext");
    } else if (mCurrentView == mListView) {
        view = QStringLiteral("List");
    } else if (mCurrentView == mJournalView) {
        view = QStringLiteral("Journal");
    } else if (mCurrentView == mTodoView) {
        view = QStringLiteral("Todo");
    } else if (mCurrentView == mTimelineView) {
        view = QStringLiteral("Timeline");
    } else if (mCurrentView == mMonthView) {
        view = QStringLiteral("Month");
    } else {
        view = QStringLiteral("Agenda");
    }

    generalConfig.writeEntry("Current View", view);

    if (mAgendaView) {
        mAgendaView->writeSettings(config);
    }
    if (mListView) {
        mListView->writeSettings(config);
    }
    if (mTodoView) {
        mTodoView->saveLayout(config, QStringLiteral("Todo View"));
    }

    // write out custom view configuration
    for (KOrg::BaseView *const baseView : std::as_const(mViews)) {
        KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1StringView(baseView->identifier()));
        baseView->saveConfig(group);
    }

    generalConfig.writeEntry("Range Mode", int(mRangeMode));
}

void KOViewManager::showView(KOrg::BaseView *view)
{
    if (view == mCurrentView) {
        return;
    }

    mCurrentView = view;
    mMainView->updateHighlightModes();

    if (mCurrentView && mCurrentView->isEventView()) {
        mLastEventView = mCurrentView;
    }

    if (mAgendaView) {
        mAgendaView->deleteSelectedDateTime();
    }

    raiseCurrentView();
    mMainView->processIncidenceSelection(Akonadi::Item(), QDate());
    mMainView->updateView();
    KActionCollection *ac = getActionCollection();
    if (ac) {
        if (QAction *action = ac->action(QStringLiteral("configure_view"))) {
            action->setEnabled(view->hasConfigurationDialog());
        }

        const QStringList zoomActions = QStringList() << QStringLiteral("zoom_in_horizontally") << QStringLiteral("zoom_out_horizontally")
                                                      << QStringLiteral("zoom_in_vertically") << QStringLiteral("zoom_out_vertically");
        for (int i = 0; i < zoomActions.size(); ++i) {
            if (QAction *action = ac->action(zoomActions[i])) {
                action->setEnabled(view->supportsZoom());
            }
        }
    }
}

void KOViewManager::goMenu(bool enable)
{
    KActionCollection *ac = getActionCollection();
    if (ac) {
        QAction *action = ac->action(QStringLiteral("go_today"));
        if (action) {
            action->setEnabled(enable);
        }
        action = ac->action(QStringLiteral("go_previous"));
        if (action) {
            action->setEnabled(enable);
        }
        action = ac->action(QStringLiteral("go_next"));
        if (action) {
            action->setEnabled(enable);
        }
    }
}

void KOViewManager::raiseCurrentView()
{
    if (mCurrentView && mCurrentView->usesFullWindow()) {
        mMainView->showLeftFrame(false);
        if (mCurrentView == mTodoView) {
            mMainView->navigatorBar()->hide();
        } else {
            mMainView->navigatorBar()->show();
        }
    } else {
        mMainView->showLeftFrame(true);
        mMainView->navigatorBar()->hide();
    }
    mMainView->viewStack()->setCurrentWidget(widgetForView(mCurrentView));
}

void KOViewManager::updateView()
{
    if (mCurrentView) {
        mCurrentView->updateView();
    }
}

void KOViewManager::updateView(QDate start, QDate end, QDate preferredMonth)
{
    if (mCurrentView && mCurrentView != mTodoView) {
        mCurrentView->setDateRange(QDateTime(start.startOfDay()), QDateTime(end.startOfDay()), preferredMonth);
    } else if (mTodoView) {
        mTodoView->updateView();
    }
}

void KOViewManager::connectView(KOrg::BaseView *view)
{
    if (!view) {
        return;
    }

    if (view->isEventView()) {
        connect(static_cast<KOEventView *>(view), &KOEventView::datesSelected, this, &KOViewManager::datesSelected);
    }

    // selecting an incidence
    connect(view, &BaseView::incidenceSelected, mMainView, &CalendarView::processMainViewSelection);

    // showing/editing/deleting an incidence. The calendar view takes care of the action.
    connect(view, &BaseView::showOccurrenceSignal, mMainView, &CalendarView::showOccurrence);
    connect(view, &BaseView::showIncidenceSignal, mMainView, qOverload<const Akonadi::Item &>(&CalendarView::showIncidence));
    connect(view, &BaseView::editIncidenceSignal, this, [this](const Akonadi::Item &i) {
        mMainView->editIncidence(i, false);
    });
    connect(view, &BaseView::deleteIncidenceSignal, this, [this](const Akonadi::Item &i) {
        mMainView->deleteIncidence(i, false);
    });
    connect(view, &BaseView::copyIncidenceSignal, mMainView, &CalendarView::copyIncidence);
    connect(view, &BaseView::cutIncidenceSignal, mMainView, &CalendarView::cutIncidence);
    connect(view, &BaseView::pasteIncidenceSignal, mMainView, &CalendarView::pasteIncidence);
    connect(view, &BaseView::toggleAlarmSignal, mMainView, &CalendarView::toggleAlarm);
    connect(view, &BaseView::toggleTodoCompletedSignal, mMainView, &CalendarView::toggleTodoCompleted);
    connect(view, &BaseView::toggleOccurrenceCompletedSignal, mMainView, &CalendarView::toggleOccurrenceCompleted);
    connect(view, &BaseView::copyIncidenceToResourceSignal, mMainView, &CalendarView::copyIncidenceToResource);
    connect(view, &BaseView::moveIncidenceToResourceSignal, mMainView, &CalendarView::moveIncidenceToResource);
    connect(view, &BaseView::dissociateOccurrencesSignal, mMainView, &CalendarView::dissociateOccurrences);

    // signals to create new incidences
    connect(view, qOverload<>(&BaseView::newEventSignal), mMainView, qOverload<>(&CalendarView::newEvent));
    connect(view, qOverload<const QDateTime &>(&BaseView::newEventSignal), mMainView, qOverload<const QDateTime &>(&CalendarView::newEvent));
    connect(view, qOverload<const QDateTime &, const QDateTime &>(&BaseView::newEventSignal), this, [this](const QDateTime &s, const QDateTime &e) {
        mMainView->newEvent(s, e, false);
    });
    connect(view, qOverload<const QDate &>(&BaseView::newEventSignal), mMainView, qOverload<const QDate &>(&CalendarView::newEvent));

    connect(view, qOverload<const QDate &>(&BaseView::newTodoSignal), mMainView, qOverload<const QDate &>(&CalendarView::newTodo));
    connect(view, qOverload<const Akonadi::Item &>(&BaseView::newSubTodoSignal), mMainView, qOverload<const Akonadi::Item &>(&CalendarView::newSubTodo));
    connect(view, &BaseView::newJournalSignal, mMainView, qOverload<const QDate &>(&CalendarView::newJournal));

    // reload settings
    connect(mMainView, &CalendarView::configChanged, view, &KOrg::BaseView::updateConfig);

    // Notifications about added, changed and deleted incidences
    connect(mMainView, &CalendarView::dayPassed, view, &BaseView::dayPassed);
    connect(view, &BaseView::startMultiModify, mMainView, &CalendarView::startMultiModify);
    connect(view, &BaseView::endMultiModify, mMainView, &CalendarView::endMultiModify);

    connect(mMainView, &CalendarView::calendarAdded, view, &BaseView::calendarAdded);
    connect(mMainView, &CalendarView::calendarRemoved, view, &BaseView::calendarRemoved);

    view->setIncidenceChanger(mMainView->incidenceChanger());
}

void KOViewManager::connectTodoView(KOTodoView *todoView)
{
    if (!todoView) {
        return;
    }

    // SIGNALS/SLOTS FOR TODO VIEW
    connect(todoView, &KOTodoView::purgeCompletedSignal, mMainView, &CalendarView::purgeCompleted);
    connect(todoView, &KOTodoView::unSubTodoSignal, mMainView, &CalendarView::todo_unsub);
    connect(todoView, &KOTodoView::unAllSubTodoSignal, mMainView, &CalendarView::makeSubTodosIndependent);

    connect(todoView, &KOTodoView::fullViewChanged, mMainView, &CalendarView::changeFullView);
}

void KOViewManager::zoomInHorizontally()
{
    if (mAgendaView == mCurrentView) {
        mAgendaView->zoomInHorizontally();
    }
}

void KOViewManager::zoomOutHorizontally()
{
    if (mAgendaView == mCurrentView) {
        mAgendaView->zoomOutHorizontally();
    }
}

void KOViewManager::zoomInVertically()
{
    if (mAgendaView == mCurrentView) {
        mAgendaView->zoomInVertically();
    }
}

void KOViewManager::zoomOutVertically()
{
    if (mAgendaView == mCurrentView) {
        mAgendaView->zoomOutVertically();
    }
}

void KOViewManager::addView(KOrg::BaseView *view, bool isTab)
{
    view->setModel(mMainView->calendar()->entityTreeModel());
    connectView(view);
    mViews.append(view);
    const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1StringView(view->identifier()));
    view->restoreConfig(group);
    if (!isTab) {
        mMainView->viewStack()->addWidget(view);
    }
    for (const auto &calendar : std::as_const(mCalendars)) {
        view->calendarAdded(calendar);
    }
}

void KOViewManager::viewActionEnable(QObject *obj)
{
    viewActionEnable(qobject_cast<QAction *>(obj));
}

void KOViewManager::viewActionEnable(QAction *action)
{
    if (!action) {
        return;
    }

    KActionCollection *ac = getActionCollection();
    if (!ac) {
        return;
    }

    /* Do nothing -- the action hasn't changed since last time */
    if (mLastViewAction && action == mLastViewAction) {
        return;
    }

    /* Do nothing for top-level agenda view button */
    if (action == ac->action(QStringLiteral("view_agenda"))) {
        return;
    }

    /* Re-enable last view action */
    if (mLastViewAction) {
        mLastViewAction->setEnabled(true);
    }

    /* Disable the any newly selected action */
    action->setEnabled(false);

    /* Save for the next time */
    mLastViewAction = action;
}

void KOViewManager::showMonthView()
{
    if (!mMonthView) {
        mMonthView = new KOrg::MonthView(mMainView->viewStack());
        mMonthView->setIdentifier("DefaultMonthView");
        addView(mMonthView);
        connect(mMonthView, &MonthView::fullViewChanged, mMainView, &CalendarView::changeFullView);
    }
    viewActionEnable(viewToAction(QStringLiteral("Month"), NO_RANGE));
    goMenu(true);
    showView(mMonthView);
}

void KOViewManager::showWhatsNextView()
{
    if (!mWhatsNextView) {
        mWhatsNextView = new KOWhatsNextView(mMainView->viewStack());
        mWhatsNextView->setIdentifier("DefaultWhatsNextView");
        addView(mWhatsNextView);
    }
    viewActionEnable(viewToAction(QStringLiteral("WhatsNext"), NO_RANGE));
    goMenu(true);
    showView(mWhatsNextView);
}

void KOViewManager::showListView()
{
    if (!mListView) {
        mListView = new KOListView(mMainView->viewStack());
        mListView->setIdentifier("DefaultListView");
        addView(mListView);
    }
    viewActionEnable(viewToAction(QStringLiteral("List"), NO_RANGE));
    goMenu(true);
    showView(mListView);
}

void KOViewManager::showAgendaView()
{
    const bool showBoth = KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::AllCalendarViews;
    const bool showMerged = showBoth || KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsMerged;
    const bool showSideBySide = showBoth || KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsSideBySide;

    QWidget *parent = mMainView->viewStack();
    if (showBoth) {
        if (!mAgendaViewTabs) {
            mAgendaViewTabs = new QTabWidget(mMainView->viewStack());
            connect(mAgendaViewTabs, &QTabWidget::currentChanged, this, &KOViewManager::currentAgendaViewTabChanged);
            mMainView->viewStack()->addWidget(mAgendaViewTabs);

            KConfigGroup const viewConfig = KSharedConfig::openConfig()->group(QStringLiteral("Views"));
            mAgendaViewTabIndex = viewConfig.readEntry("Agenda View Tab Index", 0);
        }
        parent = mAgendaViewTabs;
    }

    if (showMerged) {
        if (!mAgendaView) {
            mAgendaView = new KOAgendaView(parent);
            mAgendaView->setIdentifier("DefaultAgendaView");

            addView(mAgendaView, showBoth);

            connect(mAgendaView, &KOAgendaView::zoomViewHorizontally, this, [this](const QDate &d, int n) {
                mMainView->dateNavigator()->selectDates(d, n, QDate());
            });
            auto config = KSharedConfig::openConfig();
            mAgendaView->readSettings(config.data());
        }
        if (showBoth && mAgendaViewTabs->indexOf(mAgendaView) < 0) {
            mAgendaViewTabs->addTab(mAgendaView, i18n("Merged calendar"));
        } else if (!showBoth && mMainView->viewStack()->indexOf(mAgendaView) < 0) {
            mAgendaView->setParent(parent);
            mMainView->viewStack()->addWidget(mAgendaView);
        }
    }

    if (showSideBySide) {
        if (!mAgendaSideBySideView) {
            mAgendaSideBySideView = new MultiAgendaView(mMainView, parent);
            mAgendaSideBySideView->setIdentifier("DefaultAgendaSideBySideView");
            mAgendaSideBySideView->setCollectionSelectionProxyModel(mMainView->calendar()->checkableProxyModel());
            addView(mAgendaSideBySideView, showBoth);
        }
        if (showBoth && mAgendaViewTabs->indexOf(mAgendaSideBySideView) < 0) {
            mAgendaViewTabs->addTab(mAgendaSideBySideView, i18n("Calendars Side by Side"));
            mAgendaViewTabs->setCurrentIndex(mAgendaViewTabIndex);
        } else if (!showBoth && mMainView->viewStack()->indexOf(mAgendaSideBySideView) < 0) {
            mAgendaSideBySideView->setParent(parent);
            mMainView->viewStack()->addWidget(mAgendaSideBySideView);
        }
    }

    goMenu(true);

    if (showBoth) {
        showView(static_cast<KOrg::BaseView *>(mAgendaViewTabs->currentWidget()));
    } else if (showMerged) {
        showView(mAgendaView);
    } else if (showSideBySide) {
        showView(mAgendaSideBySideView);
    }
    viewActionEnable(viewToAction(QStringLiteral("Agenda"), mRangeMode));
}

void KOViewManager::selectDay()
{
    showAgendaView();
    mRangeMode = DAY_RANGE;
    viewActionEnable(viewToAction(QStringLiteral("Agenda"), mRangeMode));
    const QDate date = mMainView->activeDate(true);
    mMainView->dateNavigator()->selectDate(date);
}

void KOViewManager::selectWorkWeek()
{
    showAgendaView();
    if (KOGlobals::self()->getWorkWeekMask() != 0) {
        mRangeMode = WORK_WEEK_RANGE;
        QDate const date = mMainView->activeDate();
        mMainView->dateNavigator()->selectWorkWeek(date);
    } else {
        KMessageBox::error(mMainView,
                           i18n("Unable to display the work week since there are no work days configured. "
                                "Please properly configure at least 1 work day in the Time and Date preferences."));
    }
    viewActionEnable(viewToAction(QStringLiteral("Agenda"), mRangeMode));
}

void KOViewManager::selectWeek()
{
    showAgendaView();
    mRangeMode = WEEK_RANGE;
    viewActionEnable(viewToAction(QStringLiteral("Agenda"), mRangeMode));
    QDate const date = mMainView->activeDate();
    mMainView->dateNavigator()->selectWeek(date);
}

void KOViewManager::selectNextX()
{
    showAgendaView();
    mRangeMode = NEXTX_RANGE;
    viewActionEnable(viewToAction(QStringLiteral("Agenda"), mRangeMode));
    mMainView->dateNavigator()->selectDates(QDate::currentDate(), KOPrefs::instance()->mNextXDays);
}

void KOViewManager::showTodoView()
{
    if (!mTodoView) {
        mTodoView = new KOTodoView(false /*not sidebar*/, mMainView->viewStack());
        mTodoView->setIdentifier("DefaultTodoView");
        addView(mTodoView);
        connectTodoView(mTodoView);

        KSharedConfig::Ptr const config = KSharedConfig::openConfig();
        mTodoView->restoreLayout(config.data(), QStringLiteral("Todo View"), false);
    }
    viewActionEnable(viewToAction(QStringLiteral("Todo"), NO_RANGE));
    goMenu(false);
    showView(mTodoView);
}

void KOViewManager::showJournalView()
{
    if (!mJournalView) {
        mJournalView = new KOJournalView(mMainView->viewStack());
        mJournalView->setIdentifier("DefaultJournalView");
        addView(mJournalView);
    }
    viewActionEnable(viewToAction(QStringLiteral("Journal"), NO_RANGE));
    goMenu(true);
    showView(mJournalView);
}

void KOViewManager::showTimeLineView()
{
    if (!mTimelineView) {
        mTimelineView = new KOTimelineView(mMainView->viewStack());
        mTimelineView->setIdentifier("DefaultTimelineView");
        addView(mTimelineView);
    }
    viewActionEnable(viewToAction(QStringLiteral("Timeline"), NO_RANGE));
    goMenu(true);
    showView(mTimelineView);
}

void KOViewManager::showEventView()
{
    if (mLastEventView) {
        goMenu(true);
        showView(mLastEventView);
    } else {
        showAgendaView();
        selectWeek();
    }
}

Akonadi::Item KOViewManager::currentSelection()
{
    if (!mCurrentView) {
        return {};
    }

    Akonadi::Item::List incidenceList = mCurrentView->selectedIncidences();
    if (incidenceList.isEmpty()) {
        return {};
    }
    return incidenceList.first();
}

QDate KOViewManager::currentSelectionDate()
{
    QDate qd;
    if (mCurrentView) {
        KCalendarCore::DateList qvl = mCurrentView->selectedIncidenceDates();
        if (!qvl.isEmpty()) {
            qd = qvl.first();
        }
    }
    return qd;
}

void KOViewManager::setDocumentId(const QString &id)
{
    if (mTodoView) {
        mTodoView->setDocumentId(id);
    }
}

QWidget *KOViewManager::widgetForView(KOrg::BaseView *view) const
{
    if (mAgendaViewTabs && mAgendaViewTabs->indexOf(view) >= 0) {
        return mAgendaViewTabs;
    }
    return view;
}

void KOViewManager::currentAgendaViewTabChanged(int index)
{
    KSharedConfig::Ptr const config = KSharedConfig::openConfig();
    KConfigGroup viewConfig(config, QStringLiteral("Views"));
    viewConfig.writeEntry("Agenda View Tab Index", mAgendaViewTabs->currentIndex());

    if (index > -1) {
        viewActionEnable(sender());
        goMenu(true);
        QWidget *widget = mAgendaViewTabs->widget(index);
        if (widget) {
            showView(static_cast<KOrg::BaseView *>(widget));
        }
    }
}

void KOViewManager::addChange(EventViews::EventView::Change change)
{
    for (BaseView *view : std::as_const(mViews)) {
        if (view) {
            view->setChanges(view->changes() | change);
        }
    }
}

void KOViewManager::updateMultiCalendarDisplay()
{
    if (agendaIsSelected()) {
        showAgendaView();
    } else {
        updateView();
    }
}

bool KOViewManager::agendaIsSelected() const
{
    return mCurrentView == mAgendaView || mCurrentView == mAgendaSideBySideView || (mAgendaViewTabs && mCurrentView == mAgendaViewTabs->currentWidget());
}

void KOViewManager::addCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mCalendars.push_back(calendar);
}

void KOViewManager::removeCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mCalendars.removeAll(calendar);
}

#include "moc_koviewmanager.cpp"
