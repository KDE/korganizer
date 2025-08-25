/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997, 1998, 1999 Preston Brown <preston.brown@yale.edu>
  SPDX-FileCopyrightText: Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  SPDX-FileCopyrightText: Ian Dawes <iadawes@globalserve.net>
  SPDX-FileCopyrightText: Laszlo Boloni <boloni@cs.purdue.edu>

  SPDX-FileCopyrightText: 2000-2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2004-2025 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "calendarview.h"

#include "akonadicollectionview.h"
#include "collectiongeneralpage.h"
#include "datechecker.h"
#include "datenavigator.h"
#include "datenavigatorcontainer.h"
#include "dialog/koeventviewerdialog.h"
#include "kodaymatrix.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "koviewmanager.h"
#include "pimmessagebox.h"
#include "prefs/koprefs.h"
#include "views/agendaview/koagendaview.h"
#include "views/monthview/monthview.h"
#include "views/todoview/kotodoview.h"
#include "whatsnew/whatsnewtranslations.h"
#include "widgets/navigatorbar.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/CalFilterPartStatusProxyModel>
#include <Akonadi/CalendarClipboard>
#include <Akonadi/CalendarUtils>
#include <Akonadi/CollectionIdentificationAttribute>
#include <Akonadi/CollectionMaintenancePage>
#include <Akonadi/CollectionPropertiesDialog>
#include <Akonadi/ControlGui>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/FreeBusyManager>
#include <Akonadi/IncidenceChanger>
#include <Akonadi/TodoPurger>
#include <akonadi/calendarsettings.h>

#include <CalendarSupport/CalPrinter>
#include <CalendarSupport/CalendarSingleton>
#include <CalendarSupport/IncidenceViewer>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <IncidenceEditor/IncidenceDefaults>
#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>
#include <IncidenceEditor/IncidenceEditorSettings>
#include <IncidenceEditor/IndividualMailComponentFactory>

#include <KCalendarCore/CalFilter>
#include <KCalendarCore/FileStorage>
#include <KCalendarCore/ICalFormat>

#include <KCalUtils/DndFactory>
#include <KCalUtils/Stringify>

#include <PimCommon/WhatsNewDialog>

#include <PimCommonAkonadi/CollectionAclPage>
#include <PimCommonAkonadi/ImapAclAttribute>

#include <KDatePicker>
#include <KDialogJobUiDelegate>
#include <KIO/CommandLauncherJob>
#include <KMessageBox>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

// Meaningful aliases for dialog box return codes.
enum ItemActions {
    Cancel = KMessageBox::Cancel, // Do nothing.
    Current = KMessageBox::Ok, // Selected recurrence only.
    AlsoFuture = KMessageBox::ButtonCode::SecondaryAction, // Selected and future recurrences.
    Parent = KMessageBox::ButtonCode::PrimaryAction, // Instance, but not child instances.
    All = KMessageBox::Continue, // Instance and child instances.
};

CalendarView::CalendarView(QWidget *parent)
    : CalendarViewBase(parent)
{
    Akonadi::ControlGui::widgetNeedsAkonadi(this);
    mChanger = new Akonadi::IncidenceChanger(new IncidenceEditorNG::IndividualMailComponentFactory(this), this);
    mChanger->setDefaultCollection(Akonadi::Collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId()));

    mChanger->setDestinationPolicy(static_cast<Akonadi::IncidenceChanger::DestinationPolicy>(KOPrefs ::instance()->destination()));

    // We reuse the EntityTreeModel from the calendar singleton to save memory.
    // We don't reuse the entire ETMCalendar because we want a different selection model. Checking/unchecking
    // calendars in korganizer shouldn't affect kontact's summary view
    mCalendar = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(CalendarSupport::calendarSingleton().data()));

    connect(mCalendar->entityTreeModel(), &Akonadi::EntityTreeModel::errorOccurred, this, [this](const QString &message) {
        KMessageBox::error(this, message);
    });

    mCalendar->setObjectName(QLatin1StringView("KOrg Calendar"));
    mCalendarClipboard = new Akonadi::CalendarClipboard(mCalendar, mChanger, this);
    mITIPHandler = new Akonadi::ITIPHandler(this);
    mITIPHandler->setCalendar(mCalendar);
    connect(mCalendarClipboard, &Akonadi::CalendarClipboard::cutFinished, this, &CalendarView::onCutFinished);

    mChanger->setEntityTreeModel(mCalendar->entityTreeModel());

    mPartStatFilterProxy = new Akonadi::CalFilterPartStatusProxyModel(this);
    if (Akonadi::CalendarSettings::self()->hideDeclinedInvitations()) {
        mPartStatFilterProxy->setBlockedStatusList({KCalendarCore::Attendee::Declined});
    }
    mPartStatFilterProxy->setSourceModel(mCalendar->entityTreeModel());

    Akonadi::AttributeFactory::registerAttribute<PimCommon::ImapAclAttribute>();
    Akonadi::AttributeFactory::registerAttribute<Akonadi::CollectionIdentificationAttribute>();

    mViewManager = new KOViewManager(this);
    mDialogManager = new KODialogManager(this);
    mTodoPurger = new Akonadi::TodoPurger(this);
    mTodoPurger->setCalendar(mCalendar);
    mTodoPurger->setIncidenceChager(mChanger);
    connect(mTodoPurger, &Akonadi::TodoPurger::todosPurged, this, &CalendarView::onTodosPurged);

    mReadOnly = false;
    mSplitterSizesValid = false;

    mCalPrinter = nullptr;

    mDateNavigator = new DateNavigator(this);
    mDateChecker = new DateChecker(this);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins({});

    // create the main layout frames.
    mPanner = new QSplitter(Qt::Horizontal, this);
    mPanner->setObjectName(QLatin1StringView("CalendarView::Panner"));
    topLayout->addWidget(mPanner);

    mLeftSplitter = new QSplitter(Qt::Vertical, mPanner);
    mLeftSplitter->setObjectName(QLatin1StringView("CalendarView::LeftFrame"));
    // The GUI checkboxes of "show widget XYZ" are confusing when the QSplitter
    // hides the widget magically. I know I blamed Akonadi for not showing my
    // calendar more than once.
    mLeftSplitter->setChildrenCollapsible(false);

    mDateNavigatorContainer = new DateNavigatorContainer(mLeftSplitter);
    mDateNavigatorContainer->setObjectName(QLatin1StringView("CalendarView::DateNavigator"));

    mTodoList = new KOTodoView(true /*sidebar*/, mLeftSplitter);
    mTodoList->setObjectName(QLatin1StringView("todolist"));
    connect(this, &CalendarView::calendarAdded, mTodoList, &KOTodoView::calendarAdded);
    connect(this, &CalendarView::calendarRemoved, mTodoList, &KOTodoView::calendarRemoved);

    mEventViewerBox = new QWidget(mLeftSplitter);
    auto mEventViewerBoxVBoxLayout = new QVBoxLayout(mEventViewerBox);
    mEventViewerBoxVBoxLayout->setContentsMargins({});
    mEventViewerBoxVBoxLayout->setContentsMargins({});
    mEventViewer = new CalendarSupport::IncidenceViewer(mCalendar->entityTreeModel(), mEventViewerBox);
    mEventViewer->setObjectName(QLatin1StringView("EventViewer"));
    mEventViewerBoxVBoxLayout->addWidget(mEventViewer);

    auto rightBox = new QWidget(mPanner);
    auto rightBoxVBoxLayout = new QVBoxLayout(rightBox);
    rightBoxVBoxLayout->setContentsMargins({});
    mNavigatorBar = new NavigatorBar(rightBox);
    rightBoxVBoxLayout->addWidget(mNavigatorBar);
    mRightFrame = new QStackedWidget(rightBox);
    rightBoxVBoxLayout->addWidget(mRightFrame);
    mMessageWidget = new CalendarSupport::MessageWidget(rightBox);
    rightBoxVBoxLayout->addWidget(mMessageWidget);

    rightBoxVBoxLayout->setStretchFactor(mRightFrame, 1);

    mLeftFrame = mLeftSplitter;
    mLeftFrame->installEventFilter(this);

    mChanger->setGroupwareCommunication(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication());
    connect(mChanger, &Akonadi::IncidenceChanger::createFinished, this, &CalendarView::slotCreateFinished);

    connect(mChanger, &Akonadi::IncidenceChanger::deleteFinished, this, &CalendarView::slotDeleteFinished);

    connect(mChanger, &Akonadi::IncidenceChanger::modifyFinished, this, &CalendarView::slotModifyFinished);

    // Signals emitted by mDateNavigator
    connect(mDateNavigator, &DateNavigator::datesSelected, this, &CalendarView::showDates);

    connect(mDateNavigatorContainer, &DateNavigatorContainer::newEventSignal, this, [this](const QDate &s) {
        newEvent(s);
    });

    connect(mDateNavigatorContainer, &DateNavigatorContainer::newTodoSignal, this, qOverload<const QDate &>(&CalendarView::newTodo));
    connect(mDateNavigatorContainer, &DateNavigatorContainer::newJournalSignal, this, qOverload<const QDate &>(&CalendarView::newJournal));

    // Signals emitted by mNavigatorBar
    connect(mNavigatorBar, &NavigatorBar::prevYearClicked, mDateNavigator, &DateNavigator::selectPreviousYear);
    connect(mNavigatorBar, &NavigatorBar::nextYearClicked, mDateNavigator, &DateNavigator::selectNextYear);
    connect(mNavigatorBar, &NavigatorBar::prevMonthClicked, this, [this]() {
        mDateNavigator->selectPreviousMonth();
    });
    connect(mNavigatorBar, &NavigatorBar::nextMonthClicked, this, [this]() {
        mDateNavigator->selectNextMonth();
    });
    connect(mNavigatorBar, &NavigatorBar::monthSelected, mDateNavigator, &DateNavigator::selectMonth);
    connect(mNavigatorBar, &NavigatorBar::yearSelected, mDateNavigator, &DateNavigator::selectYear);

    // Signals emitted by mDateNavigatorContainer
    connect(mDateNavigatorContainer, &DateNavigatorContainer::weekClicked, this, &CalendarView::selectWeek);

    connect(mDateNavigatorContainer, &DateNavigatorContainer::prevMonthClicked, mDateNavigator, &DateNavigator::selectPreviousMonth);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::nextMonthClicked, mDateNavigator, &DateNavigator::selectNextMonth);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::prevYearClicked, mDateNavigator, &DateNavigator::selectPreviousYear);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::nextYearClicked, mDateNavigator, &DateNavigator::selectNextYear);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::monthSelected, mDateNavigator, &DateNavigator::selectMonth);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::yearSelected, mDateNavigator, &DateNavigator::selectYear);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::goPrevious, mDateNavigator, &DateNavigator::selectPrevious);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::goNext, mDateNavigator, &DateNavigator::selectNext);

    connect(mDateNavigatorContainer,
            &DateNavigatorContainer::datesSelected,
            mDateNavigator,
            qOverload<const KCalendarCore::DateList &, QDate>(&DateNavigator::selectDates));

    connect(mViewManager, &KOViewManager::datesSelected, mDateNavigator, [this](const KCalendarCore::DateList &dates) {
        mDateNavigator->selectDates(dates);
        agendaViewMenuSelector(dates);
    });

    connect(mDateNavigatorContainer, &DateNavigatorContainer::incidenceDropped, this, &CalendarView::addIncidenceOn);
    connect(mDateNavigatorContainer, &DateNavigatorContainer::incidenceDroppedMove, this, &CalendarView::moveIncidenceTo);

    connect(mDateChecker, &DateChecker::dayPassed, mTodoList, &BaseView::dayPassed);
    connect(mDateChecker, &DateChecker::dayPassed, this, &CalendarView::dayPassed);
    connect(mDateChecker, &DateChecker::dayPassed, mDateNavigatorContainer, &DateNavigatorContainer::updateToday);

    connect(this, &CalendarView::configChanged, mDateNavigatorContainer, &DateNavigatorContainer::updateConfig);

    connect(this, &CalendarView::incidenceSelected, mEventViewer, &CalendarSupport::IncidenceViewer::setIncidence);

    // TODO: do a pretty Summary,
    const QString s = xi18nc("@info",
                             "<para><emphasis>No Item Selected</emphasis></para>"
                             "<para>Select an event, to-do or journal entry to view its details here.</para>");

    mEventViewer->setDefaultMessage(s);
    mEventViewer->setWhatsThis(i18nc("@info:whatsthis", "View the details of events, journal entries or to-dos selected in KOrganizer's main view here."));
    mEventViewer->setIncidence(Akonadi::Item(), QDate());

    mViewManager->connectTodoView(mTodoList);
    mViewManager->connectView(mTodoList);

    KOGlobals::self()->setHolidays(CalendarSupport::KCalPrefs::instance()->mHolidays);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &CalendarView::checkClipboard);

    connect(mTodoList, &BaseView::incidenceSelected, this, &CalendarView::processTodoListSelection);
    disconnect(mTodoList, &BaseView::incidenceSelected, this, &CalendarView::processMainViewSelection);

    {
        static bool pageRegistered = false;

        if (!pageRegistered) {
            Akonadi::CollectionPropertiesDialog::registerPage(new CalendarSupport::CollectionGeneralPageFactory);
            Akonadi::CollectionPropertiesDialog::registerPage(new PimCommon::CollectionAclPageFactory);
            Akonadi::CollectionPropertiesDialog::registerPage(new Akonadi::CollectionMaintenancePageFactory);
            pageRegistered = true;
        }
    }

    Akonadi::FreeBusyManager::self()->setCalendar(mCalendar);

    mCalendar->registerObserver(this);
}

