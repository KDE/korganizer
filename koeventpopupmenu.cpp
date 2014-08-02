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
#include "kocorehelper.h"
#include "koglobals.h"
#include "noteeditdialog.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/printing/calprinter.h>
#include <calendarsupport/printing/calprintdefaultplugins.h>

#include <KCalCore/CalFormat>
#include <KCalCore/Incidence>
#include <Akonadi/Notes/NoteUtils>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemCreateJob>

#include <KDebug>

#include <KActionCollection>
#include <KMime/Message>
#include <KMimeTypeTrader>

#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <incidenceeditor-ng/incidencedialog.h>

KOEventPopupMenu::KOEventPopupMenu( Akonadi::ETMCalendar * calendar, QWidget *parent )
  : QMenu( parent ), mCalendar( calendar )
{
  mHasAdditionalItems = false;

  QAction *action;

  addAction( KOGlobals::self()->smallIcon( QLatin1String("document-preview") ), i18n( "&Show" ),
             this, SLOT(popupShow()) );
  mEditOnlyItems.append(
    addAction( KOGlobals::self()->smallIcon( QLatin1String("document-edit") ), i18n( "&Edit..." ),
               this, SLOT(popupEdit()) ) );
  mEditOnlyItems.append( addSeparator() );
  addAction( KOGlobals::self()->smallIcon( QLatin1String("document-print") ), i18n( "&Print..." ),
             this, SLOT(print()) );
  QAction *preview = addAction( KOGlobals::self()->smallIcon( QLatin1String("document-print-preview") ),
                                i18n( "Print Previe&w..." ),
                                this, SLOT(printPreview()) );
  preview->setEnabled( !KMimeTypeTrader::self()->query(QLatin1String( "application/pdf"),
                                                        QLatin1String("KParts/ReadOnlyPart") ).isEmpty() );
  //------------------------------------------------------------------------
  mEditOnlyItems.append( addSeparator() );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( QLatin1String("edit-cut") ),
                                    i18nc( "cut this event", "C&ut" ),
                                    this, SLOT(popupCut()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( QLatin1String("edit-copy") ),
                                    i18nc( "copy this event", "&Copy" ),
                                    this, SLOT(popupCopy()) ) );
  // paste is always possible
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( QLatin1String("edit-paste") ),
                                    i18n( "&Paste" ),
                                    this, SLOT(popupPaste()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( QLatin1String("edit-delete") ),
                                    i18nc( "delete this incidence", "&Delete" ),
                                    this, SLOT(popupDelete()) ) );
  //------------------------------------------------------------------------
  addSeparator();
  action = addAction( KOGlobals::self()->smallIcon( QLatin1String("task-new") ),
             i18n( "Create To-do" ),
             this, SLOT(createTodo()) );
  action->setObjectName(QLatin1String("createtodo"));
  mEventOnlyItems.append(action);

  action = addAction( KOGlobals::self()->smallIcon( QLatin1String("appointment-new") ),
             i18n( "Create Event" ),
             this, SLOT(createEvent()) );
  action->setObjectName(QLatin1String("createevent"));
  mTodoOnlyItems.append(action);

  action = addAction( KOGlobals::self()->smallIcon( QLatin1String("view-pim-notes") ),
             i18n( "Create Note" ),
             this, SLOT(createNote()) );
  action->setObjectName(QLatin1String("createnote"));
  //------------------------------------------------------------------------
  addSeparator();
  mTodoOnlyItems.append( addAction( KOGlobals::self()->smallIcon( QLatin1String("task-complete") ),
                                    i18n( "Togg&le To-do Completed" ),
                                    this, SLOT(toggleTodoCompleted()) ) );
  mToggleReminder =  addAction( QIcon( KOGlobals::self()->smallIcon( QLatin1String("appointment-reminder") ) ),
                                    i18n( "&Toggle Reminder" ), this, SLOT(toggleAlarm()));
  mEditOnlyItems.append( mToggleReminder );
  //------------------------------------------------------------------------
  mRecurrenceItems.append( addSeparator() );
  mDissociateOccurrences = addAction( i18n( "&Dissociate From Recurrence..." ),
                                      this, SLOT(dissociateOccurrences()) );
  mRecurrenceItems.append( mDissociateOccurrences );

  addSeparator();
  addAction( KOGlobals::self()->smallIcon( QLatin1String("mail-forward") ),
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

  const bool hasChangeRights = mCalendar->hasRight( mCurrentIncidence, Akonadi::Collection::CanChangeItem );

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mCurrentIncidence );
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
  mToggleReminder->setVisible( ( incidence->type() != KCalCore::Incidence::TypeJournal ) );
  for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
    (*it)->setVisible( incidence->recurs() );
  }
  for ( it = mTodoOnlyItems.begin(); it != mTodoOnlyItems.end(); ++it ) {
    (*it)->setVisible( incidence->type() == KCalCore::Incidence::TypeTodo );
    (*it)->setEnabled( hasChangeRights );
  }
  for ( it = mEventOnlyItems.begin(); it != mEventOnlyItems.end(); ++it ) {
    (*it)->setVisible( incidence->type() == KCalCore::Incidence::TypeEvent );
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
  CalendarSupport::CalPrinter printer( this, mCalendar, true );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  //Item::List selectedIncidences;
  KCalCore::Incidence::List selectedIncidences;
  Q_ASSERT( mCurrentIncidence.hasPayload<KCalCore::Incidence::Ptr>() );
  selectedIncidences.append( mCurrentIncidence.payload<KCalCore::Incidence::Ptr>() );

  printer.print( CalendarSupport::CalPrinterBase::Incidence,
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
  QAction *action = ac->action( QLatin1String("schedule_forward") );
  if ( action ) {
    action->trigger();
  } else {
    kError() << "What happened to the schedule_forward action?";
  }
}

void KOEventPopupMenu::createEvent(const Akonadi::Item &item)
{
    mCurrentIncidence = item;
    createEvent();
}


void KOEventPopupMenu::createEvent()
{
    // Must be a Incidence
    if ( !CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
        return;
    }
    // Event ->event doesn't make sense
    if ( CalendarSupport::hasEvent( mCurrentIncidence ) ) {
        return;
    }

    if ( CalendarSupport::hasTodo(mCurrentIncidence) ) {
        KCalCore::Todo::Ptr todo( CalendarSupport::todo( mCurrentIncidence ) );
        KCalCore::Event::Ptr event( new KCalCore::Event(*todo) );
        event->setUid(KCalCore::CalFormat::createUniqueId());
        event->setDtStart(todo->dtStart());
        event->setAllDay(todo->allDay());
        event->setDtEnd(todo->dtDue());
        Akonadi::Item newEventItem;
        newEventItem.setMimeType( KCalCore::Event::eventMimeType() );
        newEventItem.setPayload<KCalCore::Event::Ptr>( event );

        IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalCore::IncidenceBase::TypeEvent, 0, this);
        dlg->setObjectName(QLatin1String("incidencedialog"));
        dlg->load(newEventItem);
        dlg->open();
    }
}

