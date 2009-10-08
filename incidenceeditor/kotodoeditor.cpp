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
#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"
#ifdef AKONADI_PORT_DISABLED
#include "koprefs.h"
#endif
#include "koeditorattachments.h"

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

using namespace KOrg;

KOTodoEditor::KOTodoEditor( CalendarBase *calendar, QWidget *parent )
  : KOIncidenceEditor( QString(), calendar, parent ),
    mTodo( 0 ), mCalendar( 0 ), mRelatedTodo( 0 )
{
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mTodo );
}

bool KOTodoEditor::incidenceModified()
{
  Todo *newTodo = 0;
  Todo *oldTodo = 0;
  bool modified;

  if ( mTodo ) { // modification
    oldTodo = mTodo->clone();
  } else { // new one
    // don't remove .clone(), it's on purpose, clone() strips relation attributes
    // if you compare a non-cloned parent to-do with a cloned to-do you will always
    // get false, so we use clone() in both cases.
    oldTodo = mInitialTodo.clone();
  }

  newTodo = oldTodo->clone();
  fillTodo( newTodo );

  modified = !( *newTodo == *oldTodo );

  delete newTodo;
  delete oldTodo;

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
  if ( mTodo ) {
    readTodo( mTodo, true );
  }
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo( mCalendar, this );

#ifdef AKONADI_PORT_DISABLED
  const bool compactDialogs = KOPrefs::instance()->mCompactDialogs;
#else
  const bool compactDialogs = false;
#endif
  if ( compactDialogs ) {
    QFrame *topFrame = new QFrame();
    addPage( topFrame, i18nc( "@title:tab general to-do settings","General" ) );

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );
    topLayout->setMargin( marginHint() );
    topLayout->setSpacing( spacingHint() );

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime( topFrame, topLayout );
    QHBoxLayout *priorityLayout = new QHBoxLayout();
    topLayout->addItem( priorityLayout );
    mGeneral->initPriority( topFrame, priorityLayout );
    topLayout->addStretch( 1 );

    QFrame *topFrame2 = new QFrame();
    addPage( topFrame2, i18nc( "@title:tab", "Details" ) );

    QBoxLayout *topLayout2 = new QVBoxLayout( topFrame2 );
    topLayout2->setMargin( marginHint() );
    topLayout2->setSpacing( spacingHint() );

    QHBoxLayout *completionLayout = new QHBoxLayout();
    topLayout2->addItem( completionLayout );
    mGeneral->initCompletion( topFrame2, completionLayout );

    mGeneral->initAlarm( topFrame, topLayout );

    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription( topFrame2, topLayout2 );
  } else {
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
  }
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

void KOTodoEditor::editIncidence( Incidence *incidence, KOrg::CalendarBase *calendar )
{
  Todo *todo = dynamic_cast<Todo*>( incidence );
  if ( todo ) {
    init();

    mTodo = todo;
    mCalendar = calendar;
    readTodo( mTodo, false );
  }
#ifdef AKONADI_PORT_DISABLED
  setCaption( i18nc( "@title:window",
                     "Edit To-do: %1",
                     IncidenceFormatter::resourceString( calendar, incidence ) ) );
#else // AKONADI_PORT_DISABLED
  setCaption( i18nc( "@title:window",
                     "Edit To-do: %1",
                     QLatin1String("AKONADI_PORT_DISABLED") ) );

#endif // AKONADI_PORT_DISABLED
}

void KOTodoEditor::newTodo()
{
  init();
  mTodo = 0;
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
  setDates( QDateTime::currentDateTime().addDays(7), true, 0 );
#ifdef AKONADI_PORT_DISABLED
  mGeneral->toggleAlarm( KOPrefs::instance()->defaultTodoReminders() );
#endif
}

bool KOTodoEditor::processInput()
{
  if ( !validateInput() ) {
    return false;
  }

  if ( mTodo ) {
    bool rc = true;
    Todo *oldTodo = mTodo->clone();
    Todo *todo = mTodo->clone();

    fillTodo( todo );

    if( *mTodo == *todo ) {
      // Don't do anything cause no changes where done
    } else {
      mTodo->startUpdates(); //merge multiple mTodo->updated() calls into one
      fillTodo( mTodo );
#ifdef AKONADI_PORT_DISABLED
      rc = mChanger->changeIncidence( oldTodo, mTodo );
#endif
      mTodo->endUpdates();
    }
    delete todo;
    delete oldTodo;
    return rc;
  } else {
    mTodo = new Todo;
#ifdef AKONADI_PORT_DISABLED
    mTodo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                 KOPrefs::instance()->email() ) );

    fillTodo( mTodo );

    if ( !mChanger->addIncidence( mTodo, this ) ) {
      delete mTodo;
      mTodo = 0;
      return false;
    }
#endif
  }

  return true;

}

void KOTodoEditor::deleteTodo()
{
  if ( mTodo ) {
    emit deleteIncidenceSignal( mTodo );
  }
  emit dialogClose( mTodo );
  reject();
}

void KOTodoEditor::setDates( const QDateTime &due, bool allDay, Todo *relatedEvent )
{
  mRelatedTodo = relatedEvent;
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
  if ( mTodo ) {
    mRecurrence->setDefaults(
      mTodo->dtStart().toTimeSpec( timeSpec ).dateTime(), due, false );
  } else {
    mRecurrence->setDefaults(
      KDateTime::currentUtcDateTime().toTimeSpec( timeSpec ).dateTime(), due, false );
  }
}

void KOTodoEditor::readTodo( Todo *todo, bool tmpl )
{
  if ( !todo ) {
    return;
  }

  mGeneral->readTodo( todo, tmpl );
  mDetails->readIncidence( todo );
  mRecurrence->readIncidence( todo );

  createEmbeddedURLPages( todo );
  readDesignerFields( todo );
}

void KOTodoEditor::fillTodo( Todo *todo )
{
  Incidence *oldIncidence = todo->clone();

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
    todo->setRelatedTo( mRelatedTodo );
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
#endif

QStringList &KOTodoEditor::templates() const
{
#ifdef AKONADI_PORT_DISABLED
  return KOPrefs::instance()->mTodoTemplates;
#endif
}

void KOTodoEditor::show()
{
  fillTodo( &mInitialTodo );
  KOIncidenceEditor::show();
}

#include "kotodoeditor.moc"
