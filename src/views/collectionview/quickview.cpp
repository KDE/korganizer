/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "quickview.h"
#include "korganizer_debug.h"
#include "ui_quickview.h"

#include <Akonadi/CalendarUtils>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KCalendarCore/Event>
#include <KCalendarCore/MemoryCalendar>

#include <EventViews/AgendaView>

#include <KCheckableProxyModel>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

Quickview::Quickview(const Akonadi::Collection &col)
    : QDialog()
    , mUi(new Ui_quickview)
    , mCollection(col)
{
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Quickview::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Quickview::reject);
    auto w = new QWidget(this);
    mUi->setupUi(w);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false, false);
    mUi->mAgendaBtn->hide();
    mUi->mMothBtn->hide();
    mUi->mWeekBtn->hide();
    mUi->mDayBtn->hide();

    // create etm for mCollection
    auto monitor = new Akonadi::ChangeRecorder(this);
    Akonadi::ItemFetchScope scope;
    const QStringList allMimeTypes = {KCalendarCore::Event::eventMimeType(), KCalendarCore::Todo::todoMimeType(), KCalendarCore::Journal::journalMimeType()};

    scope.fetchFullPayload(true);
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

    monitor->setCollectionMonitored(mCollection);
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setAllMonitored(true);

    for (const QString &mimetype : allMimeTypes) {
        monitor->setMimeTypeMonitored(mimetype, true);
    }

    Akonadi::ETMCalendar::Ptr calendar = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(monitor));

    calendar->setCollectionFilteringEnabled(false);
    mAgendaView->setCalendar(calendar);
    setWindowTitle(i18nc("@title:window", "%1", Akonadi::CalendarUtils::displayName(calendar.data(), mCollection)));

    mUi->calendar->addWidget(mAgendaView);

    setWindowTitle(i18nc("@title:window", "%1", Akonadi::CalendarUtils::displayName(calendar.data(), mCollection)));

    connect(mUi->mTodayBtn, &QPushButton::clicked, this, &Quickview::onTodayClicked);
    connect(mUi->mNextBtn, &QPushButton::clicked, this, &Quickview::onNextClicked);
    connect(mUi->mPreviousBtn, &QPushButton::clicked, this, &Quickview::onPreviousClicked);

    onTodayClicked();

    readConfig();
}

Quickview::~Quickview()
{
    writeConfig();
    delete mUi;
}

void Quickview::onNextClicked()
{
    const QDate start = mAgendaView->startDate().addDays(mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

void Quickview::onPreviousClicked()
{
    const QDate start = mAgendaView->startDate().addDays(-mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

void Quickview::onTodayClicked()
{
    QDate start = QDate::currentDate();
    start = start.addDays(-QDate::currentDate().dayOfWeek() + 1);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

void Quickview::readConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("Quickview"));

    const QSize size = group.readEntry("Size", QSize(775, 600));
    if (size.isValid()) {
        resize(size);
    }

    const QList<int> sizes = group.readEntry("Separator", QList<int>());

    // don't allow invalid/corrupted settings or else agenda becomes invisible
    if (sizes.count() >= 2 && !sizes.contains(0)) {
        mAgendaView->splitter()->setSizes(sizes);
    }
}

void Quickview::writeConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("Quickview"));

    group.writeEntry("Size", size());

    const QList<int> list = mAgendaView->splitter()->sizes();
    group.writeEntry("Separator", list);

    group.sync();
}
