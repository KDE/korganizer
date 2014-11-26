/*
  This file is part of KOrganizer.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

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

#include "monthview.h"
#include "koeventpopupmenu.h"
#include "koprefs.h"

#include <calendarviews/month/monthview.h>

#include <QVBoxLayout>

using namespace KOrg;

class MonthView::Private
{
  public:
    Private( MonthView *qq ) : q( qq )
    {
      QVBoxLayout *layout = new QVBoxLayout( q );
      layout->setMargin( 0 );
      mMonthView = new EventViews::MonthView( EventViews::MonthView::Visible, q );
      mMonthView->setPreferences( KOPrefs::instance()->eventViewsPreferences() );
      layout->addWidget( mMonthView );
      mPopup = q->eventPopup();
    }

    EventViews::MonthView *mMonthView;
    KOEventPopupMenu *mPopup;

  private:
    MonthView *q;
};

MonthView::MonthView( QWidget *parent )
  : KOEventView( parent ), d( new Private( this ) )
{
  connect( d->mMonthView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           d->mPopup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( d->mMonthView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( d->mMonthView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( d->mMonthView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );

  connect( d->mMonthView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->mMonthView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( d->mMonthView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)) );

  connect( d->mMonthView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)) );

  connect( d->mMonthView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( d->mMonthView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( d->mMonthView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( d->mMonthView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( d->mMonthView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( d->mMonthView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( d->mMonthView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( d->mMonthView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );

  connect( d->mMonthView, SIGNAL(fullViewChanged(bool)),
           SIGNAL(fullViewChanged(bool)) );
}

MonthView::~MonthView()
{
  delete d;
}

CalendarSupport::CalPrinterBase::PrintType MonthView::printType() const
{
  return CalendarSupport::CalPrinterBase::Month;
}

int MonthView::currentDateCount() const
{
  return d->mMonthView->currentDateCount();
}

int MonthView::currentMonth() const
{
  return d->mMonthView->currentMonth();
}

KCalCore::DateList MonthView::selectedIncidenceDates()
{
  return d->mMonthView->selectedIncidenceDates();
}

QDateTime MonthView::selectionStart()
{
  return d->mMonthView->selectionStart();
}

QDateTime MonthView::selectionEnd()
{
  return d->mMonthView->selectionEnd();
}

bool MonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  return d->mMonthView->eventDurationHint( startDt, endDt, allDay );
}

QDate MonthView::averageDate() const
{
  return d->mMonthView->averageDate();
}

bool MonthView::usesFullWindow()
{
  return d->mMonthView->usesFullWindow();
}

bool MonthView::supportsDateRangeSelection()
{
  return d->mMonthView->supportsDateRangeSelection();
}

void MonthView::updateView()
{
  d->mMonthView->updateView();
}

void MonthView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  d->mMonthView->showIncidences( incidenceList, date );
}

void MonthView::changeIncidenceDisplay( const Akonadi::Item &item,
                                        Akonadi::IncidenceChanger::ChangeType changeType )
{
  d->mMonthView->changeIncidenceDisplay( item, changeType );
}

void MonthView::updateConfig()
{
  d->mMonthView->updateConfig();
}

int MonthView::maxDatesHint() const
{
  return 6 * 7;
}

Akonadi::Item::List MonthView::selectedIncidences()
{
  return d->mMonthView->selectedIncidences();
}

void MonthView::setTypeAheadReceiver( QObject *o )
{
  d->mMonthView->setTypeAheadReceiver( o );
}

void MonthView::setDateRange( const KDateTime &start, const KDateTime &end,
                              const QDate &preferredMonth )
{
  d->mMonthView->setDateRange( start, end, preferredMonth );
}

void MonthView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  KOEventView::setCalendar( cal );
  d->mPopup->setCalendar( cal );
  d->mMonthView->setCalendar( cal );
}

void MonthView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  d->mMonthView->setIncidenceChanger( changer );
}

void MonthView::showDates( const QDate &start, const QDate &end, const QDate &preferredMonth )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( preferredMonth );
}

