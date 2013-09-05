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

#include "ui_searchdialog_base.h"
#include "calendarview.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <calendarviews/list/listview.h>

#include <KMessageBox>

#include <QTimer>

using namespace KOrg;

SearchDialog::SearchDialog( CalendarView *calendarview )
  : KDialog( calendarview ),
    m_ui( new Ui::SearchDialog ),
    m_calendarview( calendarview )
{
  setCaption( i18n( "Search Calendar" ) );
  setModal( false );

  QWidget *mainWidget = new QWidget( this );
  m_ui->setupUi( mainWidget );
  setMainWidget( mainWidget );

  resize( QSize( 775, 600 ).expandedTo( minimumSizeHint() ) );

  // Set nice initial start and end dates for the search
  const QDate currDate = QDate::currentDate();
  m_ui->startDate->setDate( currDate );
  m_ui->endDate->setDate( currDate.addYears( 1 ) );

  connect( m_ui->searchEdit, SIGNAL(textChanged(QString)),
           this, SLOT(searchTextChanged(QString)) );

  // Results list view
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin( 0 );
  listView = new EventViews::ListView( m_calendarview->calendar(), this );
  layout->addWidget( listView );
  m_ui->listViewFrame->setLayout( layout );

  connect( this, SIGNAL(user1Clicked()), SLOT(doSearch()) );

  // Propagate edit and delete event signals from event list view
  connect( listView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
          SIGNAL(showIncidenceSignal(Akonadi::Item)) );
  connect( listView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
          SIGNAL(editIncidenceSignal(Akonadi::Item)) );
  connect( listView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
          SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  setButtons( User1 | Cancel );
  setDefaultButton( User1 );
  setButtonGuiItem( User1,
                    KGuiItem( i18nc( "search in calendar", "&Search" ), QLatin1String("edit-find") ) );
  setButtonToolTip( User1, i18n( "Start searching" ) );
  showButtonSeparator( false );
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::showEvent( QShowEvent *event )
{
  Q_UNUSED( event );
  m_ui->searchEdit->setFocus();
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
  re.setPattern( m_ui->searchEdit->text() );
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
  if ( mMatchedEvents.isEmpty() ) {
    m_ui->numItems->setText ( QString() );
    KMessageBox::information(
      this,
      i18n( "No items were found that match your search pattern." ),
      i18n( "Search Results" ),
      QLatin1String( "NoSearchResults" ) );
  } else {
    m_ui->numItems->setText( i18np( "%1 item","%1 items", mMatchedEvents.count() ) );
  }
}

void SearchDialog::updateView()
{
  QRegExp re;
  re.setPatternSyntax( QRegExp::Wildcard ); // most people understand these better.
  re.setCaseSensitivity( Qt::CaseInsensitive );
  re.setPattern( m_ui->searchEdit->text() );
  if ( re.isValid() ) {
    search( re );
  } else {
    mMatchedEvents.clear();
  }
  listView->showIncidences( mMatchedEvents, QDate() );
}

void SearchDialog::search( const QRegExp &re )
{
  const QDate startDt = m_ui->startDate->date();
  const QDate endDt = m_ui->endDate->date();

  KCalCore::Event::List events;
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  if ( m_ui->eventsCheck->isChecked() ) {
    events =
      m_calendarview->calendar()->events(
        startDt, endDt, timeSpec, m_ui->inclusiveCheck->isChecked() );
  }

  KCalCore::Todo::List todos;

  if ( m_ui->todosCheck->isChecked() ) {
    if ( m_ui->includeUndatedTodos->isChecked() ) {
      KDateTime::Spec spec = CalendarSupport::KCalPrefs::instance()->timeSpec();
      KCalCore::Todo::List alltodos = m_calendarview->calendar()->todos();
      Q_FOREACH ( const KCalCore::Todo::Ptr &todo, alltodos ) {
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
          todos.append( todo );
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

  KCalCore::Journal::List journals;
  if ( m_ui->journalsCheck->isChecked() ) {
    QDate dt = startDt;
    while ( dt <= endDt ) {
      journals += m_calendarview->calendar()->journals( dt );
      dt = dt.addDays( 1 );
    }
  }

  mMatchedEvents.clear();
  KCalCore::Incidence::List incidences = Akonadi::ETMCalendar::mergeIncidenceList( events, todos, journals );
  Q_FOREACH ( const KCalCore::Incidence::Ptr &ev, incidences) {
    Q_ASSERT( ev );
    Akonadi::Item item = m_calendarview->calendar()->item( ev->uid() );
    if ( m_ui->summaryCheck->isChecked() ) {
      if ( re.indexIn( ev->summary() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( m_ui->descriptionCheck->isChecked() ) {
      if ( re.indexIn( ev->description() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( m_ui->categoryCheck->isChecked() ) {
      if ( re.indexIn( ev->categoriesStr() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( m_ui->locationCheck->isChecked() ) {
      if ( re.indexIn( ev->location() ) != -1 ) {
        mMatchedEvents.append( item );
        continue;
      }
    }
    if ( m_ui->attendeeCheck->isChecked() ) {
      Q_FOREACH( const KCalCore::Attendee::Ptr &attendee, ev->attendees() ) {
        if ( re.indexIn( attendee->fullName() ) != -1 ) {
          mMatchedEvents.append( item );
          break;
        }
      }
    }
  }
}

#include "searchdialog.moc"
