/*
  This file is part of KOrganizer.

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

#include "koeventview.h"
#include "kocore.h"
#include "koeventpopupmenu.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <libkdepim/widgets/pimmessagebox.h>

#include <KXMLGUIFactory>

#include <QApplication>
#include <QMenu>

//---------------------------------------------------------------------------

KOEventView::KOEventView( QWidget *parent )
  : KOrg::BaseView( parent )
{
  mReturnPressed = false;
  mTypeAhead = false;
  mTypeAheadReceiver = 0;

  //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(),
  //which triggered finishTypeAhead. But the global focus widget in QApplication is
  //changed later, thus subsequent keyevents still went to this view, triggering
  //another editor, for each keypress
  //Thus listen to the global focusChanged() signal (seen with Qt 4.6-stable-patched 20091112)
  //  -Frank
  connect( qobject_cast<QApplication*>( QApplication::instance() ),
           SIGNAL(focusChanged(QWidget*,QWidget*)),
           this, SLOT(focusChanged(QWidget*,QWidget*)) );
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu( calendar().data(), this );

  connect( eventPopup, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()));
  connect( eventPopup, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)));
  connect( eventPopup, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)));
  connect( eventPopup, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,Akonadi::Collection)));
  connect( eventPopup, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  return eventPopup;
}

QMenu *KOEventView::newEventPopup()
{
  KXMLGUIClient *client = KOCore::self()->xmlguiClient( this );
  if ( !client ) {
    kError() << "no xmlGuiClient.";
    return 0;
  }
  if ( !client->factory() ) {
    kError() << "no factory";
    return 0; // can happen if called too early
  }

  return static_cast<QMenu*>
      ( client->factory()->container( QLatin1String("rmb_selection_popup"), client ) );
}
//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
  emit showIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
  emit editIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
  emit deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCut()
{
  emit cutIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCopy()
{
  emit copyIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
  QMenu *popup = newEventPopup();
  if ( !popup ) {
    kError() << "popup creation failed";
    return;
  }

  popup->popup( QCursor::pos() );
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction( const Akonadi::Item &aitem )
{
  kDebug();
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  if ( !incidence ) {
    kDebug() << "Ouch, null incidence";
    return;
  }

  if ( !calendar()->hasRight( aitem, Akonadi::Collection::CanChangeItem ) ) {
    emit showIncidenceSignal( aitem );
  } else {
    emit editIncidenceSignal( aitem );
  }
}

//---------------------------------------------------------------------------
int KOEventView::showMoveRecurDialog( const Akonadi::Item &aitem, const QDate &date )
{
  const KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( aitem );
  int answer = KMessageBox::Ok;
  KGuiItem itemFuture( i18n( "Also &Future Items" ) );

  KDateTime dateTime( date, CalendarSupport::KCalPrefs::instance()->timeSpec() );
  bool isFirst = !inc->recurrence()->getPreviousDateTime( dateTime ).isValid();
  bool isLast  = !inc->recurrence()->getNextDateTime( dateTime ).isValid();

  QString message;

  if ( !isFirst && !isLast ) {
    itemFuture.setEnabled( true );
    message = i18n( "The item you are trying to change is a recurring item. "
                    "Should the changes be applied only to this single occurrence, "
                    "also to future items, or to all items in the recurrence?" );
  } else {
    itemFuture.setEnabled( false );
    message = i18n( "The item you are trying to change is a recurring item. "
                    "Should the changes be applied only to this single occurrence "
                    "or to all items in the recurrence?" );
  }

  if ( !( isFirst && isLast ) ) {
    answer = PIMMessageBox::fourBtnMsgBox(
      this,
      QMessageBox::Question,
      message,
      i18n( "Changing Recurring Item" ),
      KGuiItem( i18n( "Only &This Item" ) ),
      itemFuture,
      KGuiItem( i18n( "&All Occurrences" ) ) );
  }

  return answer;
}

void KOEventView::setTypeAheadReceiver( QObject *o )
{
  mTypeAheadReceiver = o;
}

void KOEventView::focusChanged( QWidget *, QWidget *now )
{
  if ( mTypeAhead && now && now == mTypeAheadReceiver ) {
    finishTypeAhead();
  }
}

void KOEventView::finishTypeAhead()
{
  if ( mTypeAheadReceiver ) {
    foreach ( QEvent *e, mTypeAheadEvents ) {
      QApplication::sendEvent( mTypeAheadReceiver, e );
    }
  }
  qDeleteAll( mTypeAheadEvents );
  mTypeAheadEvents.clear();
  mTypeAhead = false;
}

bool KOEventView::usesCompletedTodoPixmap( const Akonadi::Item &aitem, const QDate &date )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem );
  if ( !todo ) {
    return false;
  }

  if ( todo->isCompleted() ) {
    return true;
  } else if ( todo->recurs() ) {
    QTime time;
    if ( todo->allDay() ) {
      time = QTime( 0, 0 );
    } else {
      time = todo->dtDue().toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).time();
    }

    KDateTime itemDateTime( date, time, CalendarSupport::KCalPrefs::instance()->timeSpec() );

    return itemDateTime < todo->dtDue( false );

  } else {
    return false;
  }
}


