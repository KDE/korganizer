/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "quickview.h"
#include "ui_quickview.h"
#include "korganizer_debug.h"

#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/itemfetchscope.h>

#include <KCalendarCore/Event>
#include <KCalendarCore/MemoryCalendar>

#include <EventViews/AgendaView>
#include <EventViews/ViewCalendar>
#include <CalendarSupport/Utils>

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
    , mDayRange(7)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Quickview::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Quickview::reject);
    QWidget *w = new QWidget(this);
    mUi->setupUi(w);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false, false);
    mUi->mAgendaBtn->hide();
    mUi->mMothBtn->hide();
    mUi->mWeekBtn->hide();
    mUi->mDayBtn->hide();

    //create etm for mCollection
    Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder(this);
    Akonadi::ItemFetchScope scope;
    const QStringList allMimeTypes
        = { KCalendarCore::Event::eventMimeType(), KCalendarCore::Todo::todoMimeType(),
            KCalendarCore::Journal::journalMimeType()};

    scope.fetchFullPayload(true);
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

    monitor->setCollectionMonitored(mCollection);
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setAllMonitored(true);

    for (const QString &mimetype : allMimeTypes) {
        monitor->setMimeTypeMonitored(mimetype, true);
    }

    Akonadi::ETMCalendar::Ptr calendar
        = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(monitor));

    calendar->setCollectionFilteringEnabled(false);
    mAgendaView->setCalendar(calendar);
    setWindowTitle(i18nc("@title:window", "%1", CalendarSupport::displayName(calendar.data(), mCollection)));

    mUi->calendar->addWidget(mAgendaView);

    setWindowTitle(i18nc("@title:window",
                         "%1",
                         CalendarSupport::displayName(calendar.data(), mCollection)));

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

    QList<int> list = mAgendaView->splitter()->sizes();
    group.writeEntry("Separator", list);

    group.sync();
}