CalendarView::~CalendarView()
{
    mCalendar->unregisterObserver(this);
    mCalendar->setFilter(nullptr); // So calendar doesn't deleted it twice
    forEachCalendar([](const auto &calendar) {
        calendar->setFilter(nullptr);
    });
    qDeleteAll(mFilters);
    qDeleteAll(mExtensions);

    delete mDialogManager;
    delete mViewManager;
    delete mEventViewer;
}

QAbstractItemModel *CalendarView::eventsModel() const
{
    return mPartStatFilterProxy;
}

Akonadi::ETMCalendar::Ptr CalendarView::calendar() const
{
    return mCalendar;
}

QList<Akonadi::CollectionCalendar::Ptr> &CalendarView::enabledCalendars()
{
    return mEnabledCalendars;
}

Akonadi::CollectionCalendar::Ptr CalendarView::calendarForCollection(const Akonadi::Collection &collection)
{
    auto it = mCalendars.begin();
    while (it != mCalendars.end()) {
        if (it->isNull()) {
            it = mCalendars.erase(it);
            continue;
        }

        const auto calendarIt = it->toStrongRef();
        if (calendarIt->collection() == collection) {
            return calendarIt;
        }

        ++it;
    }

    const auto newCalendar = Akonadi::CollectionCalendar::Ptr::create(eventsModel(), collection);
    newCalendar->setFilter(mCurrentFilter);
    mCalendars.emplace_back(newCalendar);
    return newCalendar;
}

// NOLINTBEGIN(performance-unnecessary-value-param)
void CalendarView::forEachCalendar(std::function<void(Akonadi::CollectionCalendar::Ptr)> func)
{
    auto it = mCalendars.begin();
    while (it != mCalendars.end()) {
        auto collection = it->lock();
        if (!collection) {
            it = mCalendars.erase(it);
            continue;
        }

        func(collection);
        ++it;
    }
}
// NOLINTEND(performance-unnecessary-value-param)

QDate CalendarView::activeDate(bool fallbackToToday)
{
    KOrg::BaseView *curView = mViewManager->currentView();
    if (curView) {
        if (curView->selectionStart().isValid()) {
            return curView->selectionStart().date();
        }

        // Try the view's selectedIncidenceDates()
        if (!curView->selectedIncidenceDates().isEmpty()) {
            if (curView->selectedIncidenceDates().constFirst().isValid()) {
                return curView->selectedIncidenceDates().constFirst();
            }
        }
    }

    // When all else fails, use the navigator start date, or today.
    if (fallbackToToday) {
        return QDate::currentDate();
    } else {
        return mDateNavigator->selectedDates().constFirst();
    }
}

QDate CalendarView::activeIncidenceDate()
{
    KOrg::BaseView *curView = mViewManager->currentView();
    if (curView) {
        KCalendarCore::DateList dates = curView->selectedIncidenceDates();
        if (!dates.isEmpty()) {
            return dates.first();
        }
    }

    return {};
}

QDate CalendarView::startDate()
{
    KCalendarCore::DateList dates = mDateNavigator->selectedDates();
    return dates.first();
}

QDate CalendarView::endDate()
{
    KCalendarCore::DateList dates = mDateNavigator->selectedDates();
    return dates.last();
}

void CalendarView::createPrinter()
{
    if (!mCalPrinter) {
        mCalPrinter = new CalendarSupport::CalPrinter(this, mCalendar);
        connect(this, &CalendarView::configChanged, mCalPrinter, &CalendarSupport::CalPrinter::updateConfig);
    }
}

bool CalendarView::saveCalendar(const QString &filename)
{
    // Store back all unsaved data into calendar object
    mViewManager->currentView()->flushView();

    KCalendarCore::FileStorage storage(mCalendar);
    storage.setFileName(filename);
    storage.setSaveFormat(new KCalendarCore::ICalFormat);

    return storage.save();
}

void CalendarView::archiveCalendar()
{
    mDialogManager->showArchiveDialog();
}

void CalendarView::readSettings()
{
    // read settings from the KConfig, supplying reasonable
    // defaults where none are to be found

    KSharedConfig::Ptr const config = KSharedConfig::openConfig();
    KConfigGroup const geometryConfig(config, QStringLiteral("KOrganizer Geometry"));

    QList<int> sizes = geometryConfig.readEntry("Separator1", QList<int>());
    /* cppcheck-suppress knownConditionTrueFalse */
    if (sizes.count() != 2 || sizes.count() == sizes.count(0)) {
        sizes << mDateNavigatorContainer->minimumSizeHint().width();
        sizes << 300;
    }
    mPanner->setSizes(sizes);

    sizes = geometryConfig.readEntry("Separator2", QList<int>());
    if (!sizes.isEmpty() && sizes.count() != sizes.count(0)) {
        mLeftSplitter->setSizes(sizes);
    }

    mViewManager->readSettings(config.data());
    mTodoList->restoreLayout(config.data(), QStringLiteral("Sidebar Todo View"), true);

    readFilterSettings(config.data());

    KConfigGroup const viewConfig(config, QStringLiteral("Views"));
    int const dateCount = viewConfig.readEntry("ShownDatesCount", 7);
    if (dateCount == 7) {
        mDateNavigator->selectWeek();
    } else {
        const KCalendarCore::DateList dates = mDateNavigator->selectedDates();
        if (!dates.isEmpty()) {
            mDateNavigator->selectDates(dates.first(), dateCount);
        }
    }
}

void CalendarView::writeSettings()
{
    auto config = KSharedConfig::openConfig();
    KConfigGroup geometryConfig(config, QStringLiteral("KOrganizer Geometry"));

    QList<int> list = mMainSplitterSizes.isEmpty() ? mPanner->sizes() : mMainSplitterSizes;
    // splitter sizes are invalid (all zero) unless we have been shown once
    if (list.count() != list.count(0) && mSplitterSizesValid) {
        geometryConfig.writeEntry("Separator1", list);
    }

    list = mLeftSplitter->sizes();
    if (list.count() != list.count(0) && mSplitterSizesValid) {
        geometryConfig.writeEntry("Separator2", list);
    }

    mViewManager->writeSettings(config.data());
    mTodoList->saveLayout(config.data(), QStringLiteral("Sidebar Todo View"));

    Akonadi::CalendarSettings::self()->save();
    KOPrefs::instance()->save();
    CalendarSupport::KCalPrefs::instance()->save();

    writeFilterSettings(config.data());

    KConfigGroup viewConfig(config, QStringLiteral("Views"));
    viewConfig.writeEntry("ShownDatesCount", mDateNavigator->selectedDates().count());

    config->sync();
}

void CalendarView::readFilterSettings(KConfig *config)
{
    qDeleteAll(mFilters);
    mFilters.clear();

    KConfigGroup const generalConfig(config, QStringLiteral("General"));
    // FIXME: Move the filter loading and saving to the CalFilter class in libkcal
    QStringList const filterList = generalConfig.readEntry("CalendarFilters", QStringList());
    QString const currentFilter = generalConfig.readEntry("Current Filter");

    QStringList::ConstIterator it = filterList.constBegin();
    QStringList::ConstIterator const end = filterList.constEnd();
    while (it != end) {
        auto filter = new KCalendarCore::CalFilter(*it);
        KConfigGroup const filterConfig(config, QStringLiteral("Filter_") + (*it));
        filter->setCriteria(filterConfig.readEntry("Criteria", 0));
        filter->setCategoryList(filterConfig.readEntry("CategoryList", QStringList()));
        if (filter->criteria() & KCalendarCore::CalFilter::HideNoMatchingAttendeeTodos) {
            filter->setEmailList(CalendarSupport::KCalPrefs::instance()->allEmails());
        }
        filter->setCompletedTimeSpan(filterConfig.readEntry("HideTodoDays", 0));
        mFilters.append(filter);

        ++it;
    }

    int const pos = filterList.indexOf(currentFilter);
    mCurrentFilter = nullptr;
    if (pos >= 0) {
        mCurrentFilter = mFilters.at(pos);
    }
    updateFilter();
}

void CalendarView::writeFilterSettings(KConfig *config)
{
    static QRegularExpression const re(QStringLiteral("^Filter_.*"));
    QStringList filterList;

    const QStringList oldFilterList = config->groupList().filter(re);
    // Delete Old Group
    for (const QString &conf : oldFilterList) {
        KConfigGroup group = config->group(conf);
        group.deleteGroup(QLatin1StringView());
    }

    filterList.reserve(mFilters.count());
    for (KCalendarCore::CalFilter *filter : std::as_const(mFilters)) {
        filterList << filter->name();
        KConfigGroup filterConfig(config, QStringLiteral("Filter_") + filter->name());
        filterConfig.writeEntry("Criteria", filter->criteria());
        filterConfig.writeEntry("CategoryList", filter->categoryList());
        filterConfig.writeEntry("HideTodoDays", filter->completedTimeSpan());
    }
    KConfigGroup generalConfig(config, QStringLiteral("General"));
    generalConfig.writeEntry("CalendarFilters", filterList);
    if (mCurrentFilter) {
        generalConfig.writeEntry("Current Filter", mCurrentFilter->name());
    } else {
        generalConfig.writeEntry("Current Filter", QString());
    }
}

void CalendarView::goDate(QDate date)
{
    mDateNavigator->selectDate(date);
}

void CalendarView::showDate(QDate date)
{
    int const dateCount = mDateNavigator->datesCount();
    if (dateCount == 7) {
        mDateNavigator->selectWeek(date);
    } else {
        mDateNavigator->selectDates(date, dateCount);
    }
}

void CalendarView::goToday()
{
    mDateNavigator->selectToday();
}

void CalendarView::goSelectADate()
{
    auto datePicker = new KDatePicker(); // must be parent-less
    datePicker->setAttribute(Qt::WA_DeleteOnClose, true);
    datePicker->show();
    QObject::connect(datePicker, &KDatePicker::dateChanged, this, [this](QDate date) {
        mDateNavigator->selectADate(date);
    });
}

void CalendarView::goNext()
{
    if (qobject_cast<MonthView *>(mViewManager->currentView())) {
        const QDate month = mDateNavigatorContainer->monthOfNavigator(0);
        QPair<QDate, QDate> const limits = KODayMatrix::matrixLimits(month);
        mDateNavigator->selectNextMonth(month, limits.first, limits.second);
    } else {
        mDateNavigator->selectNext();
    }
}

void CalendarView::goPrevious()
{
    if (qobject_cast<MonthView *>(mViewManager->currentView())) {
        const QDate month = mDateNavigatorContainer->monthOfNavigator(0);
        QPair<QDate, QDate> const limits = KODayMatrix::matrixLimits(month);
        mDateNavigator->selectPreviousMonth(month, limits.first, limits.second);
    } else {
        mDateNavigator->selectPrevious();
    }
}

