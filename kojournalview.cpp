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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$
//
// View of Journal entries

#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "kojournalview.moc"

KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent,
		       const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  mEntry = new JournalEntry(calendar,this);
  
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mEntry);
}

KOJournalView::~KOJournalView()
{
}

int KOJournalView::currentDateCount()
{
  return 0;
}

QPtrList<Incidence> KOJournalView::selectedIncidences()
{
  QPtrList<Incidence> eventList;

  return eventList;
}

void KOJournalView::updateView()
{
  kdDebug() << "KOJournalView::updateView() does nothing" << endl;
}

void KOJournalView::flushView()
{
  mEntry->flushEntry();
}

void KOJournalView::showDates(const QDate &start, const QDate &)
{
//  kdDebug() << "KOJournalView::selectDates()" << endl;

  mEntry->setDate(start);

  Journal *j = calendar()->journal(start);
  if (j) mEntry->setJournal(j);
  else mEntry->clear();
  
//  emit eventsSelected(false);
}

void KOJournalView::showEvents(QPtrList<Event>)
{
  // After new creation of list view no events are selected.
//  emit eventsSelected(false);
}

void KOJournalView::changeEventDisplay(Event *, int /*action*/)
{
  updateView();
}
