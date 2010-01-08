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
#include "koeventview.h"

#include <akonadi/kcal/utils.h>

#include <KCal/Incidence>

#include <KActionCollection>
#include <KLocale>

using namespace Akonadi;
using namespace KCal;

KOEventPopupMenu::KOEventPopupMenu( KOEventView *eventview )
  : QMenu( eventview ), mEventview( eventview ),
    mCopyToCalendarMenu( 0 ), mMoveToCalendarMenu( 0 )
{
  mHasAdditionalItems = false;

  addAction( KOGlobals::self()->smallIcon( "document-preview" ), i18n( "&Show" ),
             this, SLOT( popupShow() ) );
  mEditOnlyItems.append(
    addAction( KOGlobals::self()->smallIcon( "document-edit" ), i18n( "&Edit..." ),
               this, SLOT( popupEdit() ) ) );
  mEditOnlyItems.append( addSeparator() );
  addAction( KOGlobals::self()->smallIcon( "document-print" ), i18n( "&Print..." ),
             this, SLOT( print() ) );
  addAction( KOGlobals::self()->smallIcon( "document-print-preview" ), i18n( "Print Previe&w..." ),
             this, SLOT( printPreview() ) );
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

  if ( !Akonadi::hasIncidence( mCurrentIncidence ) && qd.isValid() ) {
    kDebug() << "No event selected";
    return;
  }

  if ( mCopyToCalendarMenu ) {
    removeAction( mCopyToCalendarMenu->menuAction() );
    delete mCopyToCalendarMenu;
    mCopyToCalendarMenu = 0;
  }
  if ( mMoveToCalendarMenu ) {
    removeAction( mMoveToCalendarMenu->menuAction() );
    delete mMoveToCalendarMenu;
    mMoveToCalendarMenu = 0;
  }

  Incidence::Ptr incidence = Akonadi::incidence( mCurrentIncidence );
  Q_ASSERT( incidence );
  if ( incidence->recurs() ) {
    KDateTime thisDateTime( qd, KOPrefs::instance()->timeSpec() );
    bool isLastOccurrence =
      !incidence->recurrence()->getNextDateTime( thisDateTime ).isValid();
    bool isFirstOccurrence =
      !incidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();
    mDissociateOccurrences->setEnabled(
      !( isFirstOccurrence && isLastOccurrence ) &&
      !incidence->isReadOnly() );
  }

  // Enable/Disabled menu items only valid for editable events.
  QList<QAction *>::Iterator it;
  for ( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
    (*it)->setEnabled( !incidence->isReadOnly() );
  }
  for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
    (*it)->setVisible( incidence->recurs() );
  }
  for ( it = mTodoOnlyItems.begin(); it != mTodoOnlyItems.end(); ++it ) {
    (*it)->setVisible( incidence->type() == "Todo" );
    (*it)->setEnabled( !incidence->isReadOnly() );
  }
  popup( QCursor::pos() );
}

void KOEventPopupMenu::popupShow()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit showIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupEdit()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit editIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::print()
{
  KOCoreHelper helper;
  CalPrinter printer( this, mEventview->calendar(), &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  //Item::List selectedIncidences;
  KCal::ListBase<KCal::Incidence> selectedIncidences;
  Q_ASSERT( mCurrentIncidence.hasPayload<KCal::Incidence::Ptr>() );
  selectedIncidences.append( mCurrentIncidence.payload<KCal::Incidence::Ptr>().get() );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, false );
}

void KOEventPopupMenu::printPreview()
{
  KOCoreHelper helper;
  CalPrinter printer( this, mEventview->calendar(), &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  //Item::List selectedIncidences;
  KCal::ListBase<KCal::Incidence> selectedIncidences;
  Q_ASSERT( mCurrentIncidence.hasPayload<KCal::Incidence::Ptr>() );
  selectedIncidences.append( mCurrentIncidence.payload<KCal::Incidence::Ptr>().get() );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, true );
}

