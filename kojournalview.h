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
*/
#ifndef _KOJOURNALVIEW_H
#define _KOJOURNALVIEW_H
// $Id$

#include <korganizer/baseview.h>

class JournalEntry;

/**
 * This class provides a journal view.
 
 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @see KOBaseView
 */
class KOJournalView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOJournalView(Calendar *calendar, QWidget *parent = 0, 
	       const char *name = 0);
    ~KOJournalView();

    virtual int currentDateCount();
    virtual QPtrList<Incidence> getSelected();

  public slots:
    void updateView();
    void flushView();
    
    void selectDates(const QDateList dateList);
    void selectEvents(QPtrList<Event> eventList);

    void changeEventDisplay(Event *, int);
  
  private:
    JournalEntry *mEntry;
};

#endif
