/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H
// $Id$

#include <qregexp.h>

#include <kdialogbase.h>

#include <libkcal/calendar.h>

#include "kolistview.h"

class KDateEdit;
class QCheckBox;
class KLineEdit;
class QLabel;

using namespace KCal;

class SearchDialog : public KDialogBase
{
    Q_OBJECT
  public:
    SearchDialog(Calendar *calendar,QWidget *parent=0);
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeEventDisplay(Event *, int) { updateView(); } 
  
  protected slots:
    void doSearch();
  void searchTextChanged( const QString &_text );	
  signals:
    void showEventSignal(Event *);
    void editEventSignal(Event *);
    void deleteEventSignal(Event *);

  private:
    void search(const QRegExp &);

    Calendar *mCalendar;
    
    QPtrList<Event> mMatchedEvents;
    
    QLabel *searchLabel;
    QLineEdit *searchEdit;
    KOListView *listView;
    
    KDateEdit *mStartDate;
    KDateEdit *mEndDate;
    
    QCheckBox *mInclusiveCheck;
    QCheckBox *mSummaryCheck;
    QCheckBox *mDescriptionCheck;
    QCheckBox *mCategoryCheck;
};

#endif