void CalendarView::updateConfig()
{
    if (mCalPrinter) {
        mCalPrinter->deleteLater();
        mCalPrinter = nullptr;
    }

    KOGlobals::self()->setHolidays(CalendarSupport::KCalPrefs::instance()->mHolidays);

    // config changed lets tell the date navigator the new modes
    // if there weren't changed they are ignored
    updateHighlightModes();

    Q_EMIT configChanged();

    // switch between merged, side by side and tabbed agenda if needed
    mViewManager->updateMultiCalendarDisplay();

    // To make the "fill window" configurations work
    mViewManager->raiseCurrentView();

    mChanger->setDestinationPolicy(static_cast<Akonadi::IncidenceChanger::DestinationPolicy>(KOPrefs::instance()->destination()));

    mChanger->setGroupwareCommunication(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication());

    if (Akonadi::CalendarSettings::self()->hideDeclinedInvitations()) {
        mPartStatFilterProxy->setBlockedStatusList({KCalendarCore::Attendee::Declined});
    } else {
        mPartStatFilterProxy->setBlockedStatusList({});
    }
}

void CalendarView::slotCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    Q_UNUSED(changeId)
    if (resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess) {
        changeIncidenceDisplay(item, Akonadi::IncidenceChanger::ChangeTypeCreate);
        updateUnmanagedViews();
        checkForFilteredChange(item);
    } else if (!errorString.isEmpty()) {
        qCCritical(KORGANIZER_LOG) << "Incidence not added, job reported error: " << errorString;
    }
}

void CalendarView::slotModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    Q_UNUSED(changeId)
    if (resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess) {
        qCCritical(KORGANIZER_LOG) << "Incidence not modified, job reported error: " << errorString;
        return;
    }

    Q_ASSERT(item.isValid());
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    Q_ASSERT(incidence);
    QSet<KCalendarCore::IncidenceBase::Field> const dirtyFields = incidence->dirtyFields();
    incidence->resetDirtyFields();
    // Record completed todos in journals, if enabled. we should to this here in
    // favor of the todolist. users can mark a task as completed in an editor
    // as well.
    if (incidence->type() == KCalendarCore::Incidence::TypeTodo && KOPrefs::instance()->recordTodosInJournals()
        && (dirtyFields.contains(KCalendarCore::Incidence::FieldCompleted))) {
        KCalendarCore::Todo::Ptr const todo = incidence.dynamicCast<KCalendarCore::Todo>();
        if (todo->isCompleted() || todo->recurs()) {
            QString const timeStr = QLocale::system().toString(QTime::currentTime(), QLocale::ShortFormat);
            QString description = i18nc("@info/plain", "Todo completed: %1 (%2)", incidence->summary(), timeStr);

            KCalendarCore::Journal::List journals = calendar()->journals(QDate::currentDate());

            if (journals.isEmpty()) {
                KCalendarCore::Journal::Ptr const journal(new KCalendarCore::Journal);
                journal->setDtStart(QDateTime::currentDateTime());

                QString const dateStr = QLocale::system().toString(QDate::currentDate(), QLocale::LongFormat);
                journal->setSummary(i18nc("@info/plain", "Journal of %1", dateStr));
                journal->setDescription(description);

                if (mChanger->createIncidence(journal, item.parentCollection(), this) == -1) {
                    qCCritical(KORGANIZER_LOG) << "Unable to add Journal";
                    return;
                }
            } else { // journal list is not empty
                Akonadi::Item const journalItem = mCalendar->item(journals.first()->uid());
                KCalendarCore::Journal::Ptr const journal = Akonadi::CalendarUtils::journal(journalItem);
                KCalendarCore::Journal::Ptr const oldJournal(journal->clone());
                journal->setDescription(journal->description().append(u'\n' + description));
                (void)mChanger->modifyIncidence(journalItem, oldJournal, this);
            }
        }
    }

    changeIncidenceDisplay(item, Akonadi::IncidenceChanger::ChangeTypeCreate);
    updateUnmanagedViews();
    checkForFilteredChange(item);
}

void CalendarView::slotDeleteFinished(int changeId,
                                      const QList<Akonadi::Item::Id> &itemIdList,
                                      Akonadi::IncidenceChanger::ResultCode resultCode,
                                      const QString &errorString)
{
    Q_UNUSED(changeId)
    if (resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess) {
        for (Akonadi::Item::Id const id : itemIdList) {
            Akonadi::Item const item = mCalendar->item(id);
            if (item.isValid()) {
                changeIncidenceDisplay(item, Akonadi::IncidenceChanger::ChangeTypeDelete);
            }
        }
        updateUnmanagedViews();
    } else {
        qCCritical(KORGANIZER_LOG) << "Incidence not deleted, job reported error: " << errorString;
    }
}

void CalendarView::checkForFilteredChange(const Akonadi::Item &item)
{
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    KCalendarCore::CalFilter *filter = calendar()->filter();
    if (filter && !filter->filterIncidence(incidence)) {
        // Incidence is filtered and thus not shown in the view, tell the
        // user so that he isn't surprised if his new event doesn't show up
        const auto message = xi18nc("@info",
                                    "The item <resource>%1</resource> is filtered by your current filter rules, "
                                    "so it will be hidden and not appear in the view.",
                                    incidence->summary());
        showMessage(message, KMessageWidget::Information);
    }
}

void CalendarView::startMultiModify(const QString &text)
{
    mChanger->startAtomicOperation(text);
}

void CalendarView::endMultiModify()
{
    mChanger->endAtomicOperation();
}

void CalendarView::changeIncidenceDisplay(const Akonadi::Item &item, Akonadi::IncidenceChanger::ChangeType changeType)
{
    if (mDateNavigatorContainer->isVisible()) {
        mDateNavigatorContainer->updateView();
    }

    if (CalendarSupport::hasIncidence(item)) {
        // If there is an event view visible update the display
        mViewManager->currentView()->changeIncidenceDisplay(item, changeType);
    } else {
        mViewManager->currentView()->updateView();
    }
}

void CalendarView::updateView(const QDate &start, const QDate &end, const QDate &preferredMonth, const bool updateTodos)
{
    const bool currentViewIsTodoView = mViewManager->currentView()->identifier() == "DefaultTodoView";

    /* Never show the todolist in the sidebar when in todoview mode */
    if (currentViewIsTodoView) {
        mTodoList->setVisible(false);
    } else {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        const KConfigGroup group = config->group(QStringLiteral("Settings"));
        mTodoList->setVisible(group.readEntry("TodoViewVisible", false));
    }

    if (updateTodos && mTodoList->isVisible()) {
        // Update the sidepane todoView
        mTodoList->updateView();
    }

    if (start.isValid() && end.isValid() && !(currentViewIsTodoView && !updateTodos)) {
        mViewManager->updateView(start, end, preferredMonth);
    }

    if (mDateNavigatorContainer->isVisible()) {
        mDateNavigatorContainer->updateView();
    }
}

void CalendarView::updateView()
{
    const KCalendarCore::DateList tmpList = mDateNavigator->selectedDates();
    const QDate month = mDateNavigatorContainer->monthOfNavigator();

    // We assume that the navigator only selects consecutive days.
    updateView(tmpList.first(), tmpList.last(), month /**preferredMonth*/);
}

void CalendarView::updateUnmanagedViews()
{
    if (mDateNavigatorContainer->isVisible()) {
        mDateNavigatorContainer->updateDayMatrix();
    }
}

int CalendarView::msgItemDelete(const Akonadi::Item &item)
{
    return KMessageBox::warningContinueCancel(
        this,
        i18nc("@info", "Do you really want to permanently remove the item \"%1\"?", Akonadi::CalendarUtils::incidence(item)->summary()),
        i18nc("@title:window", "Delete Item?"),
        KStandardGuiItem::del());
}

void CalendarView::edit_cut()
{
    const Akonadi::Item item = selectedIncidence();
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }

    mCalendarClipboard->cutIncidence(incidence, Akonadi::CalendarClipboard::AskMode);
}

