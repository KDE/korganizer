/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include "kojournalview.moc"

KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent,
                       const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  mEntries.setAutoDelete( true );
  
  QVBoxLayout*topLayout = new QVBoxLayout( this );
  topLayout->setAutoAdd(true);
  mSV = new QScrollView( this, "JournalScrollView" );
  topLayout = new QVBoxLayout( mSV->viewport() );
  topLayout->setAutoAdd(true);
  mVBox = new QVBox( mSV->viewport() );
  mSV->setVScrollBarMode( QScrollView::Auto );
  mSV->setHScrollBarMode( QScrollView::AlwaysOff );
}

KOJournalView::~KOJournalView()
{
}

void KOJournalView::appendJournal( Journal*journal, const QDate &dt)
{
//  kdDebug(5850) << "KOJournalView::appendJournal(): "<< journal<<endl;
//  JournalEntry*j = new JournalEntry( calendar(), this );
  JournalEntry*j = new JournalEntry( calendar(), mVBox );
  if ( j ) {
    j->show();
    j->setJournal( journal );
    j->setDate( dt );
    j->setIncidenceChanger( mChanger );

    mEntries.append( j );
    mSelectedDates.append( dt );
  }
}

int KOJournalView::currentDateCount()
{
  return mSelectedDates.size();
}

/*void KOJournalView::resizeEvent( QResizeEvent *event ) 
{
//  mSV->setSize( 
}*/

Incidence::List KOJournalView::selectedIncidences()
{
  // We don't have a selection in the journal view.
  Incidence::List eventList;
  return eventList;
}

void KOJournalView::clearEntries()
{
//  kdDebug(5850)<<"KOJournalView::clearEntries()"<<endl;
  JournalEntry::List::iterator it;
  for (it=mEntries.begin(); it!=mEntries.end(); ++it ) {
    delete (*it);
  }
  mEntries.clear();
  mSelectedDates.clear();
}
void KOJournalView::updateView()
{
  JournalEntry::List::iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    if ( (*it) ) {
      QDate dt((*it)->date());
      (*it)->setJournal( calendar()->journal( dt ) );
    }
  }
}

void KOJournalView::flushView()
{
//  kdDebug(5850) << "KOJournalView::flushView(): "<< endl;
  JournalEntry::List::iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    if (*it) (*it)->flushEntry();
  }
}

void KOJournalView::showDates(const QDate &start, const QDate &end)
{
//  kdDebug(5850) << "KOJournalView::showDates(): "<<start.toString().latin1()<<" - "<<end.toString().latin1() << endl;
  clearEntries();
  if ( end<start ) return;
  
  QDate d=start;
  for ( QDate d=start; d<=end; d=d.addDays(1) ) {
    Journal *j = calendar()->journal( d );
    appendJournal( j, d );
  }
}

void KOJournalView::showIncidences( const Incidence::List &incidences )
{
//  kdDebug(5850) << "KOJournalView::showIncidences(): "<< endl;
  clearEntries();
  Incidence::List::const_iterator it;
  for ( it=incidences.constBegin(); it!=incidences.constEnd(); ++it) {
    if ((*it) && ( (*it)->type()=="Journal" ) ) {
      Journal*j = (Journal*)(*it);
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
        //addIncidence( incidence );
        break;
      case KOGlobals::INCIDENCEEDITED:
        /*
        item = getItemForEvent(incidence);
        if (item) {
          delete item;
          mUidDict.remove( incidence->uid() );
          addIncidence( incidence );
        }
        */
        break;
      case KOGlobals::INCIDENCEDELETED:
        /*
        item = getItemForEvent(incidence);
        if (item) {
          delete item;
        }
        */
        break;
      default:
        kdDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
    }
  }
}

void KOJournalView::setIncidenceChanger( IncidenceChangerBase *changer ) 
{ 
  mChanger = changer;
  JournalEntry::List::iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    if ( (*it) ) (*it)->setIncidenceChanger( changer );
  }
}
