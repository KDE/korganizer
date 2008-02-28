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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#include "koprefs.h"
#include "koeditorattachments.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"

#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"

#include "kotodoeditor.h"
#include "kocore.h"

KOTodoEditor::KOTodoEditor( Calendar *calendar, QWidget *parent ) :
  KOIncidenceEditor( QString::null, calendar, parent )
{
  mTodo = 0;
  mCalendar = 0;
  mRelatedTodo = 0;
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mTodo );
}

void KOTodoEditor::init()
{
  kdDebug(5850) << k_funcinfo << endl;
  setupGeneral();
  setupRecurrence();
  setupAttendeesTab();
//  setupAttachmentsTab();

  connect( mGeneral, SIGNAL( dateTimeStrChanged( const QString & ) ),
           mRecurrence, SLOT( setDateTimeStr( const QString & ) ) );
  connect( mGeneral, SIGNAL( signalDateTimeChanged( const QDateTime &, const QDateTime & ) ),
           mRecurrence, SLOT( setDateTimes( const QDateTime &, const QDateTime & ) ) );

  connect( mGeneral, SIGNAL( openCategoryDialog() ),
           SIGNAL( editCategories() ) );

  connect( mDetails, SIGNAL(updateAttendeeSummary(int)),
           mGeneral, SLOT(updateAttendeeSummary(int)) );
}

void KOTodoEditor::reload()
{
  if ( mTodo ) readTodo( mTodo, mCalendar );
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo(this);

  if (KOPrefs::instance()->mCompactDialogs) {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);
    QHBoxLayout *priorityLayout = new QHBoxLayout( topLayout );
    mGeneral->initPriority(topFrame,priorityLayout);
    topLayout->addStretch(1);

    QFrame *topFrame2 = addPage(i18n("Details"));

    QBoxLayout *topLayout2 = new QVBoxLayout(topFrame2);
    topLayout2->setMargin(marginHint());
    topLayout2->setSpacing(spacingHint());

    QHBoxLayout *completionLayout = new QHBoxLayout( topLayout2 );
    mGeneral->initCompletion(topFrame2,completionLayout);

    mGeneral->initAlarm(topFrame,topLayout);
 
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = addPage(i18n("&General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);
    mGeneral->initStatus(topFrame,topLayout);
    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,alarmLineLayout);
    alarmLineLayout->addStretch( 1 );
    mGeneral->initDescription(topFrame,topLayout);
    mGeneral->initAttachments(topFrame,topLayout);
    connect( mGeneral, SIGNAL( openURL( const KURL& ) ),
             this, SLOT( openURL( const KURL& ) ) );
    connect( this, SIGNAL( signalAddAttachments( const QStringList&, const QStringList&, bool ) ),
             mGeneral, SLOT( addAttachments( const QStringList&, const QStringList&, bool ) ) );
  }
  mGeneral->enableAlarm( true );

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

void KOTodoEditor::editIncidence(Incidence *incidence, Calendar *calendar)
{
  kdDebug(5850) << k_funcinfo << endl;
  Todo *todo=dynamic_cast<Todo*>(incidence);
  if (todo)
  {
    init();

    mTodo = todo;
    mCalendar = calendar;
    readTodo( mTodo, mCalendar );
  }

  setCaption( i18n("Edit To-do") );
}

void KOTodoEditor::newTodo()
{
  kdDebug(5850) << k_funcinfo << endl;
  init();
  mTodo = 0;
  mCalendar = 0;
  setCaption( i18n("New To-do") );
  loadDefaults();
}

void KOTodoEditor::setTexts( const QString &summary, const QString &description )
{
  if ( description.isEmpty() && summary.contains("\n") ) {
    mGeneral->setDescription( summary );
    int pos = summary.find( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description );
  }
}



void KOTodoEditor::loadDefaults()
{
  kdDebug(5850) << k_funcinfo << endl;
  setDates( QDateTime::currentDateTime().addDays(7), true, 0 );
  mGeneral->toggleAlarm( true );
}

bool KOTodoEditor::processInput()
{
  if ( !validateInput() ) return false;

  if ( mTodo ) {
    bool rc = true;
    Todo *oldTodo = mTodo->clone();
    Todo *todo = mTodo->clone();

    kdDebug(5850) << "KOTodoEditor::processInput() write event." << endl;
    writeTodo( todo );
    kdDebug(5850) << "KOTodoEditor::processInput() event written." << endl;

    if( *mTodo == *todo )
      // Don't do anything
      kdDebug(5850) << "Todo not changed\n";
    else {
      kdDebug(5850) << "Todo changed\n";
      //IncidenceChanger::assignIncidence( mTodo, todo );
      writeTodo( mTodo );
      mChanger->changeIncidence( oldTodo, mTodo );
    }
    delete todo;
    delete oldTodo;
    return rc;

  } else {
    mTodo = new Todo;
    mTodo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );

    writeTodo( mTodo );

    if ( !mChanger->addIncidence( mTodo, this ) ) {
      delete mTodo;
      mTodo = 0;
      return false;
    }
  }

  return true;

}