void CalendarView::edit_copy()
{
    const Akonadi::Item item = selectedIncidence();

    if (!item.isValid()) {
        qCCritical(KORGANIZER_LOG) << "Invalid item";
        return;
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    Q_ASSERT(incidence);
    if (!mCalendarClipboard->copyIncidence(incidence, Akonadi::CalendarClipboard::AskMode)) {
        qCCritical(KORGANIZER_LOG) << "Error copying incidence";
    }

    checkClipboard();
}

void CalendarView::edit_paste()
{
    // If in agenda and month view, use the selected time and date from there.
    // In all other cases, use the navigator's selected date.

    QDateTime endDT;
    QDateTime finalDateTime;
    bool useEndTime = false;
    KCalUtils::DndFactory::PasteFlags pasteFlags = {};

    KOrg::BaseView *curView = mViewManager->currentView();
    KOAgendaView *agendaView = mViewManager->agendaView();
    MonthView *monthView = mViewManager->monthView();

    if (!curView) {
        qCWarning(KORGANIZER_LOG) << "No view is selected, can't paste";
        return;
    }

    if (curView == agendaView && agendaView->selectionStart().isValid()) {
        const QDate date = agendaView->selectionStart().date();
        endDT = agendaView->selectionEnd();
        useEndTime = !agendaView->selectedIsSingleCell();
        if (agendaView->selectedIsAllDay()) {
            finalDateTime = QDateTime(date.startOfDay());
        } else {
            finalDateTime = QDateTime(date, agendaView->selectionStart().time());
        }
    } else if (curView == monthView && monthView->selectionStart().isValid()) {
        finalDateTime = QDateTime(monthView->selectionStart().date().startOfDay());
        pasteFlags = KCalUtils::DndFactory::FlagPasteAtOriginalTime;
    } else if (!mDateNavigator->selectedDates().isEmpty() && curView->supportsDateNavigation()) {
        // default to the selected date from the navigator
        const KCalendarCore::DateList dates = mDateNavigator->selectedDates();
        if (!dates.isEmpty()) {
            finalDateTime = QDateTime(dates.first().startOfDay());
            pasteFlags = KCalUtils::DndFactory::FlagPasteAtOriginalTime;
        }
    }

    if (!finalDateTime.isValid() && curView->supportsDateNavigation()) {
        KMessageBox::error(this, i18nc("@info", "Paste failed: unable to determine a valid target date."));
        return;
    }

    KCalUtils::DndFactory factory(mCalendar);

    KCalendarCore::Incidence::List pastedIncidences = factory.pasteIncidences(finalDateTime, pasteFlags);
    KCalendarCore::Incidence::List::Iterator it;

    for (it = pastedIncidences.begin(); it != pastedIncidences.end(); ++it) {
        // FIXME: use a visitor here
        if ((*it)->type() == KCalendarCore::Incidence::TypeEvent) {
            KCalendarCore::Event::Ptr const pastedEvent = (*it).staticCast<KCalendarCore::Event>();
            // only use selected area if event is of the same type (all-day or non-all-day
            // as the current selection is
            if (agendaView && endDT.isValid() && useEndTime) {
                if ((pastedEvent->allDay() && agendaView->selectedIsAllDay()) || (!pastedEvent->allDay() && !agendaView->selectedIsAllDay())) {
                    QDateTime const kdt = endDT.toLocalTime();
                    pastedEvent->setDtEnd(kdt.toTimeZone(pastedEvent->dtEnd().timeZone()));
                }
            }

            pastedEvent->setRelatedTo(QString());
            (void)mChanger->createIncidence(KCalendarCore::Event::Ptr(pastedEvent->clone()), Akonadi::Collection(), this);
        } else if ((*it)->type() == KCalendarCore::Incidence::TypeTodo) {
            KCalendarCore::Todo::Ptr const pastedTodo = (*it).staticCast<KCalendarCore::Todo>();
            Akonadi::Item const _selectedTodoItem = selectedTodo();

            // if we are cutting a hierarchy only the root
            // should be son of _selectedTodo
            KCalendarCore::Todo::Ptr const _selectedTodo = Akonadi::CalendarUtils::todo(_selectedTodoItem);
            if (_selectedTodo && pastedTodo->relatedTo().isEmpty()) {
                pastedTodo->setRelatedTo(_selectedTodo->uid());
            }

            // When pasting multiple incidences, don't ask which collection to use, for each one
            (void)mChanger->createIncidence(KCalendarCore::Todo::Ptr(pastedTodo->clone()), Akonadi::Collection(), this);
        } else if ((*it)->type() == KCalendarCore::Incidence::TypeJournal) {
            // When pasting multiple incidences, don't ask which collection to use, for each one
            (void)mChanger->createIncidence(KCalendarCore::Incidence::Ptr((*it)->clone()), Akonadi::Collection(), this);
        }
    }
}

void CalendarView::edit_options()
{
    mDialogManager->showOptionsDialog();
}

static QTime nextQuarterHour(const QTime &time)
{
    if (time.second() % 900) {
        return time.addSecs(900 - (time.minute() * 60 + time.second()) % 900);
    }
    return time; // roundup not needed
}

static QTime adjustedDefaultStartTime(const QDate &startDt)
{
    QTime prefTime;
    if (CalendarSupport::KCalPrefs::instance()->startTime().isValid()) {
        prefTime = CalendarSupport::KCalPrefs::instance()->startTime().time();
    }

    const QDateTime currentDateTime = QDateTime::currentDateTime();
    if (startDt == currentDateTime.date()) {
        // If today and the current time is already past the default time
        // use the next quarter hour after the current time.
        // but don't spill over into tomorrow.
        const QTime currentTime = currentDateTime.time();
        if (!prefTime.isValid() || (currentTime > prefTime && currentTime < QTime(23, 45))) {
            prefTime = nextQuarterHour(currentTime);
        }
    }
    return prefTime;
}

void CalendarView::dateTimesForNewEvent(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    mViewManager->currentView()->eventDurationHint(startDt, endDt, allDay);

    if (!startDt.isValid() || !endDt.isValid()) {
        startDt.setDate(activeDate(true));
        startDt.setTime(adjustedDefaultStartTime(startDt.date()));

        int const addSecs = (CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().hour() * 3600)
            + (CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().minute() * 60);
        endDt = startDt.addSecs(addSecs);
    }
}

IncidenceEditorNG::IncidenceDialog *CalendarView::incidenceDialog(const Akonadi::Item &item)
{
    IncidenceEditorNG::IncidenceDialog *dialog = mDialogManager->createDialog(item);
    connect(dialog, &IncidenceEditorNG::IncidenceDialog::incidenceCreated, this, &CalendarView::handleIncidenceCreated);
    return dialog;
}

IncidenceEditorNG::IncidenceDialog *CalendarView::newEventEditor(const KCalendarCore::Event::Ptr &event)
{
    Akonadi::Item item;
    item.setPayload(event);
    IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog(item);

    dialog->load(item);

    mDialogManager->connectTypeAhead(dialog, qobject_cast<KOEventView *>(viewManager()->currentView()));

    return dialog;
}

void CalendarView::newEvent()
{
    newEvent(QDateTime(), QDateTime());
}

void CalendarView::newEvent(const QDate &dt)
{
    QDateTime const startDt(dt, adjustedDefaultStartTime(dt));
    QTime const duration = CalendarSupport::KCalPrefs::instance()->defaultDuration().time();
    QTime time = startDt.time();
    time = time.addSecs(duration.hour() * 3600 + duration.minute() * 60 + duration.second());
    QDateTime endDt(startDt);
    endDt.setTime(time);
    newEvent(startDt, endDt);
}

void CalendarView::newEvent(const QDateTime &startDt)
{
    newEvent(startDt, startDt);
}

void CalendarView::newEvent(const QDateTime &startDtParam, const QDateTime &endDtParam, bool allDay)
{
    // Let the current view change the default start/end datetime
    QDateTime startDt(startDtParam);
    QDateTime endDt(endDtParam);

    // Adjust the start/end date times (i.e. replace invalid values by defaults,
    // and let the view adjust the type.
    dateTimesForNewEvent(startDt, endDt, allDay);

    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
    defaults.setStartDateTime(startDt);
    defaults.setEndDateTime(endDt);

    KCalendarCore::Event::Ptr const event(new KCalendarCore::Event);
    defaults.setDefaults(event);
    event->setAllDay(allDay);

    IncidenceEditorNG::IncidenceDialog *eventEditor = newEventEditor(event);
    Q_ASSERT(eventEditor);

    // Fallsback to the default collection defined in config
    eventEditor->selectCollection(defaultCollection(KCalendarCore::Event::eventMimeType()));
}

void CalendarView::newEvent(const QString &summary,
                            const QString &description,
                            const QStringList &attachments,
                            const QStringList &attendees,
                            const QStringList &attachmentMimetypes,
                            bool inlineAttachment)
{
    Akonadi::Collection const defaultCol = defaultCollection(KCalendarCore::Event::eventMimeType());

    IncidenceEditorNG::IncidenceDialogFactory::createEventEditor(summary,
                                                                 description,
                                                                 attachments,
                                                                 attendees,
                                                                 attachmentMimetypes,
                                                                 QStringList() /* attachment labels */,
                                                                 inlineAttachment,
                                                                 defaultCol,
                                                                 true /* cleanupAttachmentTempFiles */,
                                                                 this /* parent */);
}

void CalendarView::newTodo(const QString &summary,
                           const QString &description,
                           const QStringList &attachments,
                           const QStringList &attendees,
                           const QStringList &attachmentMimetypes,
                           bool inlineAttachment)
{
    Akonadi::Collection const defaultCol = defaultCollection(KCalendarCore::Todo::todoMimeType());

    IncidenceEditorNG::IncidenceDialogFactory::createTodoEditor(summary,
                                                                description,
                                                                attachments,
                                                                attendees,
                                                                attachmentMimetypes,
                                                                QStringList() /* attachment labels */,
                                                                inlineAttachment,
                                                                defaultCol,
                                                                true /* cleanupAttachmentTempFiles */,
                                                                this /* parent */);
}

void CalendarView::newTodo()
{
    newTodo(Akonadi::Collection());
}

void CalendarView::newTodo(const Akonadi::Collection &collection)
{
    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

    bool allDay = true;
    if (mViewManager->currentView()->isEventView()) {
        QDateTime startDt;
        QDateTime endDt;
        dateTimesForNewEvent(startDt, endDt, allDay);

        defaults.setStartDateTime(startDt);
        defaults.setEndDateTime(endDt);
    }

    KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo);
    defaults.setDefaults(todo);
    todo->setAllDay(allDay);

    Akonadi::Item item;
    item.setPayload(todo);

    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item, collection);

    dialog->load(item);
}

void CalendarView::newTodo(const QDate &date)
{
    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
    defaults.setEndDateTime(QDateTime(date, QTime::currentTime()));

    KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo);
    defaults.setDefaults(todo);
    todo->setAllDay(true);

    Akonadi::Item item;
    item.setPayload(todo);

    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item);
    dialog->load(item);
}

void CalendarView::newJournal()
{
    newJournal(QString(), activeDate(true));
}

void CalendarView::newJournal(const QDate &date)
{
    newJournal(QString(), date.isValid() ? date : activeDate(true));
}

void CalendarView::newJournal(const Akonadi::Collection &collection)
{
    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

    if (mViewManager->currentView()->isEventView()) {
        QDateTime startDt;
        QDateTime endDt;
        bool allDay = true;
        dateTimesForNewEvent(startDt, endDt, allDay);

        defaults.setStartDateTime(startDt);
        defaults.setEndDateTime(endDt);
    }

    KCalendarCore::Journal::Ptr const journal(new KCalendarCore::Journal);
    defaults.setDefaults(journal);

    Akonadi::Item item;
    item.setPayload(journal);
    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item, collection);
    dialog->load(item);
}

void CalendarView::newJournal(const QString &text, QDate date)
{
    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

    KCalendarCore::Journal::Ptr const journal(new KCalendarCore::Journal);
    defaults.setStartDateTime(QDateTime(date.startOfDay()));
    defaults.setDefaults(journal);

    journal->setSummary(text);

    Akonadi::Item item;
    item.setPayload(journal);

    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item);
    dialog->load(item);
}

KOrg::BaseView *CalendarView::currentView() const
{
    return mViewManager->currentView();
}

void CalendarView::configureCurrentView()
{
    KOrg::BaseView *const view = currentView();
    if (view && view->hasConfigurationDialog()) {
        view->showConfigurationDialog(this);
    }
}

void CalendarView::newSubTodo()
{
    const Akonadi::Item item = selectedTodo();
    if (CalendarSupport::hasTodo(item)) {
        newSubTodo(item);
    }
}

void CalendarView::newSubTodo(const Akonadi::Collection &collection)
{
    if (!CalendarSupport::hasTodo(selectedTodo())) {
        qCWarning(KORGANIZER_LOG) << "CalendarSupport::hasTodo() is false";
        return;
    }

    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
    defaults.setRelatedIncidence(Akonadi::CalendarUtils::incidence(selectedTodo()));

    KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo);
    defaults.setDefaults(todo);

    Akonadi::Item item;
    item.setPayload(todo);

    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item, collection);
    dialog->load(item);
}

void CalendarView::newSubTodo(const Akonadi::Item &parentTodo)
{
    IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
    defaults.setRelatedIncidence(Akonadi::CalendarUtils::incidence(parentTodo));

    KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo);
    defaults.setDefaults(todo);

    Q_ASSERT(!todo->relatedTo().isEmpty());

    Akonadi::Item item;
    item.setPayload(todo);

    // Don't use parentTodo.parentCollection() because that can be a search folder.
    Akonadi::Collection const collection = mCalendar->collection(parentTodo.storageCollectionId());
    IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor(item, collection);
    dialog->load(item);
}

bool CalendarView::addIncidence(const QString &ical)
{
    KCalendarCore::ICalFormat format;
    format.setTimeZone(mCalendar->timeZone());
    KCalendarCore::Incidence::Ptr const incidence(format.fromString(ical));
    return addIncidence(incidence);
}

bool CalendarView::addIncidence(const KCalendarCore::Incidence::Ptr &incidence)
{
    return incidence ? mChanger->createIncidence(incidence, Akonadi::Collection(), this) != -1 : false;
}

void CalendarView::appointment_delete()
{
    const Akonadi::Item item = selectedIncidence();
    if (CalendarSupport::hasIncidence(item)) {
        deleteIncidence(item);
    }
}

void CalendarView::todo_unsub()
{
    const Akonadi::Item aTodo = selectedTodo();
    if (incidence_unsub(aTodo)) {
        updateView();
    }
}

bool CalendarView::incidence_unsub(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr inc = Akonadi::CalendarUtils::incidence(item);

    if (!inc || inc->relatedTo().isEmpty()) {
        qCDebug(KORGANIZER_LOG) << "Refusing to unparent this instance" << inc;
        return false;
    }

    const KCalendarCore::Incidence::List instances = mCalendar->instances(inc);
    for (const KCalendarCore::Incidence::Ptr &instance : instances) {
        KCalendarCore::Incidence::Ptr const oldInstance(instance->clone());
        instance->setRelatedTo(QString());
        (void)mChanger->modifyIncidence(mCalendar->item(instance), oldInstance, this);
    }

    KCalendarCore::Incidence::Ptr const oldInc(inc->clone());
    inc->setRelatedTo(QString());
    (void)mChanger->modifyIncidence(item, oldInc, this);

    return true;
}

bool CalendarView::makeSubTodosIndependent()
{
    const Akonadi::Item aTodo = selectedTodo();

    startMultiModify(i18nc("@info/plain", "Make sub-to-dos independent"));
    bool const status = makeChildrenIndependent(aTodo);
    endMultiModify();
    if (status) {
        updateView();
    }
    return status;
}

bool CalendarView::makeChildrenIndependent(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr inc = Akonadi::CalendarUtils::incidence(item);

    const Akonadi::Item::List subIncs = mCalendar->childItems(item.id());

    if (!inc || subIncs.isEmpty()) {
        qCDebug(KORGANIZER_LOG) << "Refusing to  make children independent" << inc;
        return false;
    }

    for (const Akonadi::Item &subInc : subIncs) {
        incidence_unsub(subInc);
    }

    return true;
}

bool CalendarView::deleteIncidence(Akonadi::Item::Id id, bool force)
{
    Akonadi::Item const item = mCalendar->item(id);
    if (!CalendarSupport::hasIncidence(item)) {
        qCCritical(KORGANIZER_LOG) << "CalendarView::deleteIncidence(): Item does not contain incidence.";
        return false;
    }
    return deleteIncidence(item, force);
}

