/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

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

#include "kolistview.h"
#include "koeventpopupmenu.h"

#include <calendarviews/list/listview.h>

#include <QVBoxLayout>

using namespace KOrg;

KOListView::KOListView( const Akonadi::ETMCalendar::Ptr &calendar,
                        QWidget *parent, bool nonInteractive )
  : KOEventView( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mListView = new EventViews::ListView( calendar, this, nonInteractive );
  mPopupMenu = eventPopup();
  setCalendar( calendar );

  layout->addWidget( mListView );

  connect( mListView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           mPopupMenu, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( mListView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( mListView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( mListView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );

  connect( mListView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( mListView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( mListView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( mListView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( mListView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( mListView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( mListView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( mListView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( mListView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( mListView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( mListView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( mListView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );
}

KOListView::~KOListView()
{
  delete mPopupMenu;
  delete mListView;
}

int KOListView::maxDatesHint() const
{
  return 0;
}

int KOListView::currentDateCount() const
{
  return mListView->currentDateCount();
}

Akonadi::Item::List KOListView::selectedIncidences()
{
  return mListView->selectedIncidences();
}

KCalCore::DateList KOListView::selectedIncidenceDates()
{
  return mListView->selectedIncidenceDates();
}

void KOListView::updateView()
{
  mListView->updateView();
}

void KOListView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  mListView->showDates( start, end );
}

void KOListView::showAll()
{
  mListView->showAll();
}

void KOListView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  mListView->showIncidences( incidenceList, date );
}

void KOListView::changeIncidenceDisplay( const Akonadi::Item & aitem,
                                         Akonadi::IncidenceChanger::ChangeType changeType )
{
  mListView->changeIncidenceDisplay( aitem, changeType );
}

void KOListView::defaultItemAction( const QModelIndex &index )
{
  mListView->defaultItemAction( index );
}

void KOListView::defaultItemAction( const Akonadi::Item::Id id )
{
  mListView->defaultItemAction( id );
}

void KOListView::popupMenu( const QPoint &point )
{
  mListView->popupMenu( point );
}

void KOListView::readSettings( KConfig *config )
{
  mListView->readSettings( config );
}

void KOListView::writeSettings( KConfig *config )
{
  mListView->writeSettings( config );
}

void KOListView::clearSelection()
{
  mListView->clearSelection();
}

void KOListView::clear()
{
  mListView->clear();
}

KOrg::CalPrinterBase::PrintType KOListView::printType() const
{
  return KOrg::CalPrinterBase::Incidence;
}

QSize KOListView::sizeHint() const
{
  return mListView->sizeHint();
}

void KOListView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  KOEventView::setCalendar( cal );
  mPopupMenu->setCalendar( cal );
  mListView->setCalendar( cal );
}

void KOListView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  mListView->setIncidenceChanger( changer );
}

#include "kolistview.moc"