void KOTodoEditor::deleteTodo()
{
  if (mTodo)
    emit deleteIncidenceSignal( mTodo );
  emit dialogClose(mTodo);
  reject();
}

void KOTodoEditor::setDates( const QDateTime &due, bool allDay, Todo *relatedEvent )
{
  mRelatedTodo = relatedEvent;

  // inherit some properties from parent todo
  if ( mRelatedTodo ) {
    mGeneral->setCategories( mRelatedTodo->categories() );
  }
  if ( !due.isValid() && mRelatedTodo && mRelatedTodo->hasDueDate() ) {
    mGeneral->setDefaults( mRelatedTodo->dtDue(), allDay );
  } else {
    mGeneral->setDefaults( due, allDay );
  }

  mDetails->setDefaults();
  if ( mTodo )
    mRecurrence->setDefaults( mTodo->dtStart(), due, false );
  else
    mRecurrence->setDefaults( QDateTime::currentDateTime(), due, false );
}

void KOTodoEditor::readTodo( Todo *todo, Calendar *calendar )
{
  if ( !todo ) return;
//   mRelatedTodo = todo->relatedTo();
  kdDebug(5850)<<"read todo"<<endl;
  mGeneral->readTodo( todo, calendar );
  mDetails->readEvent( todo );
  mRecurrence->readIncidence( todo );

  createEmbeddedURLPages( todo );
  readDesignerFields( todo );
}

void KOTodoEditor::writeTodo( Todo *todo )
{
  Incidence *oldIncidence = todo->clone();

  mRecurrence->writeIncidence( todo );
  mGeneral->writeTodo( todo );
  mDetails->writeEvent( todo );

  if ( *(oldIncidence->recurrence()) != *(todo->recurrence() ) ) {
    todo->setDtDue( todo->dtDue(), true );
    if ( todo->hasStartDate() )
      todo->setDtStart( todo->dtStart() );
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
  if ( !mGeneral->validateInput() ) return false;
  if ( !mRecurrence->validateInput() ) return false;
  if ( !mDetails->validateInput() ) return false;
  return true;
}

int KOTodoEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"), KStdGuiItem::del() );
}

void KOTodoEditor::modified (int /*modification*/)
{
  // Play dump, just reload the todo. This dialog has become so complicated that
  // there is no point in trying to be smart here...
  reload();
}

void KOTodoEditor::loadTemplate( /*const*/ CalendarLocal& cal )
{
  Todo::List todos = cal.todos();
  if ( todos.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid to-do.") );
  } else {
    readTodo( todos.first(), 0 );
  }
}

void KOTodoEditor::slotSaveTemplate( const QString &templateName )
{
  Todo *todo = new Todo;
  writeTodo( todo );
  saveAsTemplate( todo, templateName );
}

QStringList& KOTodoEditor::templates() const
{
  return KOPrefs::instance()->mTodoTemplates;
}

#include "kotodoeditor.moc"