void CalendarView::toggleAlarm(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }
    KCalendarCore::Incidence::Ptr const oldincidence(incidence->clone());

    const KCalendarCore::Alarm::List alarms = incidence->alarms();
    KCalendarCore::Alarm::List::ConstIterator it;
    KCalendarCore::Alarm::List::ConstIterator const end(alarms.constEnd());
    for (it = alarms.constBegin(); it != end; ++it) {
        (*it)->toggleAlarm();
    }
    if (alarms.isEmpty()) {
        // Add an alarm if it didn't have one
        KCalendarCore::Alarm::Ptr const alm = incidence->newAlarm();
        CalendarSupport::createAlarmReminder(alm, incidence->type());
    }
    mChanger->startAtomicOperation(i18nc("@info/plain", "Toggle Reminder"));
    (void)mChanger->modifyIncidence(item, oldincidence, this);
    mChanger->endAtomicOperation();
    if (incidence->hasEnabledAlarms()) {
        showMessage(xi18nc("@info alarm enabled", "<message>%1</message> reminder is enabled", incidence->summary()), KMessageWidget::Information);
    } else {
        showMessage(xi18nc("@info alarm disabled", "<message>%1</message> reminder is disabled", incidence->summary()), KMessageWidget::Information);
    }
}

void CalendarView::toggleTodoCompleted(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);

    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }
    if (incidence->type() != KCalendarCore::Incidence::TypeTodo) {
        qCDebug(KORGANIZER_LOG) << "called for a non-Todo incidence";
        return;
    }

    KCalendarCore::Todo::Ptr const todo = Akonadi::CalendarUtils::todo(item);
    Q_ASSERT(todo);
    KCalendarCore::Todo::Ptr const oldtodo(todo->clone());

    if (todo->isCompleted()) {
        todo->setPercentComplete(0);
        todo->setCompleted(false);
        todo->setStatus(KCalendarCore::Incidence::StatusNone);
    } else {
        todo->setPercentComplete(0);
        todo->setCompleted(QDateTime::currentDateTime());
        todo->setStatus(KCalendarCore::Incidence::StatusCompleted);
    }

    mChanger->startAtomicOperation(i18nc("@info/plain", "Toggle To-do Completed"));
    (void)mChanger->modifyIncidence(item, oldtodo, this);
    mChanger->endAtomicOperation();
    showMessage(xi18nc("@info percentage completed", "To-do <message>%1</message> %2 percent completed", todo->summary(), todo->percentComplete()),
                KMessageWidget::Information);
}

void CalendarView::toggleCompleted(const KCalendarCore::Todo::Ptr &todo, const QDate &occurrenceDate)
{
    if (todo->recurs()) {
        QDateTime const recurrenceId = recurrenceOnDate(todo, occurrenceDate);
        const auto dtRecurrence = todo->dtRecurrence();
        if (todo->isCompleted()) {
            todo->setCompleted(false);
            todo->setDtRecurrence(recurrenceId);
        } else if (dtRecurrence <= recurrenceId) {
            // Occurrence is not complete.
            const auto dtNextOccurrence = todo->recurrence()->getNextDateTime(recurrenceId);
            if (dtNextOccurrence.isValid()) {
                todo->setDtRecurrence(dtNextOccurrence);
            } else {
                todo->setDtRecurrence(recurrenceId);
                todo->setCompleted(true);
            }
        } else {
            todo->setDtRecurrence(recurrenceId);
        }
    } else {
        if (todo->isCompleted()) {
            todo->setPercentComplete(0);
            todo->setCompleted(false);
            todo->setStatus(KCalendarCore::Incidence::StatusNone);
        } else {
            todo->setPercentComplete(0);
            todo->setCompleted(QDateTime::currentDateTime());
            todo->setStatus(KCalendarCore::Incidence::StatusCompleted);
        }
    }
}

void CalendarView::toggleOccurrenceCompleted(const Akonadi::Item &todoItem, const QDate &occurrenceDate)
{
    KCalendarCore::Todo::Ptr const todo = Akonadi::CalendarUtils::todo(todoItem);
    if (!todo) {
        qWarning(KORGANIZER_LOG) << "item does not contain a todo.";
        return;
    }
    if (todo->recurs() && !occurrenceDate.isValid()) {
        qWarning(KORGANIZER_LOG) << "todo recurs, but no occurrence date was provided.";
        return;
    }

    KCalendarCore::Todo::Ptr const oldtodo{todo->clone()};
    toggleCompleted(todo, occurrenceDate);
    mChanger->startAtomicOperation(i18nc("@info/plain", "Toggle To-do Completed"));
    (void)mChanger->modifyIncidence(todoItem, oldtodo, this);
    mChanger->endAtomicOperation();
}

void CalendarView::copyIncidenceToResource(const Akonadi::Item &item, const Akonadi::Collection &col)
{
#ifdef AKONADI_PORT_DISABLED
    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }

    KCalendarCore::CalendarResources *const resources = KOrg::StdCalendar::self();
    KCalendarCore::CalendarResourceManager *const manager = resources->resourceManager();

    // Find the resource the incidence should be copied to
    ResourceCalendar *newCal = nullptr;
    KCalendarCore::CalendarResourceManager::iterator it;
    for (it = manager->begin(); it != manager->end(); ++it) {
        ResourceCalendar *const resource = *it;
        if (resource->identifier() == resourceId) {
            newCal = resource;
            break;
        }
    }
    if (!newCal) {
        return;
    }

    // Clone a new Incidence from the selected Incidence and give it a new Uid.
    KCalendarCore::Incidence::Ptr newInc;
    if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        KCalendarCore::Event::Ptr nEvent(static_cast<KCalendarCore::Event::Ptr>(incidence)->clone());
        nEvent->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nEvent;
    } else if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        KCalendarCore::Todo::Ptr nTodo(static_cast<KCalendarCore::Todo::Ptr>(incidence)->clone());
        nTodo->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nTodo;
    } else if (incidence->type() == KCalendarCore::Incidence::TypeJournal) {
        KCalendarCore::Journal::Ptr nJournal(static_cast<KCalendarCore::Journal::Ptr>(incidence)->clone());
        nJournal->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nJournal;
    } else {
        qCWarning(KORGANIZER_LOG) << "Trying to copy an incidence type that cannot be copied";
        return;
    }

    if (resources->addIncidence(newInc, newCal)) {
        incidenceAddFinished(newInc, true);
        KMessageBox::information(this,
                                 i18nc("@info", "\"%1\" was successfully copied to %2.", incidence->summary(), newCal->resourceName()),
                                 i18nc("@title:window", "Copying Succeeded"),
                                 "CalendarIncidenceCopy");
    } else {
        KMessageBox::error(this,
                           i18nc("@info", "Unable to copy the item \"%1\" to %2.", incidence->summary(), newCal->resourceName()),
                           i18nc("@title:window", "Copying Failed"));
    }
#else
    Q_UNUSED(col)
    Q_UNUSED(item)
    qCDebug(KORGANIZER_LOG) << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void CalendarView::moveIncidenceToResource(const Akonadi::Item &item, const Akonadi::Collection &col)
{
#ifdef AKONADI_PORT_DISABLED
    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }

    KCalendarCore::CalendarResources *const resources = KOrg::StdCalendar::self();
    KCalendarCore::CalendarResourceManager *const manager = resources->resourceManager();

    // Find the resource the incidence should be moved to
    ResourceCalendar *newCal = nullptr;
    KCalendarCore::CalendarResourceManager::iterator it;
    for (it = manager->begin(); it != manager->end(); ++it) {
        ResourceCalendar *const resource = *it;
        if (resource->identifier() == resourceId) {
            newCal = resource;
            break;
        }
    }
    if (!newCal) {
        return;
    }

    // Clone a new Incidence from the selected Incidence and give it a new Uid.
    KCalendarCore::Incidence *newInc;
    if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        KCalendarCore::Event::Ptr nEvent = static_cast<KCalendarCore::Event::Ptr>(incidence)->clone();
        nEvent->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nEvent;
    } else if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        KCalendarCore::Todo::Ptr nTodo = static_cast<KCalendarCore::Todo::Ptr>(incidence)->clone();
        nTodo->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nTodo;
    } else if (incidence->type() == KCalendarCore::Incidence::TypeJournal) {
        KCalendarCore::Journal::Ptr nJournal = static_cast<KCalendarCore::Journal::Ptr>(incidence)->clone();
        nJournal->setUid(KCalendarCore::CalFormat::createUniqueId());
        newInc = nJournal;
    } else {
        qCWarning(KORGANIZER_LOG) << "Trying to move an incidence type that cannot be moved";
        return;
    }

    if (resources->addIncidence(newInc, newCal)) {
        incidenceAddFinished(newInc, true);
        ResourceCalendar *const oldCal = resources->resource(incidence);
        if (!oldCal || resources->deleteIncidence(incidence)) {
            KMessageBox::error(this,
                               i18nc("@info",
                                     "Unable to remove the item \"%1\" from the original calendar. "
                                     "However, a copy of this item has been put into %2",
                                     incidence->summary(),
                                     newCal->resourceName()),
                               i18nc("@title:window", "Moving Failed"));
        } else {
            incidenceDeleteFinished(incidence, true);
            KMessageBox::information(
                this,
                i18nc("@info", "\"%1\" was successfully moved from %2 to %3.", incidence->summary(), oldCal->resourceName(), newCal->resourceName()),
                i18nc("@title:window", "Moving Succeeded"),
                "CalendarIncidenceMove");
        }
    } else {
        KMessageBox::error(this,
                           i18nc("@info",
                                 "Unable to add the item \"%1\" into %2. "
                                 "This item has not been moved.",
                                 incidence->summary(),
                                 newCal->resourceName()),
                           i18nc("@title:window", "Moving Failed"));
    }
#else
    Q_UNUSED(col)
    Q_UNUSED(item)
    qCDebug(KORGANIZER_LOG) << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

QDateTime CalendarView::recurrenceOnDate(const KCalendarCore::Incidence::Ptr &incidence, const QDate &displayDate)
{
    const auto start = incidence->dateTime(KCalendarCore::IncidenceBase::RoleDisplayStart);
    const auto offset = start.toLocalTime().date().daysTo(displayDate);
    return incidence->dateTime(KCalendarCore::IncidenceBase::RoleRecurrenceStart).addDays(offset);
}

void CalendarView::dissociateOccurrences(const Akonadi::Item &item, QDate date)
{
    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);

    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return;
    }

    QDateTime const recurrenceId = recurrenceOnDate(incidence, date);
    bool const isFirstOccurrence = !incidence->recurrence()->getPreviousDateTime(recurrenceId).isValid();

    int answer;
    bool doOnlyThis = false;
    bool doFuture = false;

    if (isFirstOccurrence) {
        answer = KMessageBox::questionTwoActions(this,
                                                 i18nc("@info",
                                                       "Do you want to dissociate "
                                                       "the occurrence on %1 "
                                                       "from the recurrence?",
                                                       QLocale::system().toString(date, QLocale::LongFormat)),
                                                 i18nc("@title:window", "KOrganizer Confirmation"),
                                                 KGuiItem(i18nc("@action:button", "&Dissociate")),
                                                 KStandardGuiItem::cancel());

        doOnlyThis = (answer == KMessageBox::ButtonCode::PrimaryAction);
    } else {
        answer = KMessageBox::questionTwoActionsCancel(this,
                                                       i18nc("@info",
                                                             "Do you want to dissociate "
                                                             "the occurrence on %1 "
                                                             "from the recurrence or also "
                                                             "dissociate future ones?",
                                                             QLocale::system().toString(date, QLocale::LongFormat)),
                                                       i18nc("@title:window", "KOrganizer Confirmation"),
                                                       KGuiItem(i18nc("@action:button", "&Only Dissociate This One")),
                                                       KGuiItem(i18nc("@action:button", "&Also Dissociate Future Ones")));

        doOnlyThis = (answer == KMessageBox::ButtonCode::PrimaryAction);
        doFuture = (answer == KMessageBox::ButtonCode::SecondaryAction);
    }

    if (doOnlyThis) {
        dissociateOccurrence(item, recurrenceId, false);
    } else if (doFuture) {
        dissociateOccurrence(item, recurrenceId, true);
    }
}

void CalendarView::dissociateOccurrence(const Akonadi::Item &item, const QDateTime &recurrenceId, bool thisAndFuture)
{
    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);

    if (thisAndFuture) {
        startMultiModify(i18nc("@info/plain", "Dissociate future occurrences"));
    } else {
        startMultiModify(i18nc("@info/plain", "Dissociate occurrence"));
    }
    KCalendarCore::Incidence::Ptr const newInc(KCalendarCore::Calendar::createException(incidence, recurrenceId, thisAndFuture));
    if (newInc) {
        (void)mChanger->createIncidence(newInc, item.parentCollection(), this);
    } else {
        if (thisAndFuture) {
            KMessageBox::error(this, i18nc("@info", "Dissociating the future occurrences failed."), i18nc("@title:window", "Dissociating Failed"));
        } else {
            KMessageBox::error(this, i18nc("@info", "Dissociating the occurrence failed."), i18nc("@title:window", "Dissociating Failed"));
        }
    }
    endMultiModify();
}

