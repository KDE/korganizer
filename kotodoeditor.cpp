/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#include "koprefs.h"
#include "koeditorattachments.h"
#include "kodialogmanager.h"

#include "kotodoeditor.h"

KOTodoEditor::KOTodoEditor( Calendar *calendar, QWidget *parent ) :
  KOIncidenceEditor( i18n("Edit To-Do"), calendar, parent )
{
  mTodo = 0;
  mRelatedTodo = 0;
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mTodo );
}

void KOTodoEditor::init()
{
  setupGeneral();
  setupAttendeesTab();
  setupRecurrence();
  setupAttachmentsTab();

  connect( mGeneral, SIGNAL( dateTimeStrChanged( const QString & ) ),
           mRecurrence, SLOT( setDateTimeStr( const QString & ) ) );
}

void KOTodoEditor::reload()
{
kdDebug()<<"reloading todo"<<endl;
  if ( mTodo ) readTodo( mTodo );
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo(this);

  connect(mGeneral,SIGNAL(openCategoryDialog()),mCategoryDialog,SLOT(show()));
  connect(mCategoryDialog, SIGNAL(categoriesSelected(const QString &)),
          mGeneral,SLOT(setCategories(const QString &)));
  connect(mGeneral,SIGNAL( todoCompleted( Todo * )),
                   SIGNAL( todoCompleted( Todo * ) ) );


  if (KOPrefs::instance()->mCompactDialogs) {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
    QHBoxLayout *priorityLayout = new QHBoxLayout( topLayout );
    mGeneral->initPriority(topFrame,priorityLayout);
    mGeneral->initCategories( topFrame, topLayout );
    topLayout->addStretch(1);

    QFrame *topFrame2 = addPage(i18n("Details"));

    QBoxLayout *topLayout2 = new QVBoxLayout(topFrame2);
    topLayout2->setMargin(marginHint());
    topLayout2->setSpacing(spacingHint());

    QHBoxLayout *completionLayout = new QHBoxLayout( topLayout2 );
    mGeneral->initCompletion(topFrame2,completionLayout);

    mGeneral->initAlarm(topFrame,topLayout);
    mGeneral->enableAlarm( false );

    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = addPage(i18n("&General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
    mGeneral->initStatus(topFrame,topLayout);
    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,alarmLineLayout);
    mGeneral->initDescription(topFrame,topLayout);
    QBoxLayout *detailsLayout = new QHBoxLayout(topLayout);
    mGeneral->initCategories( topFrame, detailsLayout );
    mGeneral->initSecrecy( topFrame, detailsLayout );
  }

  mGeneral->finishSetup();
}

void KOTodoEditor::setupRecurrence()
{
  QFrame *topFrame = addPage( i18n("Rec&urrence") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );
  mRecurrence->setEnabled( false );
  connect(mGeneral,SIGNAL(dueDateEditToggle( bool ) ),
          mRecurrence, SLOT( setEnabled( bool ) ) );
}

void KOTodoEditor::editIncidence(Incidence *incidence)
{
  Todo *todo=dynamic_cast<Todo*>(incidence);
  if (todo) 
  {
    init();
  
    mTodo = todo;
    readTodo(mTodo);
  }
}

void KOTodoEditor::newTodo(QDateTime due,Todo *relatedTodo,bool allDay)
{
  init();

  mTodo = 0;
  setDefaults(due,relatedTodo,allDay);
}

void KOTodoEditor::newTodo( const QString &text )
{
  init();

  mTodo = 0;

  loadDefaults();

  mGeneral->setDescription( text );

  int pos = text.find( "\n" );
  if ( pos > 0 ) {
    mGeneral->setSummary( text.left( pos ) );
    mGeneral->setDescription( text );
  } else {
    mGeneral->setSummary( text );
  }
}

void KOTodoEditor::newTodo( const QString &summary,
                              const QString &description,
                              const QString &attachment )
{
  init();

  mTodo = 0;

  loadDefaults();

  mGeneral->setSummary( summary );
  mGeneral->setDescription( description );

  if ( !attachment.isEmpty() ) {
    mAttachments->addAttachment( attachment );
  }
}

void KOTodoEditor::loadDefaults()
{
  setDefaults(QDateTime::currentDateTime().addDays(7),0,false);
}

// TODO_RK: make sure calendar()->endChange is called somewhere!
bool KOTodoEditor::processInput()
{
  if ( !validateInput() ) return false;

  if ( mTodo ) {
    Todo *oldTodo = mTodo->clone();

    writeTodo( mTodo );

    mTodo->setRevision( mTodo->revision() + 1 );

    emit incidenceChanged( oldTodo, mTodo );

    delete oldTodo;
  } else {
    mTodo = new Todo;
    mTodo->setOrganizer( KOPrefs::instance()->email() );

    writeTodo( mTodo );

    if ( !mCalendar->addTodo( mTodo ) ) {
      KODialogManager::errorSaveTodo( this );
      delete mTodo;
      mTodo = 0;
      return false;
    }

    emit incidenceAdded( mTodo );
  }

  return true;
}

void KOTodoEditor::processCancel()
{
  if ( mTodo ) {
    emit editCanceled( mTodo );
  }
}

void KOTodoEditor::deleteTodo()
{
  if (mTodo) {
    if (KOPrefs::instance()->mConfirm) {
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          emit incidenceToBeDeleted(mTodo);
          emit dialogClose(mTodo);
          mCalendar->deleteTodo(mTodo);
          emit incidenceDeleted( mTodo );
          reject();
          break;
      }
    }
    else {
      emit incidenceToBeDeleted(mTodo);
      emit dialogClose(mTodo);
      mCalendar->deleteTodo(mTodo);
      emit incidenceDeleted( mTodo );
      reject();
    }
  } else {
    reject();
  }
}

