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
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "kocorehelper.h"
#include "koglobals.h"

#include <KCal/Incidence>

#include <KActionCollection>
#include <KLocale>

KOEventPopupMenu::KOEventPopupMenu()
{
  mCalendar = 0;
  mCurrentIncidence = 0;
  mCurrentDate = QDate();
  mHasAdditionalItems = false;

  addAction( i18n( "&Show" ), this, SLOT( popupShow() ) );
  mEditOnlyItems.append( addAction( i18n( "&Edit..." ), this, SLOT( popupEdit() ) ) );
#ifndef KORG_NOPRINTER
  mEditOnlyItems.append( addSeparator() );
  addAction( KOGlobals::self()->smallIcon( "document-print" ), i18n( "&Print..." ),
             this, SLOT( print() ) );
  addAction( KOGlobals::self()->smallIcon( "document-print-preview" ), i18n( "Print Previe&w..." ),
             this, SLOT( printPreview() ) );
#endif
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
  mEditOnlyItems.append( addAction( QIcon( KOGlobals::self()->smallIcon( "appointment-reminder" ) ),
                                    i18n( "&Toggle Reminder" ), this, SLOT(toggleAlarm())) );
  //------------------------------------------------------------------------
  mRecurrenceItems.append( addSeparator() );
  mDissociateOccurrences = addAction( i18n( "&Dissociate From Recurrence" ),
                                      this, SLOT(dissociateOccurrences()) );
  mRecurrenceItems.append( mDissociateOccurrences );

  addSeparator();
  addAction( KOGlobals::self()->smallIcon( "mail-forward" ),
             i18n( "Send as iCalendar..." ),
             this, SLOT(forward()) );
}

void KOEventPopupMenu::showIncidencePopup( Calendar *cal, Incidence *incidence, const QDate &qd )
{
  mCalendar = cal;
  mCurrentIncidence = incidence;
  mCurrentDate = qd;

  if ( mCurrentIncidence ) {

    if ( mCurrentIncidence->recurs() ) {
      KDateTime thisDateTime( qd, KOPrefs::instance()->timeSpec() );
      bool isLastOccurrence =
        !mCurrentIncidence->recurrence()->getNextDateTime( thisDateTime ).isValid();
      bool isFirstOccurrence =
        !mCurrentIncidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();
      mDissociateOccurrences->setEnabled(
        !( isFirstOccurrence && isLastOccurrence ) &&
        !mCurrentIncidence->isReadOnly() );
    }

    // Enable/Disabled menu items only valid for editable events.
    QList<QAction *>::Iterator it;
    for ( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      (*it)->setEnabled( !mCurrentIncidence->isReadOnly() );
    }
    for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
      (*it)->setVisible( mCurrentIncidence->recurs() );
    }
    for ( it = mTodoOnlyItems.begin(); it != mTodoOnlyItems.end(); ++it ) {
      (*it)->setVisible( mCurrentIncidence->type() == "Todo" );
      (*it)->setEnabled( !mCurrentIncidence->isReadOnly() );
    }
    popup( QCursor::pos() );
  } else {
    kDebug() << "No event selected";
  }
}

void KOEventPopupMenu::popupShow()
{
  if ( mCurrentIncidence ) {
    emit showIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupEdit()
{
  if ( mCurrentIncidence ) {
    emit editIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::print()
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, mCalendar, &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mCurrentIncidence );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, false );
#endif
}

void KOEventPopupMenu::printPreview()
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, mCalendar, &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mCurrentIncidence );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, true );
#endif
}

void KOEventPopupMenu::popupDelete()
{
  if ( mCurrentIncidence ) {
    emit deleteIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCut()
{
  if ( mCurrentIncidence ) {
    emit cutIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCopy()
{
  if ( mCurrentIncidence ) {
    emit copyIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupPaste()
{
  emit pasteIncidenceSignal();
}

void KOEventPopupMenu::toggleAlarm()
{
  if ( mCurrentIncidence ) {
    emit toggleAlarmSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::dissociateOccurrences()
{
  if ( mCurrentIncidence ) {
    emit dissociateOccurrencesSignal( mCurrentIncidence, mCurrentDate );
  }
}

void KOEventPopupMenu::forward()
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( !w || !mCurrentIncidence ) {
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
  if ( mCurrentIncidence && mCurrentIncidence->type() == "Todo" ) {
    emit toggleTodoCompletedSignal( mCurrentIncidence );
  }
}

#include "koeventpopupmenu.moc"
