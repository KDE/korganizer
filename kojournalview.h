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
#ifndef KOJOURNALVIEW_H
#define KOJOURNALVIEW_H

#include <korganizer/baseview.h>
#include "journalentry.h"

class JournalEntry;
class QScrollView;
class QVBox;

/**
 * This class provides a journal view.

 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>, Reinhold Kainhofer <reinhold@kainhofer.com>
 * @see KOBaseView
 */
class KOJournalView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOJournalView( Calendar *calendar, QWidget *parent = 0,
                   const char *name = 0);
    ~KOJournalView();

    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences();
    DateList selectedDates() { return DateList(); }
    void appendJournal( Journal*journal, const QDate &dt);

  public slots:
    // Don't update the view when midnight passed, otherwise we'll have data loss (bug 79145)
    virtual void dayPassed( QDate ) {}
    void updateView();
    void flushView();

    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Incidence::List &incidenceList );

    void changeIncidenceDisplay( Incidence *, int );
  protected:
    void clearEntries();

  private:
    QScrollView *mSV;
    QVBox *mVBox;
    JournalEntry::List mEntries;
    DateList mSelectedDates;  // List of dates to be displayed
};

#endif