void KOTodoEditor::setDefaults( QDateTime due, Todo *relatedEvent, bool allDay )
{
  mRelatedTodo = relatedEvent;
  
  // inherit some properties from parent todo
  if ( mRelatedTodo ) {
    mGeneral->setCategories( mRelatedTodo->categoriesStr() );
    mCategoryDialog->setSelected( mRelatedTodo->categories() );
    if ( mRelatedTodo->hasDueDate() )
      mGeneral->setDefaults( mRelatedTodo->dtDue(), allDay ); 
  }
  else
    mGeneral->setDefaults( due, allDay );
  
  mDetails->setDefaults();
  mRecurrence->setDefaults(QDateTime::currentDateTime(), due, false);
  mAttachments->setDefaults();
}

void KOTodoEditor::readTodo( Todo *todo )
{
kdDebug()<<"read todo"<<endl;
  mGeneral->readTodo( todo );
  mDetails->readEvent( todo );
  mRecurrence->readIncidence( todo );
  mAttachments->readIncidence( todo );

  // categories
  mCategoryDialog->setSelected( todo->categories() );

  // TODO: We should handle read-only events here.
}

void KOTodoEditor::writeTodo( Todo *todo )
{
  // prevent mGeneral overwriting startdate recurrence
  Recurrence *r = todo->recurrence();
  QDateTime oldRecStartDate;
  if ( todo->doesRecur() )
    oldRecStartDate = r->recurStart();
  
  mGeneral->writeTodo( todo );
  mDetails->writeEvent( todo );
  mRecurrence->writeIncidence( todo );
  mAttachments->writeIncidence( todo );
    
  oldRecStartDate.isValid() ? r->setRecurStart( oldRecStartDate )
                            : r->setRecurStart( todo->dtDue() );
  
  // set related event, i.e. parent to-do in this case.
  if ( mRelatedTodo ) {
    todo->setRelatedTo( mRelatedTodo );
  }
}

bool KOTodoEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) return false;
  if ( !mRecurrence->validateInput() ) return false;
  if ( !mDetails->validateInput() ) return false;
  return true;
}

int KOTodoEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"),KGuiItem(i18n("Delete"),"editdelete"));
}

void KOTodoEditor::modified (int /*modification*/)
{
  // Play dump, just reload the todo. This dialog has become so complicated that
  // there is no point in trying to be smart here...
  reload();
}

void KOTodoEditor::slotLoadTemplate()
{
  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  Todo *todo = new Todo;
  QString templateName = loadTemplate( &cal, todo->type(),
                                       KOPrefs::instance()->mTodoTemplates );
  delete todo;
  if ( templateName.isEmpty() ) {
    return;
  }

  Todo::List todos = cal.todos();
  if ( todos.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template '%1' does not contain a valid todo.")
        .arg( templateName ) );
  } else {
    readTodo( todos.first() );
  }
}

void KOTodoEditor::saveTemplate( const QString &templateName )
{
  Todo *todo = new Todo;
  writeTodo( todo );
  saveAsTemplate( todo, templateName );
}

#include "kotodoeditor.moc"
