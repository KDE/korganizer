/*
  This file is part of KOrganizer.
  Copyright (c) 2007 Till Adam <adam@kde.org>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (c) 2010 Sérgio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotimelineview.h"
#include "koeventpopupmenu.h"

#include <calendarviews/timeline/timelineview.h>

#include <QVBoxLayout>

class KOTimelineView::Private
{
  public:
    Private( KOTimelineView *q ) : mEventPopup( 0 ), mParent( q )
    {
      QVBoxLayout *vbox = new QVBoxLayout( mParent );
      vbox->setMargin( 0 );
      mTimeLineView = new EventViews::TimelineView( mParent );
      vbox->addWidget( mTimeLineView );
      mEventPopup = q->eventPopup();
    }
    ~Private()
    {
      delete mEventPopup;
    }
    KOEventPopupMenu *mEventPopup;
    EventViews::TimelineView *mTimeLineView;

  private:
    KOTimelineView *mParent;
};

KOTimelineView::KOTimelineView( QWidget *parent )
  : KOEventView( parent ), d( new Private( this ) )
{
  connect( d->mTimeLineView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           d->mEventPopup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( d->mTimeLineView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( d->mTimeLineView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( d->mTimeLineView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );

  connect( d->mTimeLineView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->mTimeLineView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( d->mTimeLineView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mTimeLineView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mTimeLineView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( d->mTimeLineView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( d->mTimeLineView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( d->mTimeLineView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( d->mTimeLineView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( d->mTimeLineView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( d->mTimeLineView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( d->mTimeLineView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );
}

KOTimelineView::~KOTimelineView()
{
  delete d;
}

/*virtual*/
Akonadi::Item::List KOTimelineView::selectedIncidences()
{
  return d->mTimeLineView->selectedIncidences();
}

/*virtual*/
KCalCore::DateList KOTimelineView::selectedIncidenceDates()
{
  return d->mTimeLineView->selectedIncidenceDates();
}

/*virtual*/
int KOTimelineView::currentDateCount() const
{
  return d->mTimeLineView->currentDateCount();
}

/*virtual*/
void KOTimelineView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  d->mTimeLineView->showDates( start, end );
}

/*virtual*/
void KOTimelineView::showIncidences( const Akonadi::Item::List &incidenceList,
                                     const QDate &date )
{
  d->mTimeLineView->showIncidences( incidenceList, date );
}

/*virtual*/
void KOTimelineView::updateView()
{
  d->mTimeLineView->updateView();
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay( const Akonadi::Item &incidence,
                                             Akonadi::IncidenceChanger::ChangeType changeType )
{
  d->mTimeLineView->changeIncidenceDisplay( incidence, changeType );
}

bool KOTimelineView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                        bool &allDay )
{
  return d->mTimeLineView->eventDurationHint( startDt, endDt, allDay );
}

CalendarSupport::CalPrinterBase::PrintType KOTimelineView::printType() const
{
  // If up to three days are selected, use day style, otherwise week
  if ( currentDateCount() <= 3 ) {
    return CalendarSupport::CalPrinterBase::Day;
  } else {
    return CalendarSupport::CalPrinterBase::Week;
  }
}

void KOTimelineView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  KOEventView::setCalendar( cal );
  d->mEventPopup->setCalendar( cal );
  d->mTimeLineView->setCalendar( cal );
}

void KOTimelineView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  d->mTimeLineView->setIncidenceChanger( changer );
}

