/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

// View of Journal entries

#include "kojournalview.h"
#include "koprefs.h"

#include <calendarviews/journal/journalview.h>

#include <calendarsupport/utils.h>
#include <calendarsupport/printing/calprinter.h>
#include <calendarsupport/printing/calprintdefaultplugins.h>

#include <Akonadi/Calendar/ETMCalendar>


#include <QVBoxLayout>

using namespace KOrg;

KOJournalView::KOJournalView( QWidget *parent )
  : KOrg::BaseView( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mJournalView = new EventViews::JournalView( this );

  layout->addWidget( mJournalView );

  connect( mJournalView, SIGNAL(printJournal(KCalCore::Journal::Ptr)),
           SLOT(printJournal(KCalCore::Journal::Ptr)) );

  connect( mJournalView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( mJournalView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( mJournalView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( mJournalView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( mJournalView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( mJournalView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( mJournalView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( mJournalView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( mJournalView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( mJournalView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( mJournalView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( mJournalView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );
}

KOJournalView::~KOJournalView()
{
}

int KOJournalView::currentDateCount() const
{
  return mJournalView->currentDateCount();
}

Akonadi::Item::List KOJournalView::selectedIncidences()
{
  return mJournalView->selectedIncidences();
}

void KOJournalView::updateView()
{
  mJournalView->updateView();
}

void KOJournalView::flushView()
{
  mJournalView->flushView();
}

void KOJournalView::showDates( const QDate &start, const QDate &end, const QDate &dummy )
{
  mJournalView->showDates( start, end, dummy );
}

void KOJournalView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  mJournalView->showIncidences( incidences, date );
}

void KOJournalView::changeIncidenceDisplay( const Akonadi::Item &incidence,
                                            Akonadi::IncidenceChanger::ChangeType changeType )
{
  mJournalView->changeIncidenceDisplay( incidence, changeType );
}

void KOJournalView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  mJournalView->setIncidenceChanger( changer );
}

void KOJournalView::getHighlightMode( bool &highlightEvents,
                                      bool &highlightTodos,
                                      bool &highlightJournals )
{
  highlightJournals = KOPrefs::instance()->mHighlightJournals;
  highlightTodos    = false;
  highlightEvents   = !highlightJournals;
}

CalendarSupport::CalPrinterBase::PrintType KOJournalView::printType() const
{
  return CalendarSupport::CalPrinterBase::Journallist;
}

void KOJournalView::setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
{
  BaseView::setCalendar( calendar );
  mJournalView->setCalendar( calendar );
}

void KOJournalView::printJournal( const KCalCore::Journal::Ptr &journal )
{
  if ( journal ) {
    CalendarSupport::CalPrinter printer( this, calendar(), true );
    KCalCore::Incidence::List selectedIncidences;
    selectedIncidences.append( journal );

    const QDate dtStart = journal->dtStart().date();

    //make sure to clear and then restore the view stylesheet, else the view
    //stylesheet is propagated to the child print dialog. see bug 303902
    const QString ss = styleSheet();
    setStyleSheet( QString() );
    printer.print( CalendarSupport::CalPrinterBase::Incidence,
                   dtStart, dtStart, selectedIncidences );
    setStyleSheet( ss );
  }
}

