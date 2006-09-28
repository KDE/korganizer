/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <klocale.h>
#include <kmessagebox.h>

#include <kcal/calendar.h>

#include <libkdepim/kdateedit.h>

#include "koglobals.h"
#include "koprefs.h"
#include "kolistview.h"

#include "searchdialog.h"
#include "searchdialog.moc"

SearchDialog::SearchDialog(Calendar *calendar,QWidget *parent)
  : KDialog( parent )
{
  setCaption( i18n("Find Events") );
  setButtons( User1|Close );
  setDefaultButton( User1 );
  setModal( false );
  showButtonSeparator( false );
  setButtonGuiItem( User1, KGuiItem( i18n("&Find"), "find") );
  mCalendar = calendar;

  QWidget *mainwidget = new QWidget( this );
  setupUi( mainwidget );
  setMainWidget( mainwidget );

  connect( mSearchEdit, SIGNAL( textChanged( const QString & ) ),
           this, SLOT( searchTextChanged( const QString & ) ) );

  // Results list view
  QVBoxLayout *layout = new QVBoxLayout; 
  layout->setMargin( 0 );
  listView = new KOListView( mCalendar );
  listView->showDates();
  layout->addWidget( listView );
  mListViewParent->setLayout( layout );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    KOGlobals::fitDialogToScreen( this, true );
  }

  connect( this,SIGNAL(user1Clicked()),SLOT(doSearch()));

  // Propagate edit and delete event signals from event list view
  connect( listView, SIGNAL( showIncidenceSignal( Incidence * ) ),
          SIGNAL( showIncidenceSignal( Incidence *) ) );
  connect( listView, SIGNAL( editIncidenceSignal( Incidence * ) ),
          SIGNAL( editIncidenceSignal( Incidence * ) ) );
  connect( listView, SIGNAL( deleteIncidenceSignal( Incidence * ) ),
          SIGNAL( deleteIncidenceSignal( Incidence * ) ) );
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::searchTextChanged( const QString &_text )
{
  enableButton( KDialog::User1, !_text.isEmpty() );
}

void SearchDialog::doSearch()
{
  QRegExp re;

  re.setPatternSyntax( QRegExp::Wildcard ); // most people understand these better.
  re.setCaseSensitivity( Qt::CaseInsensitive );
  re.setPattern( mSearchEdit->text() );
  if ( !re.isValid() ) {
    KMessageBox::sorry( this,
                        i18n("Invalid search expression, cannot perform "
                            "the search. Please enter a search expression "
                            "using the wildcard characters '*' and '?' "
                            "where needed." ) );
    return;
  }

  search( re );

  listView->showIncidences( mMatchedEvents );

  if ( mMatchedEvents.count() == 0 ) {
    KMessageBox::information( this,
        i18n("No events were found matching your search expression."),
        "NoSearchResults" );
  }
}

void SearchDialog::updateView()
{
  QRegExp re;
  re.setPatternSyntax( QRegExp::Wildcard ); // most people understand these better.
  re.setCaseSensitivity( Qt::CaseInsensitive );
  re.setPattern( mSearchEdit->text() );
  if ( re.isValid() ) {
    search( re );
  } else {
    mMatchedEvents.clear();
  }

  listView->showIncidences( mMatchedEvents );
}

void SearchDialog::search( const QRegExp &re )
{
  QDate startDt = mStartDate->date();
  QDate endDt = mEndDate->date();

  Event::List events;
  if (mEventsCheck->isChecked()) {
    events = mCalendar->events( startDt, endDt, mInclusiveCheck->isChecked() );
  }
  Todo::List todos;
  if (mTodosCheck->isChecked()) {
    if ( mIncludeUndatedTodos->isChecked() ) {
      Todo::List alltodos = mCalendar->todos();
      Todo::List::iterator it;
      Todo *todo;
      for (it=alltodos.begin(); it!=alltodos.end(); ++it) {
        todo = *it;
        if ( (!todo->hasStartDate() && !todo->hasDueDate() ) || // undated
             ( todo->hasStartDate() && (todo->dtStart().date()>=startDt) && (todo->dtStart().date()<=endDt) ) || // start dt in range
             ( todo->hasDueDate() && (todo->dtDue().date()>=startDt) && (todo->dtDue().date()<=endDt) ) || // due dt in range
             ( todo->hasCompletedDate() && (todo->completed().date()>=startDt) && (todo->completed().date()<=endDt) ) ) { // completed dt in range
          todos.append( todo );
        }
      }
    } else {
      QDate dt = startDt;
      while ( dt <= endDt ) {
        todos += mCalendar->todos( dt );
        dt = dt.addDays( 1 );
      }
    }
  }

  Journal::List journals;
  if (mJournalsCheck->isChecked()) {
    QDate dt = startDt;
    while ( dt <= endDt ) {
      journals += mCalendar->journals( dt );
      dt = dt.addDays( 1 );
    }
  }

  Incidence::List allIncidences = Calendar::mergeIncidenceList( events, todos, journals );

  mMatchedEvents.clear();
  Incidence::List::ConstIterator it;
  for( it = allIncidences.begin(); it != allIncidences.end(); ++it ) {
    Incidence *ev = *it;
    if ( mSummaryCheck->isChecked() ) {
      if ( re.indexIn( ev->summary() ) != -1 ) {
        mMatchedEvents.append( ev );
        continue;
      }
    }
    if ( mDescriptionCheck->isChecked() ) {
      if ( re.indexIn( ev->description() ) != -1 ) {
        mMatchedEvents.append( ev );
        continue;
      }
    }
    if ( mCategoryCheck->isChecked() ) {
      if ( re.indexIn( ev->categoriesStr() ) != -1 ) {
        mMatchedEvents.append( ev );
        continue;
      }
    }
  }
}
