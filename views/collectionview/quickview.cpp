/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//http://stackoverflow.com/questions/18831242/qt-start-editing-of-cell-after-one-click

#include "quickview.h"
#include "ui_quickview.h"

#include <KCalCore/Event>
#include <KCalCore/FreeBusy>
#include <KCalCore/MemoryCalendar>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>

#include <calendarviews/agenda/agendaview.h>
#include <calendarviews/agenda/viewcalendar.h>
#include <calendarsupport/calendarsingleton.h>

#include <freebusymodel/freebusycalendar.h>

#include <KDebug>
#include <KSystemTimeZones>
#include <KCheckableProxyModel>

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {};
    virtual bool isValid(const KCalCore::Incidence::Ptr &incidence) const
    {
        return incidence->uid().startsWith(QLatin1String("fb-"));
    }

    virtual QString displayName(const KCalCore::Incidence::Ptr &incidence) const
    {
        Q_UNUSED(incidence);
        return i18n("Freebusycalendar from %1").arg(name);
    }

    virtual QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const
    {
        bool ok = false;
        int status = incidence->customProperty("FREEBUSY", "STATUS").toInt(&ok);

        if (!ok) {
            return QColor("#555");
        }

        switch (status) {
        case KCalCore::FreeBusyPeriod::Busy:
            return QColor("#f00");
        case KCalCore::FreeBusyPeriod::BusyTentative:
        case KCalCore::FreeBusyPeriod::BusyUnavailable:
            return QColor("#f70");
        case KCalCore::FreeBusyPeriod::Free:
            return QColor("#0f0");
        default:
            return QColor("#555");
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

Quickview::Quickview(const Person &person, const Akonadi::Collection &col)
    : KDialog()
    , mUi(new Ui_quickview)
    , mPerson(person)
    , mCollection(col)
    , mDayRange(7)
{
    QWidget *w = new QWidget( this );
    mUi->setupUi( w );
    setMainWidget( w );

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);

    //show fbcalendar for person in quickview
    if (!person.mail.isEmpty()) {
        FreeBusyItemModel *model = new FreeBusyItemModel(this);
        FreeBusyCalendar *fbCal = new FreeBusyCalendar(this);
        FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(person.name,  person.mail));
        FreeBusyItem::Ptr freebusy( new FreeBusyItem( attendee, this ));

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

        foreach(const QString &mimetype, allMimeTypes) {
            monitor->setMimeTypeMonitored(mimetype, true);
        }

        Akonadi::ETMCalendar::Ptr calendar = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(monitor));

        calendar->setCollectionFilteringEnabled(false);
        mAgendaView->setCalendar(calendar);
    }
    mUi->calender->addWidget( mAgendaView );

    connect(mUi->mTodayBtn, SIGNAL(clicked(bool)), SLOT(onTodayClicked()));
    connect(mUi->mNextBtn, SIGNAL(clicked(bool)), SLOT(onNextClicked()));
    connect(mUi->mPreviousBtn, SIGNAL(clicked(bool)), SLOT(onPreviousClicked()));

    onTodayClicked();
}

Quickview::~Quickview()
{
    delete mUi;
}

void Quickview::onNextClicked()
{
    QDate start = mAgendaView->startDate().addDays(mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange-1));
}

void Quickview::onPreviousClicked()
{
    QDate start = mAgendaView->startDate().addDays(-mDayRange);
    mAgendaView->showDates(start, start.addDays(mDayRange-1));
}

void Quickview::onTodayClicked()
{
    QDate start = QDate::currentDate();
    start = start.addDays(-QDate::currentDate().dayOfWeek()+1);
    mAgendaView->showDates(start, start.addDays(mDayRange-1));
}


#include "quickview.moc"
