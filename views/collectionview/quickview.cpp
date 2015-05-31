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

#include <KCalCore/Event>
#include <KCalCore/FreeBusy>
#include <KCalCore/MemoryCalendar>
#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/itemfetchscope.h>

#include <calendarviews/agenda/agendaview.h>
#include <calendarviews/agenda/viewcalendar.h>
#include <calendarsupport/calendarsingleton.h>

#include <freebusymodel/freebusycalendar.h>

#include "korganizer_debug.h"
#include <KCheckableProxyModel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {};
    virtual bool isValid(const KCalCore::Incidence::Ptr &incidence) const
    {
        return isValid(incidence->uid());
    }

    virtual bool isValid(const QString &incidenceIdentifier) const
    {
        return incidenceIdentifier.startsWith(QStringLiteral("fb-"));
    }

    virtual QString displayName(const KCalCore::Incidence::Ptr &incidence) const
    {
        Q_UNUSED(incidence);
        return i18n("Freebusycalendar from %1", name);
    }

    virtual QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const
    {
        bool ok = false;
        int status = incidence->customProperty(QStringLiteral("FREEBUSY").toLatin1(), QStringLiteral("STATUS").toLatin1()).toInt(&ok);

        if (!ok) {
            return QColor(QStringLiteral("#555"));
        }

        switch (status) {
        case KCalCore::FreeBusyPeriod::Busy:
            return QColor(QStringLiteral("#f00"));
        case KCalCore::FreeBusyPeriod::BusyTentative:
        case KCalCore::FreeBusyPeriod::BusyUnavailable:
            return QColor(QStringLiteral("#f70"));
        case KCalCore::FreeBusyPeriod::Free:
            return QColor(QStringLiteral("#0f0"));
        default:
            return QColor(QStringLiteral("#555"));
        }
    }

    virtual QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
    {
        return QString();
    }

    virtual KCalCore::Calendar::Ptr getCalendar() const
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
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Quickview::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Quickview::reject);
    mainLayout->addWidget(buttonBox);
    QWidget *w = new QWidget(this);
    mUi->setupUi(w);
    mainLayout->addWidget(w);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);

    //show fbcalendar for person in quickview
    if (!person.mail.isEmpty()) {
        KPIM::FreeBusyItemModel *model = new KPIM::FreeBusyItemModel(this);
        KPIM::FreeBusyCalendar *fbCal = new KPIM::FreeBusyCalendar(this);
        FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(person.name,  person.mail));
        KPIM::FreeBusyItem::Ptr freebusy(new KPIM::FreeBusyItem(attendee, this));

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
        QStringList allMimeTypes;

        allMimeTypes << KCalCore::Event::eventMimeType() << KCalCore::Todo::todoMimeType()
                     << KCalCore::Journal::journalMimeType();

        scope.fetchFullPayload(true);
        scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

        monitor->setCollectionMonitored(mCollection);
        monitor->fetchCollection(true);
        monitor->setItemFetchScope(scope);
        monitor->setAllMonitored(true);

        foreach (const QString &mimetype, allMimeTypes) {
            monitor->setMimeTypeMonitored(mimetype, true);
        }

        Akonadi::ETMCalendar::Ptr calendar = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(monitor));

        calendar->setCollectionFilteringEnabled(false);
        mAgendaView->setCalendar(calendar);
    }
    mUi->calender->addWidget(mAgendaView);

    connect(mUi->mTodayBtn, &QPushButton::clicked, this, &Quickview::onTodayClicked);
    connect(mUi->mNextBtn, &QPushButton::clicked, this, &Quickview::onNextClicked);
    connect(mUi->mPreviousBtn, &QPushButton::clicked, this, &Quickview::onPreviousClicked);

    onTodayClicked();
}

Quickview::~Quickview()
{
    delete mUi;
}

void Quickview::onNextClicked()
{
    QDate start = mAgendaView->startDate().addDays(mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

void Quickview::onPreviousClicked()
{
    QDate start = mAgendaView->startDate().addDays(-mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

void Quickview::onTodayClicked()
{
    QDate start = QDate::currentDate();
    start = start.addDays(-QDate::currentDate().dayOfWeek() + 1);
    mAgendaView->showDates(start, start.addDays(mDayRange - 1));
}