void CalendarView::schedule_publish(const Akonadi::Item &item)
{
    Akonadi::Item selectedItem = item;
    if (!item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        selectedItem = selectedIncidence();
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(selectedItem);
    if (incidence) {
        mITIPHandler->publishInformation(incidence, this);
    }
}

void CalendarView::schedule_request(const Akonadi::Item &item)
{
    schedule(KCalendarCore::iTIPRequest, item);
}

void CalendarView::schedule_refresh(const Akonadi::Item &item)
{
    schedule(KCalendarCore::iTIPRefresh, item);
}

void CalendarView::schedule_cancel(const Akonadi::Item &item)
{
    schedule(KCalendarCore::iTIPCancel, item);
}

void CalendarView::schedule_reply(const Akonadi::Item &item)
{
    schedule(KCalendarCore::iTIPReply, item);
}

void CalendarView::schedule_counter(const Akonadi::Item &item)
{
    schedule(KCalendarCore::iTIPCounter, item);
}

void CalendarView::schedule_forward(const Akonadi::Item &item)
{
    Akonadi::Item selectedItem = item;
    if (!item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        selectedItem = selectedIncidence();
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(selectedItem);

    if (incidence) {
        mITIPHandler->sendAsICalendar(incidence, this);
    }
}

void CalendarView::mailFreeBusy(int daysToPublish)
{
    Akonadi::FreeBusyManager::self()->mailFreeBusy(daysToPublish, this);
}

void CalendarView::uploadFreeBusy()
{
    Akonadi::FreeBusyManager::self()->publishFreeBusy(this);
}

void CalendarView::schedule(KCalendarCore::iTIPMethod method, const Akonadi::Item &item)
{
    Akonadi::Item selectedItem = item;
    if (!item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        selectedItem = selectedIncidence();
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(selectedItem);

    if (incidence) {
        mITIPHandler->sendiTIPMessage(method, incidence, this);
    }
}

void CalendarView::openAddressbook()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kaddressbook"), {}, this);
    job->setDesktopName(QStringLiteral("org.kde.kaddressbook"));
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}

bool CalendarView::isReadOnly() const
{
    return mReadOnly;
}

void CalendarView::print()
{
    createPrinter();

    KOrg::BaseView *curView = mViewManager->currentView();

    CalendarSupport::CalPrinter::PrintType printType = CalendarSupport::CalPrinter::Month;

    KCalendarCore::Incidence::List selectedIncidences;
    if (curView) {
        printType = curView->printType();
        const Akonadi::Item::List selectedViewIncidences = curView->selectedIncidences();
        for (const Akonadi::Item &item : selectedViewIncidences) {
            if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                selectedIncidences.append(item.payload<KCalendarCore::Incidence::Ptr>());
            }
        }
    }

    KCalendarCore::DateList tmpDateList = mDateNavigator->selectedDates();
    mCalPrinter->print(printType, tmpDateList.first(), tmpDateList.last(), selectedIncidences);
}

void CalendarView::printPreview()
{
    createPrinter();

    KOrg::BaseView *curView = mViewManager->currentView();

    CalendarSupport::CalPrinter::PrintType printType = CalendarSupport::CalPrinter::Month;

    KCalendarCore::Incidence::List selectedIncidences;
    if (curView) {
        printType = curView->printType();
        const Akonadi::Item::List selectedViewIncidences = curView->selectedIncidences();
        for (const Akonadi::Item &item : selectedViewIncidences) {
            if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                selectedIncidences.append(item.payload<KCalendarCore::Incidence::Ptr>());
            }
        }
    }

    KCalendarCore::DateList tmpDateList = mDateNavigator->selectedDates();
    mCalPrinter->print(printType, tmpDateList.first(), tmpDateList.last(), selectedIncidences, true);
}

void CalendarView::slotWhatsNew()
{
    const WhatsNewTranslations translations;
    PimCommon::WhatsNewDialog dlg(translations.createWhatsNewInfo(), this, i18nc("@title:window", "KOrganizer"));
    dlg.updateInformations();
    dlg.exec();
}

void CalendarView::exportICalendar()
{
    QString exportPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("Settings"));
    const QFileInfo lastLocation(group.readEntry("LastExportLocation", QString()));
    const QString lastExportPath = lastLocation.absolutePath();
    if (!lastExportPath.isEmpty()) {
        exportPath = lastExportPath;
    }
    exportPath = exportPath + QStringLiteral("/icalout.ics");

    QString filename = QFileDialog::getSaveFileName(this,
                                                    i18nc("@title:window", "Export Calendar to File"),
                                                    exportPath,
                                                    QStringLiteral("text/calendar (*.ics)"),
                                                    nullptr,
                                                    QFileDialog::DontConfirmOverwrite);
    if (!filename.isEmpty()) {
        // Force correct extension
        if (filename.right(4) != QLatin1StringView(".ics")) {
            filename += QLatin1StringView(".ics");
        }

        if (QFileInfo::exists(filename)) {
            const int answer = KMessageBox::warningContinueCancel(this,
                                                                  xi18nc("@info", "Do you want to overwrite <filename>%1</filename>?", filename),
                                                                  i18nc("@title:window", "Export Calendar"),
                                                                  KStandardGuiItem::overwrite());
            if (answer == KMessageBox::Cancel) {
                return;
            }
        }
        auto format = new KCalendarCore::ICalFormat;

        KCalendarCore::FileStorage storage(mCalendar, filename, format);
        if (!storage.save()) {
            QString errmess;
            if (format->exception()) {
                errmess = KCalUtils::Stringify::errorMessage(*format->exception());
            } else {
                errmess = i18nc("@info save failure cause unknown", "Reason unknown");
            }
            KMessageBox::error(this, xi18nc("@info", "Cannot write iCalendar file <filename>%1</filename>: <message>%2</message>", filename, errmess));
        } else {
            group.writeEntry("LastExportLocation", filename);
        }
    }
}

void CalendarView::processMainViewSelection(const Akonadi::Item &item, const QDate &date)
{
    if (CalendarSupport::hasIncidence(item)) {
        mTodoList->clearSelection();
    }
    processIncidenceSelection(item, date);
}

void CalendarView::processTodoListSelection(const Akonadi::Item &item, const QDate &date)
{
    if (CalendarSupport::hasIncidence(item) && mViewManager->currentView()) {
        mViewManager->currentView()->clearSelection();
    }
    processIncidenceSelection(item, date);
}

void CalendarView::processIncidenceSelection(const Akonadi::Item &item, const QDate &date)
{
    if (item != mSelectedIncidence || date != mSaveDate) {
        // This signal also must be emitted if incidence is 0
        Q_EMIT incidenceSelected(item, date);
    }

    if (!item.isValid()) {
        mSelectedIncidence = item;
        return;
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        mSelectedIncidence = item;
        return;
    }
    if (item == mSelectedIncidence) {
        if (!incidence->recurs() || mSaveDate == date) {
            return;
        }
    }

    mSelectedIncidence = item;
    mSaveDate = date;

    bool todo = false;
    bool subtodo = false;

    const bool organizerEvents = CalendarSupport::KCalPrefs::instance()->thatIsMe(incidence->organizer().email());
    const bool groupEvents = !incidence->attendeeByMails(CalendarSupport::KCalPrefs::instance()->allEmails()).isNull();

    if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        todo = true;
        subtodo = !incidence->relatedTo().isEmpty();
    }
    Q_EMIT todoSelected(todo);
    Q_EMIT subtodoSelected(subtodo);
    Q_EMIT organizerEventsSelected(organizerEvents);
    Q_EMIT groupEventsSelected(groupEvents);
}

void CalendarView::checkClipboard()
{
    Q_EMIT pasteEnabled(mCalendarClipboard->pasteAvailable());
}

void CalendarView::agendaViewMenuSelector(const KCalendarCore::DateList &dates)
{
    // a range of dates was selected in the date navigator.  week selections are done in selectWeek()
    if (mViewManager->currentView() == mViewManager->agendaView()) {
        if (dates.first() == dates.last()) {
            mViewManager->viewActionEnable(mViewManager->viewToAction(QStringLiteral("Agenda"), KOViewManager::DAY_RANGE));
        } else {
            mViewManager->viewActionEnable(mViewManager->viewToAction(QStringLiteral("Agenda"), KOViewManager::NEXTX_RANGE));
        }
    }
}

void CalendarView::showDates(const KCalendarCore::DateList &selectedDates, const QDate &preferredMonth)
{
    mDateNavigatorContainer->selectDates(selectedDates, preferredMonth);
    mNavigatorBar->selectDates(selectedDates);

    if (mViewManager->currentView()) {
        updateView(selectedDates.first(), selectedDates.last(), preferredMonth, false);
        agendaViewMenuSelector(selectedDates);
    } else {
        mViewManager->showAgendaView();
    }
}

void CalendarView::editFilters()
{
    mDialogManager->showFilterEditDialog(&mFilters);
}

void CalendarView::updateFilter()
{
    QStringList filters;

    int const pos = mFilters.indexOf(mCurrentFilter);
    if (pos < 0) {
        mCurrentFilter = nullptr;
    }

    filters << i18nc("@info/plain", "No filter");
    for (KCalendarCore::CalFilter *filter : std::as_const(mFilters)) {
        if (filter) {
            filters << filter->name();
        }
    }

    // account for the additional "No filter" at the beginning! if the
    // filter is not in the list, pos == -1...
    Q_EMIT filtersUpdated(filters, pos + 1);

    mCalendar->setFilter(mCurrentFilter);
    forEachCalendar([this](const auto &calendar) {
        calendar->setFilter(mCurrentFilter);
    });
}

void CalendarView::filterActivated(int filterNo)
{
    KCalendarCore::CalFilter *newFilter = nullptr;
    if (filterNo > 0 && filterNo <= int(mFilters.count())) {
        newFilter = mFilters.at(filterNo - 1);
    }
    if (newFilter != mCurrentFilter) {
        mCurrentFilter = newFilter;
        mCalendar->setFilter(mCurrentFilter);
        forEachCalendar([this](const auto &calendar) {
            calendar->setFilter(mCurrentFilter);
        });
        mViewManager->addChange(EventViews::EventView::FilterChanged);
        updateView();
    }
    Q_EMIT filterChanged();
}

bool CalendarView::isFiltered() const
{
    return mCurrentFilter != nullptr;
}

QString CalendarView::currentFilterName() const
{
    if (mCurrentFilter) {
        return mCurrentFilter->name();
    } else {
        return i18nc("@info/plain", "No filter");
    }
}

void CalendarView::showDateNavigator(bool show)
{
    if (show) {
        mDateNavigatorContainer->show();
        mDateNavigatorContainer->updateView();
    } else {
        mDateNavigatorContainer->hide();
    }
}

void CalendarView::showTodoView(bool show)
{
    if (show) {
        if (!mTodoList->model()) {
            mTodoList->setModel(mCalendar->entityTreeModel());
        }
        mTodoList->show();
        mTodoList->updateView();
    } else {
        mTodoList->hide();
    }
}

void CalendarView::showEventViewer(bool show)
{
    if (show) {
        mEventViewerBox->show();
    } else {
        mEventViewerBox->hide();
    }
}

void CalendarView::addView(KOrg::BaseView *view)
{
    mViewManager->addView(view);
}

void CalendarView::showView(KOrg::BaseView *view)
{
    mViewManager->showView(view);
}

void CalendarView::addExtension(CalendarViewExtension::Factory *factory)
{
    CalendarViewExtension *extension = factory->create(mLeftSplitter);
    mExtensions.append(extension);
    if (!mETMCollectionView) {
        mETMCollectionView = qobject_cast<AkonadiCollectionView *>(extension);
    }
}

void CalendarView::showLeftFrame(bool show)
{
    if (show) {
        mMainSplitterSizes.clear();
        mLeftFrame->show();
        Q_EMIT calendarViewExpanded(false);
    } else {
        // mPanner splitter sizes are useless if mLeftFrame is hidden, so remember them beforehand.
        if (mMainSplitterSizes.isEmpty()) {
            mMainSplitterSizes = mPanner->sizes();
        }

        mLeftFrame->hide();
        Q_EMIT calendarViewExpanded(true);
    }
}

