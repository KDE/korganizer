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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

//
// View of Journal entries

#include <qlayout.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qscrollview.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "koglobals.h"
using namespace KOrg;

KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent,
                       const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  QVBoxLayout*topLayout = new QVBoxLayout( this );
  topLayout->setAutoAdd(true);
  mSV = new QScrollView( this, "JournalScrollView" );
  topLayout = new QVBoxLayout( mSV->viewport() );
  topLayout->setAutoAdd(true);
  mVBox = new QVBox( mSV->viewport() );
  mSV->setVScrollBarMode( QScrollView::Auto );
  mSV->setHScrollBarMode( QScrollView::AlwaysOff );
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
    
    connect( entry, SIGNAL( deleteIncidence( Incidence* ) ),
             this, SIGNAL( deleteIncidenceSignal( Incidence* ) ) );

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
//  kdDebug(5850)<<"KOJournalView::clearEntries()"<<endl;
  QMap<QDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    delete (it.data());
  }
  mEntries.clear();
}
void KOJournalView::updateView()
{
  QMap<QDate, JournalDateEntry*>::Iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    it.data()->clear();
    Journal::List journals = calendar()->journals( it.key() );
    Journal::List::Iterator it1;
    for ( it1 = journals.begin(); it1 != journals.end(); ++it1 ) {
      it.data()->addJournal( *it1 );
    }
  }
}

void KOJournalView::flushView()
{
//  kdDebug(5850) << "KOJournalView::flushView(): "<< endl;
  emit flushEntries();
}

void KOJournalView::showDates(const QDate &start, const QDate &end)
{
//  kdDebug(5850) << "KOJournalView::showDates(): "<<start.toString().latin1()<<" - "<<end.toString().latin1() << endl;
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
//  kdDebug(5850) << "KOJournalView::showIncidences(): "<< endl;
  clearEntries();
  Incidence::List::const_iterator it;
  for ( it=incidences.constBegin(); it!=incidences.constEnd(); ++it) {
    if ((*it) && ( (*it)->type()=="Journal" ) ) {
      Journal*j = static_cast<Journal*>(*it);
      if ( j ) appendJournal( j, j->dtStart().date() );
    }
  }
}

void KOJournalView::changeIncidenceDisplay(Incidence *incidence, int action)
{
//  kdDebug(5850) << "KOJournalView::changeIncidenceDisplay(): "<< endl;
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
        kdDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
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
