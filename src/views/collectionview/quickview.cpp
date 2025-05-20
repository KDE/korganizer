/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "quickview.h"
#include "korganizer_debug.h"
#include "ui_quickview.h"

#include <Akonadi/CalendarUtils>

#include <KCalendarCore/Event>

#include <EventViews/AgendaView>

#include <KCheckableProxyModel>
#include <KConfigGroup>
#include <KSharedConfig>

#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWindow>

namespace
{
const char myQuickviewConfigGroupName[] = "Quickview";
}

Quickview::Quickview(const Akonadi::CollectionCalendar::Ptr &calendar, const QString &title)
    : mUi(new Ui_quickview)
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

    mAgendaView->addCalendar(calendar);

    mUi->calendar->addWidget(mAgendaView);

    setWindowTitle(title);

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
    create(); // ensure a window is created
    windowHandle()->resize(QSize(775, 600));
    KConfigGroup const group(KSharedConfig::openStateConfig(), QLatin1StringView(myQuickviewConfigGroupName));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584

    const QList<int> sizes = group.readEntry("Separator", QList<int>());

    // don't allow invalid/corrupted settings or else agenda becomes invisible
    if (sizes.count() >= 2 && !sizes.contains(0)) {
        mAgendaView->splitter()->setSizes(sizes);
    }
}

void Quickview::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myQuickviewConfigGroupName));
    KWindowConfig::saveWindowSize(windowHandle(), group);
    const QList<int> list = mAgendaView->splitter()->sizes();
    group.writeEntry("Separator", list);

    group.sync();
}

#include "moc_quickview.cpp"