Akonadi::Item CalendarView::selectedTodo()
{
    const Akonadi::Item item = currentSelection();
    if (const KCalendarCore::Todo::Ptr t = Akonadi::CalendarUtils::todo(item)) {
        return item;
    }

    Akonadi::Item incidence;

    const Akonadi::Item::List selectedIncidences = mTodoList->selectedIncidences();
    if (!selectedIncidences.isEmpty()) {
        incidence = selectedIncidences.first();
    }
    if (const KCalendarCore::Todo::Ptr t = Akonadi::CalendarUtils::todo(item)) {
        return item;
    }
    return {};
}

Akonadi::Item CalendarView::currentSelection()
{
    return mViewManager->currentSelection();
}

Akonadi::Item CalendarView::selectedIncidence()
{
    Akonadi::Item item = currentSelection();
    if (!item.isValid()) {
        Akonadi::Item::List selectedIncidences = mTodoList->selectedIncidences();
        if (!selectedIncidences.isEmpty()) {
            item = selectedIncidences.first();
        }
    }
    return item;
}

void CalendarView::showIncidence()
{
    showIncidence(selectedIncidence());
}

void CalendarView::editIncidence()
{
    editIncidence(selectedIncidence());
}

bool CalendarView::editIncidence(Akonadi::Item::Id id)
{
    Akonadi::Item const item = mCalendar->item(id);
    return editIncidence(item);
}

bool CalendarView::showIncidence(Akonadi::Item::Id id)
{
    Akonadi::Item const item = mCalendar->item(id);
    if (!CalendarSupport::hasIncidence(item)) {
        return false;
    }
    showIncidence(item);
    return true;
}

bool CalendarView::showIncidenceContext(Akonadi::Item::Id id)
{
    Akonadi::Item const item = mCalendar->item(id);
    if (!CalendarSupport::hasIncidence(item)) {
        return false;
    }
    showIncidenceContext(item);
    return true;
}

void CalendarView::deleteIncidence()
{
    deleteIncidence(selectedIncidence());
}

void CalendarView::cutIncidence(const Akonadi::Item &incidence)
{
    Q_UNUSED(incidence)
    edit_cut();
}

void CalendarView::copyIncidence(const Akonadi::Item &incidence)
{
    Q_UNUSED(incidence)
    edit_copy();
}

void CalendarView::pasteIncidence()
{
    edit_paste();
}

void CalendarView::showIncidence(const Akonadi::Item &item)
{
    showOccurrence(item, QDate());
}

void CalendarView::showOccurrence(const Akonadi::Item &item, const QDate &date)
{
    auto *eventViewer = new KOEventViewerDialog(mCalendar.data(), this);
    eventViewer->setIncidence(item, date);
    // Disable the Edit button for read-only Incidences.
    if (!mCalendar->hasRight(item, Akonadi::Collection::CanChangeItem)) {
        eventViewer->editButton()->setEnabled(false);
    }

    eventViewer->show();
}

void CalendarView::showIncidenceContext(const Akonadi::Item &item)
{
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    if (CalendarSupport::hasEvent(item)) {
        if (!viewManager()->currentView()->inherits("KOEventView")) {
            viewManager()->showAgendaView();
        }
        // just select the appropriate date
        mDateNavigator->selectWeek(incidence->dtStart().toLocalTime().date());
        return;
    } else if (CalendarSupport::hasJournal(item)) {
        if (!viewManager()->currentView()->inherits("KOJournalView")) {
            viewManager()->showJournalView();
        }
    } else if (CalendarSupport::hasTodo(item)) {
        if (!viewManager()->currentView()->inherits("KOTodoView")) {
            viewManager()->showTodoView();
        }
    }
    Akonadi::Item::List list;
    list.append(item);
    viewManager()->currentView()->showIncidences(list, QDate());
}

bool CalendarView::editIncidence(const Akonadi::Item &item, bool isCounter)
{
    Q_UNUSED(isCounter)
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        qCCritical(KORGANIZER_LOG) << "Null incidence";
        return false;
    }

    if (!mCalendar->hasRight(item, Akonadi::Collection::CanChangeItem)) {
        showIncidence(item);
        return true;
    }

    IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog(item);
    dialog->load(item, activeIncidenceDate()); // Show the dialog as soon as it loads the item.

    return true;
}

void CalendarView::showIncidenceByUid(const QString &uid, QDate activeDate)
{
    const auto item = mCalendar->item(uid);
    if (item.isValid()) {
        showIncidenceContext(item);
        mEventViewer->setIncidence(item, activeDate);
    }
}

void CalendarView::deleteIncidenceFamily(const Akonadi::Item &item)
{
    const auto incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        return;
    }
    deleteChildren(item);
    deleteRecurringIncidence(item);
}

void CalendarView::deleteChildren(const Akonadi::Item &item)
{
    const auto incidence = Akonadi::CalendarUtils::incidence(item);
    if (incidence && !incidence->hasRecurrenceId()) {
        const Akonadi::Item::List childItems = mCalendar->childItems(item.id());
        for (const Akonadi::Item &c : childItems) {
            deleteIncidenceFamily(c);
        }
    }
}

void CalendarView::deleteRecurringIncidence(const Akonadi::Item &todoItem)
{
    if (!mChanger->deletedRecently(todoItem.id())) {
        auto incidence = Akonadi::CalendarUtils::incidence(todoItem);
        if (incidence->recurs()) {
            const KCalendarCore::Incidence::List instances = mCalendar->instances(incidence);
            for (const KCalendarCore::Incidence::Ptr &instance : instances) {
                (void)mChanger->deleteIncidence(mCalendar->item(instance), this);
            }
        }
        (void)mChanger->deleteIncidence(todoItem, this);
    }
}

int CalendarView::questionIndependentChildren(const Akonadi::Item &item)
{
    int km;
    auto incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence->hasRecurrenceId() && !mCalendar->childItems(item.id()).isEmpty()) {
        km = KMessageBox::questionTwoActionsCancel(this,
                                                   xi18nc("@info",
                                                          "The item <resource>%1</resource> has sub-to-dos. "
                                                          "Do you want to delete just this item and "
                                                          "make all its sub-to-dos independent, or "
                                                          "delete the to-do with all its sub-to-dos?",
                                                          incidence->summary()),
                                                   i18nc("@title:window", "KOrganizer Confirmation"),
                                                   KGuiItem(i18nc("@action:button", "Delete Only This")),
                                                   KGuiItem(i18nc("@action:button", "Delete All")));

        if (km == KMessageBox::ButtonCode::SecondaryAction) {
            km = KMessageBox::Continue;
        }
    } else {
        km = msgItemDelete(item);
    }
    return km;
}

bool CalendarView::deleteIncidence(const Akonadi::Item &item, bool force)
{
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        if (!force) {
            qCCritical(KORGANIZER_LOG) << "Null incidence";
        }
        qCCritical(KORGANIZER_LOG) << "CalendarView::deleteIncidence(): Unable to delete, incidence is null.";
        return false;
    }

    if (mChanger->deletedRecently(item.id())) {
        // it was deleted already but the etm wasn't notified yet
        qCWarning(KORGANIZER_LOG) << "CalendarView::deleteIncidence(): item with id" << item.id() << "was deleted recently, skipping";
        return true;
    }

    if (!mCalendar->hasRight(item, Akonadi::Collection::CanDeleteItem)) {
        if (!force) {
            KMessageBox::information(this,
                                     xi18nc("@info",
                                            "The item <resource>%1</resource> is marked read-only "
                                            "and cannot be deleted; it probably belongs to a read-only calendar.",
                                            incidence->summary()),
                                     i18nc("@title:window", "Removing not possible"),
                                     QStringLiteral("deleteReadOnlyIncidence"));
        }
        qCWarning(KORGANIZER_LOG) << "CalendarView::deleteIncidence(): No rights to delete item";
        return false;
    }

    QDate const itemDate = mViewManager->currentSelectionDate();

    int km;
    if (force) {
        km = ItemActions::All;
    } else if (!incidence->recurs()) { // Non-recurring, or instance of recurring.
        km = questionIndependentChildren(item);
    } else { // Recurring incidence
        if (!itemDate.isValid()) {
            qCDebug(KORGANIZER_LOG) << "Date Not Valid";
            km = KMessageBox::warningContinueCancel(this,
                                                    xi18nc("@info",
                                                           "The calendar item <resource>%1</resource> recurs over multiple dates; "
                                                           "are you sure you want to delete it and all its recurrences?",
                                                           incidence->summary()),
                                                    i18nc("@title:window", "KOrganizer Confirmation"),
                                                    KGuiItem(i18nc("@action:button", "Delete All")));
        } else {
            QDateTime const itemDateTime(itemDate, {}, QTimeZone::LocalTime);
            bool const isFirst = !incidence->recurrence()->getPreviousDateTime(itemDateTime).isValid();
            bool const isLast = !incidence->recurrence()->getNextDateTime(itemDateTime).isValid();

            QString message;
            QString const itemFuture(i18nc("@action:button", "Also Delete &Future"));

            if (!isFirst && !isLast) {
                message = xi18nc("@info",
                                 "The calendar item <resource>%1</resource> recurs over multiple dates. "
                                 "Do you want to delete only the current one on %2, also "
                                 "future occurrences, or all its occurrences?",
                                 incidence->summary(),
                                 QLocale::system().toString(itemDate, QLocale::LongFormat));
            } else {
                message = xi18nc("@info",
                                 "The calendar item <resource>%1</resource> recurs over multiple dates. "
                                 "Do you want to delete only the current one on %2 "
                                 "or all its occurrences?",
                                 incidence->summary(),
                                 QLocale::system().toString(itemDate, QLocale::LongFormat));
            }

            if (!(isFirst && isLast)) {
                QDialogButtonBox::StandardButton const returnValue = PIMMessageBox::fourBtnMsgBox(this,
                                                                                                  QMessageBox::Warning,
                                                                                                  message,
                                                                                                  i18nc("@title:window", "KOrganizer Confirmation"),
                                                                                                  i18nc("@action:button", "Delete C&urrent"),
                                                                                                  itemFuture,
                                                                                                  i18nc("@action:button", "Delete &All"));
                switch (returnValue) {
                case QDialogButtonBox::Ok:
                    if (!mCalendar->childItems(item.id()).isEmpty()) {
                        km = questionIndependentChildren(item);
                    } else {
                        km = ItemActions::All;
                    }
                    break;
                case QDialogButtonBox::Yes:
                    km = ItemActions::Current;
                    break;
                case QDialogButtonBox::No:
                    km = ItemActions::AlsoFuture;
                    break;
                case QDialogButtonBox::Cancel:
                default:
                    km = KMessageBox::Cancel;
                    break;
                }
            } else {
                km = questionIndependentChildren(item);
            }
        }
    }

    KCalendarCore::Incidence::Ptr const oldIncidence(incidence->clone());
    KCalendarCore::Recurrence *recur = incidence->recurrence();

    switch (km) {
    case ItemActions::All:
        startMultiModify(i18nc("@info/plain", "Delete \"%1\"", incidence->summary()));
        deleteChildren(item);
        deleteRecurringIncidence(item);
        endMultiModify();
        break;

    case ItemActions::Parent:
        startMultiModify(i18nc("@info/plain", "Delete \"%1\"", incidence->summary()));
        makeChildrenIndependent(item);
        deleteRecurringIncidence(item);
        endMultiModify();
        break;

    case ItemActions::Current:
        if (recur->allDay()) {
            recur->addExDate(itemDate);
        } else {
            // Convert to local time, because the `itemDate` is a day in local timezone, but the actual
            // incidence might be originally recurring in another day in it's original timezone.
            auto itemDateTime = recur->startDateTime().toLocalTime();
            itemDateTime.setDate(itemDate);
            recur->addExDateTime(itemDateTime.toTimeZone(recur->startDateTime().timeZone()));
        }
        (void)mChanger->modifyIncidence(item, oldIncidence, this);
        break;

    case ItemActions::AlsoFuture:
        recur->setEndDate(itemDate.addDays(-1));
        (void)mChanger->modifyIncidence(item, oldIncidence, this);
        break;
    default: // cancel
        break;
    }
    return true;
}