void KOEventPopupMenu::createNote(const Akonadi::Item &item)
{
    mCurrentIncidence = item;
    createNote();
}

void KOEventPopupMenu::createNote()
{
    // Must be a Incidence
    if ( CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
        KCalCore::Incidence::Ptr incidence( CalendarSupport::incidence( mCurrentIncidence ) );
        Akonadi::NoteUtils::NoteMessageWrapper note;
        note.setTitle( incidence->summary() );
        note.setText( incidence->description(), incidence->descriptionIsRich()? Qt::RichText: Qt::PlainText);
        note.setFrom(QCoreApplication::applicationName()+QCoreApplication::applicationVersion());
        note.setLastModifiedDate(KDateTime::currentUtcDateTime());
        Akonadi::NoteUtils::Attachment attachment( mCurrentIncidence.url().url(), mCurrentIncidence.mimeType() );
        note.attachments().append( attachment );
        Akonadi::Item newNoteItem;
        newNoteItem.setMimeType( Akonadi::NoteUtils::noteMimeType() );
        newNoteItem.setPayload( note.message() );

        NoteEditDialog *noteedit = new NoteEditDialog(this);
        connect(noteedit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)), this, SLOT(slotCreateNote(Akonadi::Item,Akonadi::Collection)));
        noteedit->load(newNoteItem);
        noteedit->show();
    }
}

void KOEventPopupMenu::slotCreateNote(const Akonadi::Item &noteItem, const Akonadi::Collection &collection)
{
    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(noteItem, collection, this);
    connect(createJob, SIGNAL(result(KJob*)), this, SLOT(slotCreateNewNoteJobFinished(KJob*)));
    createJob->start();
}

void KOEventPopupMenu::slotCreateNewNoteJobFinished(KJob *job)
{
    if ( job->error() ) {
        qDebug() << "Error during create new Note "<<job->errorString();
    }
}

void KOEventPopupMenu::createTodo()
{
    // Must be a Incidence
    if ( !CalendarSupport::hasIncidence( mCurrentIncidence ) ) {
        return;
    }
    // Todo->Todo doesn't make sense
    if ( CalendarSupport::hasTodo( mCurrentIncidence ) ) {
        return;
    }

    if ( CalendarSupport::hasEvent(mCurrentIncidence) ) {
        KCalCore::Event::Ptr event( CalendarSupport::event( mCurrentIncidence ) );
        KCalCore::Todo::Ptr todo( new KCalCore::Todo(*event) );
        todo->setUid(KCalCore::CalFormat::createUniqueId());
        todo->setDtStart(event->dtStart());
        todo->setAllDay(event->allDay());
        todo->setDtDue(event->dtEnd());
        Akonadi::Item newTodoItem;
        newTodoItem.setMimeType( KCalCore::Todo::todoMimeType() );
        newTodoItem.setPayload<KCalCore::Todo::Ptr>( todo );

        IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalCore::IncidenceBase::TypeTodo, 0, this);
        dlg->setObjectName(QLatin1String("incidencedialog"));
        dlg->load(newTodoItem);
        dlg->open();
    }
}

void KOEventPopupMenu::toggleTodoCompleted()
{
  if ( CalendarSupport::hasTodo( mCurrentIncidence ) ) {
    emit toggleTodoCompletedSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
{
  mCalendar = calendar;
}

