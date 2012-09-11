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
#include "journalview.h"
#include "koprefs.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/utils.h>

#include <KVBox>

#include <QEvent>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace KOrg;

KOJournalView::KOJournalView( QWidget *parent )
  : KOrg::BaseView( parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );
  mSA = new QScrollArea( this );
  mVBox = new KVBox( mSA->viewport() );
  mSA->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mSA->setWidgetResizable ( true );
  mSA->setWidget( mVBox );
  topLayout->addWidget( mSA );

  installEventFilter( this );
}

KOJournalView::~KOJournalView()
{
}

void KOJournalView::appendJournal( const Akonadi::Item &journal, const QDate &dt )
{
  JournalDateView *entry = 0;
  if ( mEntries.contains( dt ) ) {
    entry = mEntries[dt];
  } else {
    entry = new JournalDateView( calendar(), mVBox );
    entry->setDate( dt );
    entry->setIncidenceChanger( mChanger );
    entry->show();
    connect( this, SIGNAL(flushEntries()), entry, SIGNAL(flushEntries()) );
    connect( this, SIGNAL(setIncidenceChangerSignal(CalendarSupport::IncidenceChanger*)),
             entry, SLOT(setIncidenceChanger(CalendarSupport::IncidenceChanger*)) );
    connect( this, SIGNAL(journalEdited(Akonadi::Item)),
             entry, SLOT(journalEdited(Akonadi::Item)) );
    connect( this, SIGNAL(journalDeleted(Akonadi::Item)),
             entry, SLOT(journalDeleted(Akonadi::Item)) );

    connect( entry, SIGNAL(editIncidence(Akonadi::Item)),
             this, SIGNAL(editIncidenceSignal(Akonadi::Item)) );
    connect( entry, SIGNAL(deleteIncidence(Akonadi::Item)),
             this, SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );
    connect( entry, SIGNAL(newJournal(QDate)),
             this, SIGNAL(newJournalSignal(QDate)) );
    connect( entry, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
                    SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );
    mEntries.insert( dt, entry );
  }

  if ( entry && CalendarSupport::hasJournal( journal ) ) {
    entry->addJournal( journal );
  }
}

int KOJournalView::currentDateCount() const
{
  return mEntries.size();
}

Akonadi::Item::List KOJournalView::selectedIncidences()
{
  // We don't have a selection in the journal view.
  // FIXME: The currently edited journal is the selected incidence...
  Akonadi::Item::List eventList;
  return eventList;
}

void KOJournalView::clearEntries()
{
  kDebug(5850)<<"KOJournalView::clearEntries()";
  QMap<QDate, JournalDateView*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    delete it.value();
  }
  mEntries.clear();
}
void KOJournalView::updateView()
{
  QMap<QDate, JournalDateView*>::Iterator it = mEntries.end();
  while ( it != mEntries.begin() ) {
    --it;
    it.value()->clear();
    const Akonadi::Item::List journals = calendar()->journals( it.key() );
    kDebug() << "updateview found" << journals.count();
    Q_FOREACH ( const Akonadi::Item &i, journals ) {
      it.value()->addJournal( i );
    }
  }
}

void KOJournalView::flushView()
{
  emit flushEntries();
}

void KOJournalView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  clearEntries();
  if ( end<start ) {
    kWarning() << "End is smaller than start. end=" << end << "; start=" << start;
    return;
  }

  Akonadi::Item::List::ConstIterator it;
  Akonadi::Item::List jnls;
  for ( QDate d=end; d>=start; d=d.addDays(-1) ) {
    jnls = calendar()->journals( d );
    kDebug() << "Found" << jnls.count() << "journals on date" << d;
    it = jnls.constEnd();
    while ( it != jnls.constBegin() ) {
      --it;
      appendJournal( *it, d );
    }
    if ( jnls.isEmpty() ) {
      // create an empty dateentry widget
      //updateView();
      kDebug() << "Appended null journal";
      appendJournal( Akonadi::Item(), d );
    }
  }
}

void KOJournalView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  Q_UNUSED( date );
  clearEntries();
  Q_FOREACH ( const Akonadi::Item &i, incidences ) {
    if ( const KCalCore::Journal::Ptr j = CalendarSupport::journal( i ) ) {
      appendJournal( i, j->dtStart().date() );
    }
  }
}

void KOJournalView::changeIncidenceDisplay( const Akonadi::Item &incidence, int action )
{
  if ( KCalCore::Journal::Ptr journal = CalendarSupport::journal( incidence ) ) {
    switch ( action ) {
    case CalendarSupport::IncidenceChanger::INCIDENCEADDED:
      appendJournal( incidence, journal->dtStart().date() );
      break;
    case CalendarSupport::IncidenceChanger::INCIDENCEEDITED:
      emit journalEdited( incidence );
      break;
    case CalendarSupport::IncidenceChanger::INCIDENCEDELETED:
      emit journalDeleted( incidence );
      break;
    default:
      kWarning() << "Illegal action" << action;
    }
  }
}

void KOJournalView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void KOJournalView::newJournal()
{
  emit newJournalSignal( QDate::currentDate() );
}

void KOJournalView::getHighlightMode( bool &highlightEvents,
                                      bool &highlightTodos,
                                      bool &highlightJournals )
{
  highlightJournals = KOPrefs::instance()->mHighlightJournals;
  highlightTodos    = false;
  highlightEvents   = !highlightJournals;
}

bool KOJournalView::eventFilter ( QObject *object, QEvent *event )
{
  Q_UNUSED( object );
  switch ( event->type() ) {
  case QEvent::MouseButtonDblClick:
    emit newJournalSignal( QDate() );
    return true;
  default:
    return false;
  }
}

CalPrinterBase::PrintType KOJournalView::printType() const
{
  return CalPrinterBase::Journallist;
}

#include "kojournalview.moc"