void CalendarView::purgeCompleted()
{
    if (checkedCollections().isEmpty()) {
        showMessage(i18nc("@info", "All calendars are unchecked in the Calendar Manager. No to-do was purged."), KMessageWidget::Warning);
        return;
    }

    if (mCalendar->rawTodos().isEmpty()) {
        showMessage(i18nc("@info", "There are no completed to-dos to purge."), KMessageWidget::Information);
        return;
    }

    int const result = KMessageBox::warningContinueCancel(this,
                                                          i18nc("@info", "Delete all completed to-dos from checked calendars?"),
                                                          i18nc("@title:window", "Purge To-dos"),
                                                          KGuiItem(i18nc("@action:button", "Purge"), QIcon::fromTheme(QStringLiteral("entry-delete"))));

    if (result == KMessageBox::Continue) {
        mTodoPurger->purgeCompletedTodos();
    }
}

void CalendarView::addIncidenceOn(const Akonadi::Item &itemadd, const QDate &dt)
{
    if (!CalendarSupport::hasIncidence(itemadd)) {
        KMessageBox::error(this, xi18nc("@info", "Unable to copy item <resource>%1</resource>.", dt.toString()), i18nc("@title:window", "Copying Failed"));
        return;
    }
    Akonadi::Item item = mCalendar->item(itemadd.id());
    if (!item.isValid()) {
        item = itemadd;
    }
    // Create a copy of the incidence, since the incadd doesn't belong to us.
    KCalendarCore::Incidence::Ptr const incidence(Akonadi::CalendarUtils::incidence(item)->clone());
    incidence->recreate();

    if (const KCalendarCore::Event::Ptr event = incidence.dynamicCast<KCalendarCore::Event>()) {
        // Adjust date
        QDateTime start = event->dtStart();
        QDateTime end = event->dtEnd();

        int const duration = start.daysTo(end);
        start.setDate(dt);
        end.setDate(dt.addDays(duration));

        event->setDtStart(start);
        event->setDtEnd(end);
    } else if (const KCalendarCore::Todo::Ptr todo = incidence.dynamicCast<KCalendarCore::Todo>()) {
        QDateTime due = todo->dtDue();
        due.setDate(dt);

        todo->setDtDue(due);
    }

    (void)mChanger->createIncidence(incidence, Akonadi::Collection(), this);
}

void CalendarView::moveIncidenceTo(const Akonadi::Item &itemmove, QDate dt)
{
    if (!CalendarSupport::hasIncidence(itemmove)) {
        KMessageBox::error(this, xi18nc("@info", "Unable to move item <resource>%1</resource>.", dt.toString()), i18nc("@title:window", "Moving Failed"));
        return;
    }
    Akonadi::Item const item = mCalendar->item(itemmove.id());
    if (!item.isValid()) {
        addIncidenceOn(itemmove, dt);
        return;
    }
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(itemmove);

    KCalendarCore::Incidence::Ptr const oldIncidence(incidence->clone());

    if (const KCalendarCore::Event::Ptr event = incidence.dynamicCast<KCalendarCore::Event>()) {
        // Adjust date
        QDateTime start = event->dtStart();
        QDateTime end = event->dtEnd();

        int const duration = start.daysTo(end);
        start.setDate(dt);
        end.setDate(dt.addDays(duration));

        event->setDtStart(start);
        event->setDtEnd(end);
    }
    if (const KCalendarCore::Todo::Ptr todo = incidence.dynamicCast<KCalendarCore::Todo>()) {
        QDateTime due = todo->dtDue();
        due.setDate(dt);

        todo->setDtDue(due);
    }
    (void)mChanger->modifyIncidence(itemmove, oldIncidence, this);
}

void CalendarView::resourcesChanged()
{
    mViewManager->addChange(EventViews::EventView::ResourcesChanged);
    updateView();
}

bool CalendarView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mLeftFrame && event->type() == QEvent::Show) {
        mSplitterSizesValid = true;
    }
    return KOrg::CalendarViewBase::eventFilter(watched, event);
}

void CalendarView::updateHighlightModes()
{
    KOrg::BaseView *view = mViewManager->currentView();
    if (view) {
        bool hiEvents;
        bool hiTodos;
        bool hiJournals;

        view->getHighlightMode(hiEvents, hiTodos, hiJournals);
        mDateNavigatorContainer->setHighlightMode(hiEvents, hiTodos, hiJournals);
    }
}

void CalendarView::selectWeek(const QDate &date, const QDate &preferredMonth)
{
    if (KOPrefs::instance()->mWeekNumbersShowWork && mViewManager->rangeMode() == KOViewManager::WORK_WEEK_RANGE) {
        mDateNavigator->selectWorkWeek(date);
        if (mViewManager->currentView() == mViewManager->agendaView()) {
            mViewManager->viewActionEnable(mViewManager->viewToAction(QStringLiteral("Agenda"), KOViewManager::WORK_WEEK_RANGE));
        }
    } else {
        mDateNavigator->selectWeek(date, preferredMonth);
        if (mViewManager->currentView() == mViewManager->agendaView()) {
            mViewManager->viewActionEnable(mViewManager->viewToAction(QStringLiteral("Agenda"), KOViewManager::WEEK_RANGE));
        }
    }
}

void CalendarView::changeFullView(bool fullView)
{
    if (mViewManager->currentView()) {
        if (mViewManager->currentView()->identifier() == "DefaultTodoView") {
            showLeftFrame(!fullView);
        } else if (mViewManager->currentView()->identifier() == "DefaultMonthView") {
            showLeftFrame(!fullView);
            fullView ? mNavigatorBar->show() : mNavigatorBar->hide();
        }
    }
}

static auto forCollection(const Akonadi::Collection &collection)
{
    return [collection](const Akonadi::CollectionCalendar::Ptr &calendar) {
        return calendar->collection() == collection;
    };
}

void CalendarView::collectionSelected(const Akonadi::Collection &collection)
{
    if (!collection.isValid()) {
        return;
    }

    // NOLINTBEGIN(modernize-use-ranges) to avoid detaching
    auto enabledCalendar = std::find_if(mEnabledCalendars.cbegin(), mEnabledCalendars.cend(), forCollection(collection));
    // NOLINTEND(modernize-use-ranges)
    if (enabledCalendar != mEnabledCalendars.cend()) {
        return;
    }

    const auto newCalendar = calendarForCollection(collection);
    mEnabledCalendars.push_back(newCalendar);
    mDateNavigatorContainer->addCalendar(newCalendar);
    Q_EMIT calendarAdded(newCalendar);
}

void CalendarView::collectionDeselected(const Akonadi::Collection &collection)
{
    if (!collection.isValid()) {
        return;
    }

    // NOLINTBEGIN(modernize-use-ranges) to avoid detaching
    auto calendarIt = std::find_if(mEnabledCalendars.cbegin(), mEnabledCalendars.cend(), forCollection(collection));
    // NOLINTEND(modernize-use-ranges)
    if (calendarIt == mEnabledCalendars.cend()) {
        return;
    }

    const auto &deselectCalendar = *calendarIt;
    deselectCalendar->setFilter(nullptr);
    mEnabledCalendars.removeOne(deselectCalendar);
    mDateNavigatorContainer->removeCalendar(deselectCalendar);
    Q_EMIT calendarRemoved(deselectCalendar);
}

Akonadi::Collection CalendarView::defaultCollection(const QLatin1StringView &mimeType) const
{
    // 1. Try the view collection ( used in multi-agenda view )
    Akonadi::Collection collection = mCalendar->collection(mViewManager->currentView()->collectionId());
    bool supportsMimeType = collection.contentMimeTypes().contains(mimeType) || mimeType == QLatin1StringView("");
    bool hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
    if (collection.isValid() && supportsMimeType && hasRights) {
        return collection;
    }

    // 2. Try the configured default collection
    collection = mCalendar->collection(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());
    supportsMimeType = collection.contentMimeTypes().contains(mimeType) || mimeType == QLatin1StringView("");
    hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
    if (collection.isValid() && supportsMimeType && hasRights) {
        return collection;
    }

    // 3. Try the selected collection
    collection = selectedCollection();
    supportsMimeType = collection.contentMimeTypes().contains(mimeType) || mimeType == QLatin1StringView("");
    hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
    if (collection.isValid() && supportsMimeType && hasRights) {
        return collection;
    }

    // 4. Try the checked collections
    const Akonadi::Collection::List collections = checkedCollections();
    for (const Akonadi::Collection &checkedCollection : collections) {
        supportsMimeType = checkedCollection.contentMimeTypes().contains(mimeType) || mimeType == QLatin1StringView("");
        hasRights = checkedCollection.rights() & Akonadi::Collection::CanCreateItem;
        if (checkedCollection.isValid() && supportsMimeType && hasRights) {
            return checkedCollection;
        }
    }

    // 5. Return a invalid collection, the editor will use the first one in the combo
    return {};
}

IncidenceEditorNG::IncidenceDialog *CalendarView::createIncidenceEditor(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog(item);
    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(item);
    Q_ASSERT(incidence);

    if (collection.isValid()) {
        dialog->selectCollection(collection);
    } else {
        dialog->selectCollection(defaultCollection(incidence->mimeType()));
    }

    return dialog;
}

Akonadi::History *CalendarView::history() const
{
    return mChanger->history();
}

void CalendarView::onCutFinished()
{
    checkClipboard();
}

void CalendarView::onTodosPurged(bool success, int numDeleted, int numIgnored)
{
    QString message;
    KMessageWidget::MessageType type = KMessageWidget::Information;
    if (success) {
        if (numDeleted == 0 && numIgnored > 0) {
            type = KMessageWidget::Warning;
            message = xi18nc("@info", "0 completed to-dos were purged.<nl/>")
                + xi18ncp("@info",
                          "%1 to-do was ignored because it has uncompleted or read-only children.",
                          "%1 to-dos were ignored because they have uncompleted or read-only children.",
                          numIgnored);
        } else if (numDeleted > 0 && numIgnored == 0) {
            message = i18ncp("@info", "%1 completed to-do was purged.", "%1 completed to-dos were purged.", numDeleted);
        } else if (numDeleted == 0 && numIgnored == 0) {
            message = i18nc("@info", "There are no completed to-dos to purge.");
        } else {
            type = KMessageWidget::Warning;
            message = i18ncp("@info", "%1 completed to-do was purged.", "%1 completed to-dos were purged.", numDeleted) + u'\n'
                + i18ncp("@info",
                         "%1 to-do was ignored because it has uncompleted or read-only children.",
                         "%1 to-dos were ignored because they have uncompleted or read-only children.",
                         numIgnored);
        }
    } else {
        message = xi18nc("@info", "An error occurred while purging completed to-dos: <message>%1</message>", mTodoPurger->lastError());
        type = KMessageWidget::Error;
    }

    showMessage(message, type);
}

void CalendarView::showMessage(const QString &message, KMessageWidget::MessageType type)
{
    mMessageWidget->setText(message);
    mMessageWidget->setMessageType(type);
    mMessageWidget->show();
}

Akonadi::Collection CalendarView::selectedCollection() const
{
    return mETMCollectionView ? mETMCollectionView->selectedCollection() : Akonadi::Collection();
}

Akonadi::Collection::List CalendarView::checkedCollections() const
{
    Akonadi::Collection::List collections;
    if (mETMCollectionView) {
        collections = mETMCollectionView->checkedCollections();
    }

    // If the default calendar is here, it should be first.
    int const count = collections.count();
    Akonadi::Collection::Id const id = CalendarSupport::KCalPrefs::instance()->defaultCalendarId();
    for (int i = 0; i < count; ++i) {
        if (id == collections[i].id()) {
            const Akonadi::Collection col = collections.takeAt(i);
            collections.insert(0, col);
            break;
        }
    }

    return collections;
}

void CalendarView::handleIncidenceCreated(const Akonadi::Item &item)
{
    Akonadi::Collection const collection = item.parentCollection();
    if (!collection.isValid()) {
        qCWarning(KORGANIZER_LOG) << "Item was creating in an invalid collection !? item id=" << item.id();
        return;
    }

    const bool collectionIsChecked = mETMCollectionView->isChecked(collection);

    if (!collectionIsChecked) {
        QString message;
        if (mETMCollectionView->isVisible()) {
            message = xi18nc("@info",
                             "You created an incidence in a calendar that is currently filtered out."
                             "<para>On the left sidebar, enable it in the calendar manager to see the incidence.</para>");
        } else {
            message = xi18nc("@info",
                             "You created an incidence in a calendar that is currently filtered out."
                             "<para>You can enable it through the calendar manager (Settings->Sidebar->Show Calendar Manager)</para>");
        }
        showMessage(message, KMessageWidget::Information);
    }
}

#include "moc_calendarview.cpp"
