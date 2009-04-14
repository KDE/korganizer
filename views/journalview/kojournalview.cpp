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
#include "koglobals.h"
#include "koprefs.h"

#include <kcal/calendar.h>

#include <KDebug>
#include <KLocale>
#include <KVBox>

#include <QScrollArea>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace KOrg;

KOJournalView::KOJournalView( Calendar *calendar, QWidget *parent )
  : KOrg::BaseView( calendar, parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  mSA = new QScrollArea( this );
  mVBox = new KVBox( mSA->viewport() );
  mSA->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mSA->setWidgetResizable ( true );
  mSA->setWidget( mVBox );
  topLayout->addWidget( mSA );
}

KOJournalView::~KOJournalView()
{
}

void KOJournalView::appendJournal( Journal *journal, const QDate &dt )
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
    connect( this, SIGNAL(setIncidenceChangerSignal(IncidenceChangerBase *)),
             entry, SLOT(setIncidenceChanger(IncidenceChangerBase *)) );
    connect( this, SIGNAL(journalEdited(Journal *)),
             entry, SLOT(journalEdited(Journal *)) );
    connect( this, SIGNAL(journalDeleted(Journal *)),
             entry, SLOT(journalDeleted(Journal *)) );

    connect( entry, SIGNAL(editIncidence(Incidence *)),
             this, SIGNAL(editIncidenceSignal(Incidence *)) );
    connect( entry, SIGNAL(deleteIncidence(Incidence *)),
             this, SIGNAL(deleteIncidenceSignal(Incidence *)) );
    connect( entry, SIGNAL(newJournal(const QDate &)),
             this, SIGNAL(newJournalSignal(const QDate &)) );
    mEntries.insert( dt, entry );
  }

  if ( entry && journal ) {
    entry->addJournal( journal );
  }
}

int KOJournalView::currentDateCount()
{
  return mEntries.size();
}

Incidence::List KOJournalView::selectedIncidences()
{
  // We don't have a selection in the journal view.
  // FIXME: The currently edited journal is the selected incidence...
  Incidence::List eventList;
  return eventList;
}

void KOJournalView::clearEntries()
{
//  kDebug(5850)<<"KOJournalView::clearEntries()";
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
    Journal::List journals = calendar()->journals( it.key() );
    Journal::List::Iterator it1;
    for ( it1 = journals.begin(); it1 != journals.end(); ++it1 ) {
      it.value()->addJournal( *it1 );
    }
  }
}

void KOJournalView::flushView()
{
  emit flushEntries();
}

void KOJournalView::showDates( const QDate &start, const QDate &end )
{
  clearEntries();
  if ( end<start ) {
    return;
  }

  Journal::List::ConstIterator it;
  Journal::List jnls;
  QDate d = start;
  for ( QDate d=end; d>=start; d=d.addDays(-1) ) {
    jnls = calendar()->journals( d );
    it = jnls.constEnd();
    while ( it != jnls.constBegin() ) {
      --it;
      appendJournal( *it, d );
    }
    if ( jnls.count() < 1 ) {
      // create an empty dateentry widget
      //updateView();
      appendJournal( 0, d );
    }
  }
}

void KOJournalView::showIncidences( const Incidence::List &incidences )
{
  clearEntries();
  Incidence::List::const_iterator it = incidences.constEnd();
  while ( it != incidences.constBegin() ) {
    --it;
    if ((*it) && ( (*it)->type() == "Journal" ) ) {
      Journal *j = static_cast<Journal *>(*it);
      if ( j ) {
        appendJournal( j, j->dtStart().date() );
      }
    }
  }
}

void KOJournalView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  Journal *journal = dynamic_cast<Journal *>(incidence);
  if ( journal ) {
    switch(action) {
    case KOGlobals::INCIDENCEADDED:
      appendJournal( journal, journal->dtStart().date() );
      break;
    case KOGlobals::INCIDENCEEDITED:
      emit journalEdited( journal );
      break;
    case KOGlobals::INCIDENCEDELETED:
      emit journalDeleted( journal );
      break;
    default:
      kWarning() << "Illegal action" << action;
    }
  }
}

void KOJournalView::setIncidenceChanger( IncidenceChangerBase *changer )
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

#include "kojournalview.moc"
