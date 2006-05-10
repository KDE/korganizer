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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

//
// View of Journal entries

#include <QLayout>
#include <q3popupmenu.h>

#include <QLabel>
#include <q3scrollview.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>
#include <kvbox.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "koglobals.h"
using namespace KOrg;

KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent )
  : KOrg::BaseView( calendar, parent )
{
  QVBoxLayout*topLayout = new QVBoxLayout( this );
  topLayout->setAutoAdd(true);
  mSV = new Q3ScrollView( this, "JournalScrollView" );
  topLayout = new QVBoxLayout( mSV->viewport() );
  topLayout->setAutoAdd(true);
  mVBox = new KVBox( mSV->viewport() );
  mSV->setVScrollBarMode( Q3ScrollView::Auto );
  mSV->setHScrollBarMode( Q3ScrollView::AlwaysOff );
//  mVBox->setSpacing( 10 );
}

KOJournalView::~KOJournalView()
{
}

void KOJournalView::appendJournal( Journal*journal, const QDate &dt)
{
  JournalDateEntry *entry = 0;
  if ( mEntries.contains( dt ) ) {
    entry = mEntries[dt];
  } else {
    entry = new JournalDateEntry( calendar(), mVBox );
    entry->setDate( dt );
    entry->setIncidenceChanger( mChanger );
    entry->show();
    connect( this, SIGNAL(flushEntries()), entry, SIGNAL(flushEntries()) );
    connect( this, SIGNAL(setIncidenceChangerSignal( IncidenceChangerBase * ) ),
             entry, SLOT(setIncidenceChanger( IncidenceChangerBase * ) ) );
    connect( this, SIGNAL( journalEdited( Journal* ) ),
             entry, SLOT( journalEdited( Journal* ) ) );
    connect( this, SIGNAL( journalDeleted( Journal* ) ),
             entry, SLOT( journalDeleted( Journal* ) ) );
    
    connect( entry, SIGNAL( editIncidence( Incidence* ) ),
             this, SIGNAL( editIncidenceSignal( Incidence* ) ) );
    connect( entry, SIGNAL( deleteIncidence( Incidence* ) ),
             this, SIGNAL( deleteIncidenceSignal( Incidence* ) ) );
    connect( entry, SIGNAL( newJournal( const QDate & ) ),
             this, SIGNAL( newJournalSignal( const QDate & ) ) );
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
//  kDebug(5850)<<"KOJournalView::clearEntries()"<<endl;
  QMap<QDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    delete (it.value());
  }
  mEntries.clear();
}
void KOJournalView::updateView()
{
  QMap<QDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
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
//  kDebug(5850) << "KOJournalView::flushView(): "<< endl;
  emit flushEntries();
}

void KOJournalView::showDates(const QDate &start, const QDate &end)
{
//  kDebug(5850) << "KOJournalView::showDates(): "<<start.toString().toLatin1()<<" - "<<end.toString().toLatin1() << endl;
  clearEntries();
  if ( end<start ) return;

  Journal::List::ConstIterator it;
  Journal::List jnls;
  QDate d=start;
  for ( QDate d=start; d<=end; d=d.addDays(1) ) {
    jnls = calendar()->journals( d );
    for ( it = jnls.begin(); it != jnls.end(); ++it ) {
      appendJournal( *it, d );
    }
    if ( jnls.count() < 1 ) {
      // create an empty dateentry widget
      appendJournal( 0, d );
    }
  }
}

void KOJournalView::showIncidences( const Incidence::List &incidences )
{
//  kDebug(5850) << "KOJournalView::showIncidences(): "<< endl;
  clearEntries();
  Incidence::List::const_iterator it;
  for ( it=incidences.constBegin(); it!=incidences.constEnd(); ++it) {
    if ((*it) && ( (*it)->type() == QLatin1String("Journal") ) ) {
      Journal*j = static_cast<Journal*>(*it);
      if ( j ) appendJournal( j, j->dtStart().date() );
    }
  }
}

void KOJournalView::changeIncidenceDisplay(Incidence *incidence, int action)
{
//  kDebug(5850) << "KOJournalView::changeIncidenceDisplay(): "<< endl;
  Journal *journal = dynamic_cast<Journal*>(incidence);
  if (journal) {
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
        kDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
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

#include "kojournalview.moc"
