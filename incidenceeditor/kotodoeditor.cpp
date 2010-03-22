/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
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
#include "kotodoeditor.h"
#include "koeditorattachments.h"
#include "koeditorconfig.h"
#include "koeditordetails.h"
#include "koeditorgeneraltodo.h"
#include "koeditorrecurrence.h"
#include "korganizer/incidencechangerbase.h"

#include <akonadi/kcal/utils.h> //krazy:exclude=camelcase since kdepim/akonadi

#include <Akonadi/CollectionComboBox>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <KLocale>
#include <KSystemTimeZones>

#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QVBoxLayout>

KOTodoEditor::KOTodoEditor( QWidget *parent )
  : KOIncidenceEditor( QString(),
                       QStringList() << Akonadi::IncidenceMimeTypeVisitor::todoMimeType(),
                       parent ),
    mRelatedTodo(), mGeneral( 0 ), mRecurrence( 0 )
{
  mInitialTodo = Todo::Ptr( new Todo );
  mInitialTodoItem.setPayload( mInitialTodo );
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mIncidence );
}

bool KOTodoEditor::incidenceModified()
{
  Todo::Ptr oldTodo;
  if ( Akonadi::hasTodo( mIncidence ) ) { // modification
    oldTodo = Akonadi::todo( mIncidence );
  } else { // new one
    // don't remove .clone(), it's on purpose, clone() strips relation attributes
    // if you compare a non-cloned parent to-do with a cloned to-do you will always
    // get false, so we use clone() in both cases.
    oldTodo = Todo::Ptr( mInitialTodo->clone() );
  }

  Todo::Ptr newTodo( oldTodo->clone() );

  Akonadi::Item newTodoItem;
  newTodoItem.setPayload(newTodo);
  fillTodo( newTodoItem );

  const bool modified = !( *newTodo == *oldTodo );
  return modified;
}

