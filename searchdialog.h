/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <qregexp.h>

#include <kdialogbase.h>

#include <libkcal/incidence.h>

namespace KCal {
class Calendar;
}
class KDateEdit;
class QCheckBox;
class QLineEdit;
class QLabel;
class KOListView;

using namespace KCal;

class SearchDialog : public KDialogBase
{
    Q_OBJECT
  public:
    SearchDialog(Calendar *calendar,QWidget *parent=0);
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeIncidenceDisplay(Incidence *, int) { updateView(); }

  protected slots:
    void doSearch();
    void searchTextChanged( const QString &_text );

  signals:
    void showIncidenceSignal(Incidence *);
    void editIncidenceSignal(Incidence *);
    void deleteIncidenceSignal(Incidence *);

  private:
    void search(const QRegExp &);

    Calendar *mCalendar;

    Incidence::List mMatchedEvents;

    QLabel *searchLabel;
    QLineEdit *searchEdit;
    KOListView *listView;

    QCheckBox *mEventsCheck;
    QCheckBox *mTodosCheck;
    QCheckBox *mJournalsCheck;
    
    KDateEdit *mStartDate;
    KDateEdit *mEndDate;

    QCheckBox *mInclusiveCheck;
    QCheckBox *mIncludeUndatedTodos;
    
    QCheckBox *mSummaryCheck;
    QCheckBox *mDescriptionCheck;
    QCheckBox *mCategoryCheck;
};

#endif
