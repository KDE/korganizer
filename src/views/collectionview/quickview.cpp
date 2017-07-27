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

#include <KCalCore/Event>
#include <KCalCore/FreeBusy>
#include <KCalCore/MemoryCalendar>

#include <EventViews/AgendaView>
#include <EventViews/ViewCalendar>

#include <CalendarSupport/FreeBusyCalendar>
#include <CalendarSupport/Utils>

#include <KCheckableProxyModel>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {}
    bool isValid(const KCalCore::Incidence::Ptr &incidence) const override
    {
        return isValid(incidence->uid());
    }

    bool isValid(const QString &incidenceIdentifier) const override
    {
        return incidenceIdentifier.startsWith(QStringLiteral("fb-"));
    }

    QString displayName(const KCalCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence);
        return i18n("Free/Busy calendar from %1", name);
    }

    QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const override
    {
        bool ok = false;
        int status = incidence->customProperty(QStringLiteral("FREEBUSY").toLatin1(), QStringLiteral("STATUS").toLatin1()).toInt(&ok);

        if (!ok) {
            return QColor(85, 85, 85);
        }

        switch (status) {
        case KCalCore::FreeBusyPeriod::Busy:
            return QColor(255, 0, 0);
        case KCalCore::FreeBusyPeriod::BusyTentative:
        case KCalCore::FreeBusyPeriod::BusyUnavailable:
            return QColor(255, 119, 0);
        case KCalCore::FreeBusyPeriod::Free:
            return QColor(0, 255, 0);
        default:
            return QColor(85, 85, 85);
        }
    }

    QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const override
    {
        Q_UNUSED(incidence);
        return QString();
    }

    KCalCore::Calendar::Ptr getCalendar() const override
    {
        return mCalendar;
    }

    KCalCore::Calendar::Ptr mCalendar;
    QString name;
};

Quickview::Quickview(const KPIM::Person &person, const Akonadi::Collection &col)
    : QDialog()
    , mUi(new Ui_quickview)
    , mPerson(person)
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

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);
    mUi->mAgendaBtn->hide();
    mUi->mMothBtn->hide();
    mUi->mWeekBtn->hide();
    mUi->mDayBtn->hide();
    //show fbcalendar for person in quickview
    if (!person.mail.isEmpty()) {
        CalendarSupport::FreeBusyItemModel *model = new CalendarSupport::FreeBusyItemModel(this);
        CalendarSupport::FreeBusyCalendar *fbCal = new CalendarSupport::FreeBusyCalendar(this);
        FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(person.name,  person.mail));
        CalendarSupport::FreeBusyItem::Ptr freebusy(new CalendarSupport::FreeBusyItem(attendee, this));

        fbCal->setModel(model);
        model->addItem(freebusy);
        fbCalendar->mCalendar = fbCal->calendar();
        fbCalendar->name = attendee->fullName();
        mAgendaView->addCalendar(EventViews::ViewCalendar::Ptr(fbCalendar));
    }

    if (mCollection.isValid()) {
        //create etm for mCollection
        Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder(this);
        Akonadi::ItemFetchScope scope;
        const QStringList allMimeTypes = { KCalCore::Event::eventMimeType(), KCalCore::Todo::todoMimeType(), KCalCore::Journal::journalMimeType()};

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

        setWindowTitle(i18nc("@title:window",
                             "%1",
                             CalendarSupport::displayName(calendar.data(), mCollection)));
    }
    mUi->calender->addWidget(mAgendaView);

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
