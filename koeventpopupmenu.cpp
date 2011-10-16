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

#include "koeventpopupmenu.h"
#include "actionmanager.h"
#include "calprinter.h"
#include "kocorehelper.h"
#include "koglobals.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <kcalcore/incidence.h>
#include <kmimetypetrader.h>
#include <KActionCollection>
#include <KLocale>

using namespace KCalCore;

KOEventPopupMenu::KOEventPopupMenu( CalendarSupport::Calendar *calendar, QWidget *parent )
  : QMenu( parent ), mCalendar( calendar )
{
  mHasAdditionalItems = false;

  addAction( KOGlobals::self()->smallIcon( "document-preview" ), i18n( "&Show" ),
             this, SLOT(popupShow()) );
  mEditOnlyItems.append(
    addAction( KOGlobals::self()->smallIcon( "document-edit" ), i18n( "&Edit..." ),
               this, SLOT(popupEdit()) ) );
  mEditOnlyItems.append( addSeparator() );
  addAction( KOGlobals::self()->smallIcon( "document-print" ), i18n( "&Print..." ),
             this, SLOT(print()) );
  QAction *preview = addAction( KOGlobals::self()->smallIcon( "document-print-preview" ),
                                i18n( "Print Previe&w..." ),
                                this, SLOT(printPreview()) );
  preview->setEnabled( !KMimeTypeTrader::self()->query( "application/pdf",
                                                        "KParts/ReadOnlyPart" ).isEmpty() );
  //------------------------------------------------------------------------
  mEditOnlyItems.append( addSeparator() );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-cut" ),
                                    i18nc( "cut this event", "C&ut" ),
                                    this, SLOT(popupCut()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-copy" ),
                                    i18nc( "copy this event", "&Copy" ),
                                    this, SLOT(popupCopy()) ) );
  // paste is always possible
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-paste" ),
                                    i18n( "&Paste" ),
                                    this, SLOT(popupPaste()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-delete" ),
                                    i18nc( "delete this incidence", "&Delete" ),
                                    this, SLOT(popupDelete()) ) );
  //------------------------------------------------------------------------
  mEditOnlyItems.append( addSeparator() );
  mTodoOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "task-complete" ),
                                    i18n( "Togg&le To-do Completed" ),
                                    this, SLOT(toggleTodoCompleted()) ) );
  mToggleReminder =  addAction( QIcon( KOGlobals::self()->smallIcon( "appointment-reminder" ) ),
                                    i18n( "&Toggle Reminder" ), this, SLOT(toggleAlarm()));
  mEditOnlyItems.append( mToggleReminder );
  //------------------------------------------------------------------------
  mRecurrenceItems.append( addSeparator() );
  mDissociateOccurrences = addAction( i18n( "&Dissociate From Recurrence..." ),
                                      this, SLOT(dissociateOccurrences()) );
  mRecurrenceItems.append( mDissociateOccurrences );

  addSeparator();
  addAction( KOGlobals::self()->smallIcon( "mail-forward" ),
             i18n( "Send as iCalendar..." ),
             this, SLOT(forward()) );
}

void KOEventPopupMenu::showIncidencePopup( const Akonadi::Item &item, const QDate &qd )
{
  mCurrentIncidence = item;
  mCurrentDate = qd;

  if ( !CalendarSupport::hasIncidence( mCurrentIncidence )/*&& qd.isValid()*/ ) {
    kDebug() << "No event selected";
    return;
  }

  if ( !mCalendar ) {
    //TODO fix it
    kDebug() << "Calendar is 0";
    return;
  }

  const bool hasChangeRights = mCalendar->hasChangeRights( mCurrentIncidence );

  Incidence::Ptr incidence = CalendarSupport::incidence( mCurrentIncidence );
  Q_ASSERT( incidence );
  if ( incidence->recurs() ) {
    const KDateTime thisDateTime( qd, CalendarSupport::KCalPrefs::instance()->timeSpec() );
    const bool isLastOccurrence =
      !incidence->recurrence()->getNextDateTime( thisDateTime ).isValid();
    const bool isFirstOccurrence =
      !incidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();
    mDissociateOccurrences->setEnabled(
      !( isFirstOccurrence && isLastOccurrence ) && hasChangeRights );
  }

  // Enable/Disabled menu items only valid for editable events.
  QList<QAction *>::Iterator it;
  for ( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
    (*it)->setEnabled( hasChangeRights );
  }
  mToggleReminder->setVisible( ( incidence->type() != Incidence::TypeJournal ) );
  for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
    (*it)->setVisible( incidence->recurs() );
  }
  for ( it = mTodoOnlyItems.begin(); it != mTodoOnlyItems.end(); ++it ) {
    (*it)->setVisible( incidence->type() == Incidence::TypeTodo );
    (*it)->setEnabled( hasChangeRights );
  }
  popup( QCursor::pos() );
}

void KOEventPopupMenu::popupShow()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit showIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupEdit()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit editIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::print()
{
  print( false );
}

void KOEventPopupMenu::print( bool preview )
{
  KOCoreHelper helper;
  CalPrinter printer( this, mCalendar, &helper, true );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  //Item::List selectedIncidences;
  Incidence::List selectedIncidences;
  Q_ASSERT( mCurrentIncidence.hasPayload<KCalCore::Incidence::Ptr>() );
  selectedIncidences.append( mCurrentIncidence.payload<KCalCore::Incidence::Ptr>() );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, preview );
}

void KOEventPopupMenu::printPreview()
{
  print( true );
}

void KOEventPopupMenu::popupDelete()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit deleteIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCut()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit cutIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCopy()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit copyIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupPaste()
{
  emit pasteIncidenceSignal();
}

void KOEventPopupMenu::toggleAlarm()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit toggleAlarmSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::dissociateOccurrences()
{
  if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    emit dissociateOccurrencesSignal( mCurrentIncidence, mCurrentDate );
  }
}

void KOEventPopupMenu::forward()
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( !w || !CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
    return;
  }

  KActionCollection *ac = w->getActionCollection();
  QAction *action = ac->action( "schedule_forward" );
  if ( action ) {
    action->trigger();
  } else {
    kError() << "What happened to the schedule_forward action?";
  }
}

void KOEventPopupMenu::toggleTodoCompleted()
{
  if ( CalendarSupport::hasTodo( mCurrentIncidence ) ) {
    emit toggleTodoCompletedSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::setCalendar( CalendarSupport::Calendar *calendar )
{
  mCalendar = calendar;
}

#include "koeventpopupmenu.moc"
