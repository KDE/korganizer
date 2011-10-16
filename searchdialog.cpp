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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "searchdialog.h"
#include "koglobals.h"
#include "calendarview.h"
#include "views/listview/kolistview.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

using namespace KOrg;

SearchDialog::SearchDialog( CalendarView *calendarview )
  : KDialog( calendarview ), m_calendarview( calendarview )
{
  setCaption( i18n( "Search Calendar" ) );
  setButtons( User1 | Cancel );
  setDefaultButton( User1 );
  setModal( false );
  showButtonSeparator( false );
  setButtonGuiItem( User1,
                    KGuiItem( i18nc( "search in calendar", "&Search" ), "edit-find" ) );
  setButtonToolTip( User1, i18n( "Start searching" ) );

  QWidget *mainwidget = new QWidget( this );
  setupUi( mainwidget );
  setMainWidget( mainwidget );

  // Set nice initial start and end dates for the search
  mStartDate->setDate( QDate::currentDate() );
  mEndDate->setDate( QDate::currentDate().addYears( 1 ) );

  connect( mSearchEdit, SIGNAL(textChanged(QString)),
           this, SLOT(searchTextChanged(QString)) );

  // Results list view
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin( 0 );
  listView = new KOListView( m_calendarview->calendar(), this );
  listView->showDates();
  layout->addWidget( listView );
  mListViewFrame->setLayout( layout );

  connect( this, SIGNAL(user1Clicked()), SLOT(doSearch()) );

  // Propagate edit and delete event signals from event list view
  connect( listView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
          SIGNAL(showIncidenceSignal(Akonadi::Item)) );
  connect( listView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
          SIGNAL(editIncidenceSignal(Akonadi::Item)) );
  connect( listView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
          SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );
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
    KMessageBox::sorry(
      this,
      i18n( "Invalid search expression, cannot perform the search. "
            "Please enter a search expression using the wildcard characters "
            "'*' and '?' where needed." ) );
    return;
  }

  search( re );
  listView->showIncidences( mMatchedEvents, QDate() );
  if ( mMatchedEvents.count() == 0 ) {
    mNumItems->setText ( QString() );
    KMessageBox::information(
      this,
      i18n( "No items were found that match your search pattern." ),
      "NoSearchResults" );
  } else {
    mNumItems->setText( i18np( "%1 item","%1 items", mMatchedEvents.count() ) );
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
  listView->showIncidences( mMatchedEvents, QDate() );
}

void SearchDialog::search( const QRegExp &re )
{
  QDate startDt = mStartDate->date();
  QDate endDt = mEndDate->date();

  Akonadi::Item::List events;
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  if ( mEventsCheck->isChecked() ) {
    events =
      m_calendarview->calendar()->events( startDt, endDt, timeSpec, mInclusiveCheck->isChecked() );
  }
  Akonadi::Item::List todos;
  if ( mTodosCheck->isChecked() ) {
    if ( mIncludeUndatedTodos->isChecked() ) {
      KDateTime::Spec spec = CalendarSupport::KCalPrefs::instance()->timeSpec();
      Akonadi::Item::List alltodos = m_calendarview->calendar()->todos();
      Q_FOREACH ( const Akonadi::Item &item, alltodos ) {
        const Todo::Ptr todo = CalendarSupport::todo( item );
        Q_ASSERT( todo );
        if ( ( !todo->hasStartDate() && !todo->hasDueDate() ) || // undated
             ( todo->hasStartDate() &&
               ( todo->dtStart().toTimeSpec( spec ).date() >= startDt ) &&
               ( todo->dtStart().toTimeSpec( spec ).date() <= endDt ) ) || //start dt in range
             ( todo->hasDueDate() &&
               ( todo->dtDue().toTimeSpec( spec ).date() >= startDt ) &&
               ( todo->dtDue().toTimeSpec( spec ).date() <= endDt ) ) || //due dt in range
             ( todo->hasCompletedDate() &&
               ( todo->completed().toTimeSpec( spec ).date() >= startDt ) &&
               ( todo->completed().toTimeSpec( spec ).date() <= endDt ) ) ) {//completed dt in range
          todos.append( item );
        }
      }
    } else {
      QDate dt = startDt;
      while ( dt <= endDt ) {
        todos += m_calendarview->calendar()->todos( dt );
        dt = dt.addDays( 1 );
      }
    }
  }

  Akonadi::Item::List journals;
  if ( mJournalsCheck->isChecked() ) {
    QDate dt = startDt;
    while ( dt <= endDt ) {
      journals += m_calendarview->calendar()->journals( dt );
      dt = dt.addDays( 1 );
    }
  }

  mMatchedEvents.clear();
  Q_FOREACH ( const Akonadi::Item &item,
              CalendarSupport::Calendar::mergeIncidenceList( events, todos, journals ) ) {
    const Incidence::Ptr ev = CalendarSupport::incidence( item );
    Q_ASSERT( ev );
    if ( mSummaryCheck->isChecked() ) {
      if ( re.indexIn( ev->summary() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( mDescriptionCheck->isChecked() ) {
      if ( re.indexIn( ev->description() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( mCategoryCheck->isChecked() ) {
      if ( re.indexIn( ev->categoriesStr() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( mLocationCheck->isChecked() ) {
      if ( re.indexIn( ev->location() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
  }
}

#include "searchdialog.moc"