void KOEventPopupMenu::popupDelete()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit deleteIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCut()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit cutIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCopy()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit copyIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupPaste()
{
  emit pasteIncidenceSignal();
}

void KOEventPopupMenu::toggleAlarm()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit toggleAlarmSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::dissociateOccurrences()
{
  if ( Akonadi::hasIncidence( mCurrentIncidence ) ) {
    emit dissociateOccurrencesSignal( mCurrentIncidence, mCurrentDate );
  }
}

void KOEventPopupMenu::forward()
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( !w || !Akonadi::hasIncidence( mCurrentIncidence ) ) {
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
  if ( Akonadi::hasTodo( mCurrentIncidence ) ) {
    emit toggleTodoCompletedSignal( mCurrentIncidence );
  }
}

#ifdef AKONADI_PORT_DISABLED
bool KOEventPopupMenu::isResourceWritable( const ResourceCalendar *resource ) const
{
  const bool isCurrentIncidenceOwner =
      KOrg::StdCalendar::self()->resource( mCurrentIncidence ) == resource;

  // FIXME: only take into account writable subresources here
  return
    resource->isActive() &&
    !resource->readOnly() &&
    ( !isCurrentIncidenceOwner || resource->subresources().size() > 2 );
}
#endif

QMenu *KOEventPopupMenu::buildCalendarCopyMenu()
{
  QMenu *const resourceMenu = new QMenu( i18n( "C&opy to Calendar" ), this );
  resourceMenu->setIcon( KIcon( "edit-copy" ) );
#ifdef AKONADI_PORT_DISABLED
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;

    if ( !isResourceWritable( resource ) ) {
      continue;
    }

    QAction *const resourceAction = new QAction( resource->resourceName(), resourceMenu );
    resourceAction->setData( QVariant::fromValue( resource->identifier() ) );
    resourceMenu->addAction( resourceAction );

  }
  connect( resourceMenu, SIGNAL(triggered(QAction*)),
           this, SLOT(copyIncidenceToResource(QAction*)) );
#endif
  return resourceMenu;
}

QMenu *KOEventPopupMenu::buildCalendarMoveMenu()
{
  QMenu *const resourceMenu = new QMenu( i18n( "&Move to Calendar" ), this );
  resourceMenu->setIcon( KIcon( "go-jump" ) );
#ifdef AKONADI_PORT_DISABLED
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;

    if ( !isResourceWritable( resource ) ) {
      continue;
    }

    QAction *const resourceAction = new QAction( resource->resourceName(), resourceMenu );
    resourceAction->setData( QVariant::fromValue( resource->identifier() ) );
    resourceMenu->addAction( resourceAction );

  }
  connect( resourceMenu, SIGNAL(triggered(QAction*)),
           this, SLOT(moveIncidenceToResource(QAction*)) );
#endif
  return resourceMenu;
}

bool KOEventPopupMenu::hasOtherWriteableCalendars() const
{
#ifdef AKONADI_PORT_DISABLED
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;
    if ( isResourceWritable( resource ) ) {
      return true;
    }
  }
#endif
  return false;
}

void KOEventPopupMenu::copyIncidenceToResource( QAction *action )
{
#ifdef AKONADI_PORT_DISABLED
  const QString resourceId = action->data().toString();
  if ( mCurrentIncidence && !resourceId.isEmpty() ) {
    emit copyIncidenceToResourceSignal( mCurrentIncidence, resourceId );
  }
#endif
}

void KOEventPopupMenu::moveIncidenceToResource( QAction *action )
{
#ifdef AKONADI_PORT_DISABLED
  const QString resourceId = action->data().toString();
  if ( mCurrentIncidence && !resourceId.isEmpty() ) {
    emit moveIncidenceToResourceSignal( mCurrentIncidence, resourceId );
  }
#endif
}

#include "koeventpopupmenu.moc"