void KOTodoEditor::init()
{
  setupGeneral();
  setupRecurrence();
  setupAttendeesTab();

  connect( mGeneral, SIGNAL(dateTimeStrChanged(const QString&)),
           mRecurrence, SLOT(setDateTimeStr(const QString&)) );
  connect( mGeneral, SIGNAL(signalDateTimeChanged(const QDateTime&,const QDateTime&)),
           mRecurrence, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

  connect( mDetails, SIGNAL(updateAttendeeSummary(int)),
           mGeneral, SLOT(updateAttendeeSummary(int)) );

  connect( mGeneral, SIGNAL(editRecurrence()),
           mRecurrenceDialog, SLOT(show()) );
  connect( mRecurrenceDialog, SIGNAL(okClicked()),
           SLOT(updateRecurrenceSummary()) );
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo( this );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title:tab general to-do settings", "&General" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mGeneral->initHeader( topFrame, topLayout );
  mGeneral->initTime( topFrame, topLayout );
  mGeneral->initStatus( topFrame, topLayout );

  QHBoxLayout *alarmLineLayout = new QHBoxLayout();
  alarmLineLayout->setSpacing( spacingHint() );
  topLayout->addItem( alarmLineLayout );
  mGeneral->initAlarm( topFrame, alarmLineLayout );
  alarmLineLayout->addStretch( 1 );

  mGeneral->initDescription( topFrame, topLayout );

  mGeneral->initAttachments( topFrame, topLayout );
  connect( mGeneral, SIGNAL(openURL(const KUrl&)),
           this, SLOT(openURL(const KUrl&)) );
  connect( this, SIGNAL(signalAddAttachments(const QStringList&,const QStringList&,bool)),
           mGeneral, SLOT(addAttachments(const QStringList&,const QStringList&,bool)) );

  mGeneral->enableAlarm( true );

  mGeneral->finishSetup();
}

void KOTodoEditor::setupRecurrence()
{
  mRecurrenceDialog = new KOEditorRecurrenceDialog( this );
  mRecurrenceDialog->hide();
  mRecurrence = mRecurrenceDialog->editor();
}

void KOTodoEditor::newTodo()
{
  init();
  mIncidence = Item();
  setCaption( i18nc( "@title:window", "New To-do" ) );
  loadDefaults();
}

void KOTodoEditor::setTexts( const QString &summary, const QString &description,
                             bool richDescription )
{
  if ( description.isEmpty() && summary.contains( '\n' ) ) {
    mGeneral->setDescription( summary, richDescription );
    const int pos = summary.indexOf( '\n' );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void KOTodoEditor::loadDefaults()
{
  setDates( QDateTime::currentDateTime().addDays(7), true );
  mGeneral->toggleAlarm( KOEditorConfig::instance()->defaultTodoReminders() );
}

bool KOTodoEditor::processInput()
{
  kDebug();
  if ( !validateInput() ) {
    return false;
  }

  if ( Akonadi::hasTodo( mIncidence ) ) {
    bool rc = true;
    Todo::Ptr oldTodo( Akonadi::todo( mIncidence )->clone() );
    Todo::Ptr todo( Akonadi::todo( mIncidence )->clone() );

    Akonadi::Item todoItem;
    todoItem.setPayload(todo);
    fillTodo( todoItem );

    if( *oldTodo == *todo ) {
      // Don't do anything cause no changes where done
    } else {
      if ( mChanger->beginChange( mIncidence ) ) {
        //merge multiple mIncidence->updated() calls into one
        Akonadi::todo( mIncidence )->startUpdates();
        fillTodo(mIncidence);
        rc = mChanger->changeIncidence( oldTodo, mIncidence, KOGlobals::NOTHING_MODIFIED, this );
        Akonadi::todo( mIncidence )->endUpdates();
        mChanger->endChange( mIncidence );
      } else {
        return false;
      }
    }
    return rc;
  } else {
//PENDING(AKONADI_PORT) review the newly created item will differ from mIncidence
    Todo::Ptr td( new Todo );
    td->setOrganizer( Person( KOEditorConfig::instance()->fullName(),
                              KOEditorConfig::instance()->email() ) );
    mIncidence.setPayload( td );

    Akonadi::Item tdItem;
    tdItem.setPayload(td);
    fillTodo( tdItem );

    Akonadi::Collection col = mCalSelector->currentCollection();
    if ( !mChanger->addIncidence( td, col, this ) ) {
      mIncidence = Item();
      return false;
    }
  }

  return true;
}

void KOTodoEditor::deleteTodo()
{
  if ( Akonadi::hasJournal( mIncidence ) ) {
    emit deleteIncidenceSignal( mIncidence );
  }
  emit dialogClose( mIncidence );
  reject();
}

void KOTodoEditor::setDates( const QDateTime &due, bool allDay, const Akonadi::Item &relatedEvent )
{
  mRelatedTodo = Akonadi::todo( relatedEvent );
  KDateTime::Spec timeSpec = KSystemTimeZones::local();

  // inherit some properties from parent todo
  if ( mRelatedTodo ) {
    mGeneral->setCategories( mRelatedTodo->categories() );
  }
  if ( !due.isValid() && mRelatedTodo && mRelatedTodo->hasDueDate() ) {
    mGeneral->setDefaults( mRelatedTodo->dtDue().toTimeSpec( timeSpec ).dateTime(), allDay );
  } else {
    mGeneral->setDefaults( due, allDay );
  }

  mDetails->setDefaults();
  if ( Todo::Ptr todo = Akonadi::todo( mIncidence ) ) {
    mRecurrence->setDefaults(
      todo->dtStart().toTimeSpec( timeSpec ).dateTime(), due, false );
  } else {
    mRecurrence->setDefaults(
      KDateTime::currentUtcDateTime().toTimeSpec( timeSpec ).dateTime(), due, false );
  }
}

bool KOTodoEditor::read( const Item &todoItem, const QDate &date, bool tmpl )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return false;
  }

  mGeneral->readTodo( todo.get(), date, tmpl );
  mDetails->readIncidence( todo.get() );
  mRecurrence->readIncidence( todo.get() );

  createEmbeddedURLPages( todo.get() );
  readDesignerFields( todoItem );
  return true;
}

void KOTodoEditor::fillTodo( const Akonadi::Item &item )
{
  Todo::Ptr todo = Akonadi::todo(item);
  Incidence::Ptr oldIncidence( todo->clone() );

  mGeneral->fillTodo( todo.get() );
  mDetails->fillIncidence( todo.get() );
  mRecurrence->fillIncidence( todo.get() );

  if ( *( oldIncidence->recurrence() ) != *( todo->recurrence() ) ) {
    todo->setDtDue( todo->dtDue(), true );
    if ( todo->hasStartDate() ) {
      todo->setDtStart( todo->dtStart() );
    }
  }
  writeDesignerFields( todo.get() );

  // set related incidence, i.e. parent to-do in this case.
  if ( mRelatedTodo ) {
    todo->setRelatedTo( mRelatedTodo.get() );
  }

  cancelRemovedAttendees( item );
}

bool KOTodoEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
  if ( !mRecurrence->validateInput() ) {
    return false;
  }
  if ( !mDetails->validateInput() ) {
    return false;
  }
  return true;
}

void KOTodoEditor::modified()
{
  // Play dumb, just reload the todo. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  readIncidence( mIncidence, QDate(), true );
}

void KOTodoEditor::show()
{
  fillTodo( mInitialTodoItem );
  KOIncidenceEditor::show();
}

void KOTodoEditor::updateRecurrenceSummary()
{
  Todo::Ptr todo( new Todo );

  Akonadi::Item todoItem;
  todoItem.setPayload( todo );
  fillTodo( todoItem );

  mGeneral->updateRecurrenceSummary( todo.get() );
}

#include "kotodoeditor.moc"
