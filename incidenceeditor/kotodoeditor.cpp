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
#include "koeditorconfig.h"
#include "koeditorattachments.h"
#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"
#include "korganizer/incidencechangerbase.h"

#include <KCal/IncidenceFormatter>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksystemtimezone.h>

#include <QFrame>
#include <QPixmap>
#include <QLayout>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <akonadi/kcal/utils.h>

using namespace Akonadi;
using namespace KOrg;

KOTodoEditor::KOTodoEditor( CalendarBase *calendar, QWidget *parent )
  : KOIncidenceEditor( QString(), calendar, parent ),
    mTodo(), mCalendar( 0 ), mRelatedTodo()
{
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mTodo );
}

bool KOTodoEditor::incidenceModified()
{
  const Todo::Ptr todo = Akonadi::todo( mTodo );
  Todo::Ptr oldTodo;
  bool modified;

  if ( todo ) { // modification
    oldTodo.reset( todo->clone() );
  } else { // new one
    // don't remove .clone(), it's on purpose, clone() strips relation attributes
    // if you compare a non-cloned parent to-do with a cloned to-do you will always
    // get false, so we use clone() in both cases.
    oldTodo.reset( mInitialTodo.clone() );
  }

  Todo::Ptr newTodo( oldTodo->clone() );
  fillTodo( newTodo.get() );

  modified = !( *newTodo == *oldTodo );

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
}

void KOTodoEditor::reload()
{
  readTodo( mTodo, true );
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo( mCalendar, this );

  QFrame *topFrame = new QFrame();
  addPage( topFrame, i18nc( "@title:tab general to-do settings", "&General" ) );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );

  mGeneral->initHeader( topFrame, topLayout );
  mGeneral->initTime( topFrame, topLayout );
  mGeneral->initStatus( topFrame, topLayout );

  QBoxLayout *alarmLineLayout = new QHBoxLayout();
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
  QFrame *topFrame = new QFrame();
  addPage( topFrame, i18nc( "@title:tab", "Rec&urrence" ) );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );

  mRecurrence->setEnabled( false );
  connect( mGeneral,SIGNAL(dueDateEditToggle(bool)),
           mRecurrence, SLOT(setEnabled(bool)) );
}

void KOTodoEditor::editIncidence( const Item &item, KOrg::CalendarBase *calendar )
{
  const Todo::Ptr todo = Akonadi::todo( item );
  Q_ASSERT( todo );
  init();

  mTodo = item;
  mCalendar = calendar;
  readTodo( item, false );
  setCaption( i18nc( "@title:window", "Edit To-do: %1", todo->summary() ) );
}

void KOTodoEditor::newTodo()
{
  init();
  mTodo = Item();
  mCalendar = 0;
  setCaption( i18nc( "@title:window", "New To-do" ) );
  loadDefaults();
}

void KOTodoEditor::setTexts( const QString &summary, const QString &description,
                             bool richDescription )
{
  if ( description.isEmpty() && summary.contains( "\n" ) ) {
    mGeneral->setDescription( summary, richDescription );
    int pos = summary.indexOf( "\n" );
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
  if ( !validateInput() ) {
    return false;
  }
//#ifdef AKONADI_PORT_DISABLED //incidenceChanger

  if ( Akonadi::hasTodo( mTodo ) ) {

    bool rc = true;
    Todo::Ptr oldTodo( Akonadi::todo( mTodo )->clone() );
    Todo::Ptr todo( Akonadi::todo( mTodo )->clone() );

    fillTodo( todo.get() );

    if( *oldTodo == *todo ) {
      // Don't do anything cause no changes where done
    } else {
      Akonadi::todo( mTodo )->startUpdates(); //merge multiple mTodo->updated() calls into one
      
      fillTodo( Akonadi::todo( mTodo ).get() );
      rc = mChanger->changeIncidence( oldTodo, mTodo );
      Akonadi::todo( mTodo )->endUpdates();
    }
    return rc;
  } else {
//PENDING(AKONADI_PORT) review the newly created item will differ from mTodo
    Todo::Ptr td( new Todo );
    td->setOrganizer( Person( KOEditorConfig::instance()->fullName(),
                              KOEditorConfig::instance()->email() ) );
    mTodo.setPayload( td );

    fillTodo( td.get() );
    if ( !mChanger->addIncidence( td, this ) ) {
      mTodo = Item();
      return false;
    }
  }

  return true;
//#else
//  return false;
//#endif
}

void KOTodoEditor::deleteTodo()
{
  if ( Akonadi::hasJournal( mTodo ) ) {
    emit deleteIncidenceSignal( mTodo );
  }
  emit dialogClose( mTodo );
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
  if ( Todo::Ptr todo = Akonadi::todo( mTodo ) ) {
    mRecurrence->setDefaults(
      todo->dtStart().toTimeSpec( timeSpec ).dateTime(), due, false );
  } else {
    mRecurrence->setDefaults(
      KDateTime::currentUtcDateTime().toTimeSpec( timeSpec ).dateTime(), due, false );
  }
}

void KOTodoEditor::readTodo( const Item &todoItem, bool tmpl )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return;
  }

  mGeneral->readTodo( todo.get(), tmpl );
  mDetails->readIncidence( todo.get() );
  mRecurrence->readIncidence( todo.get() );

  createEmbeddedURLPages( todo.get() );
  readDesignerFields( todoItem );
}

void KOTodoEditor::fillTodo( Todo* todo )
{
  Incidence::Ptr oldIncidence( todo->clone() );

  mGeneral->fillTodo( todo );
  mDetails->fillIncidence( todo );
  mRecurrence->fillIncidence( todo );

  if ( *( oldIncidence->recurrence() ) != *( todo->recurrence() ) ) {
    todo->setDtDue( todo->dtDue(), true );
    if ( todo->hasStartDate() ) {
      todo->setDtStart( todo->dtStart() );
    }
  }
  writeDesignerFields( todo );

  // set related incidence, i.e. parent to-do in this case.
  if ( mRelatedTodo ) {
    todo->setRelatedTo( mRelatedTodo.get() );
  }

  cancelRemovedAttendees( todo );
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

void KOTodoEditor::modified( int modification )
{
  Q_UNUSED( modification );

  // Play dumb, just reload the todo. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  reload();
}

#if 0 //AKONADI_PORT_DISABLED
void KOTodoEditor::loadTemplate( CalendarLocal &cal )
{
  Todo::List todos = cal.todos();
  if ( todos.count() == 0 ) {
    KMessageBox::error( this, i18nc( "@info", "Template does not contain a valid to-do." ) );
  } else {
    readTodo( todos.first(), true );
  }
}

void KOTodoEditor::slotSaveTemplate( const QString &templateName )
{
  Todo *todo = new Todo;
  fillTodo( todo );
  saveAsTemplate( todo, templateName );
}

QStringList KOTodoEditor::templates() const
{
  return KOPrefs::instance()->mTodoTemplates;
}
#endif

void KOTodoEditor::show()
{
  fillTodo( &mInitialTodo );
  KOIncidenceEditor::show();
}

#include "kotodoeditor.moc"
