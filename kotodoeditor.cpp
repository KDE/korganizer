/*
    This file is part of KOrganizer.
    Copyright (c) 1997, 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <kdatepik.h>
#include <kiconloader.h>
#include <klocale.h>

#include "categoryselectdialog.h"
#include "koprefs.h"

#include "kotodoeditor.h"
#include "kotodoeditor.moc"

KOTodoEditor::KOTodoEditor(Calendar *calendar) :
  KOIncidenceEditor(i18n("Edit To-Do"),calendar)
{
  mTodo = 0;
  mRelatedTodo = 0;
}

KOTodoEditor::~KOTodoEditor()
{
}

void KOTodoEditor::init()
{
  setupGeneral();
  setupAttendeesTab();
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo(this);
 
  connect(mGeneral,SIGNAL(openCategoryDialog()),mCategoryDialog,SLOT(show()));
  connect(mCategoryDialog, SIGNAL(categoriesSelected(const QString &)),
          mGeneral,SLOT(setCategories(const QString &)));

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
    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout2);
    mGeneral->initAlarm(topFrame2,alarmLineLayout);
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
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

void KOTodoEditor::editTodo(Todo *todo)
{
  init();

  mTodo = todo;
  readTodo(mTodo);
}

void KOTodoEditor::newTodo(QDateTime due,Todo *relatedTodo,bool allDay)
{
  init();

  mTodo = 0;
  setDefaults(due,relatedTodo,allDay);

  enableButton(User1,false);
}

void KOTodoEditor::slotDefault()
{
  setDefaults(QDateTime::currentDateTime().addDays(7),0,false);
}

bool KOTodoEditor::processInput()
{
  if (!validateInput()) return false;

  Todo *todo = 0;

  if (mTodo) todo = mTodo;
  else {
    todo = new Todo;
    todo->setOrganizer(KOPrefs::instance()->email());
  }

  writeTodo(todo);

  if (mTodo) {
    todo->setRevision(todo->revision()+1);
    emit todoChanged(todo);
  } else {
    mCalendar->addTodo(todo);
    mTodo = todo;
    emit todoAdded(todo);
  }

  return true;
}

void KOTodoEditor::slotUser1()
{
  if (mTodo) {
    emit todoToBeDeleted(mTodo);
    mCalendar->deleteTodo(mTodo);
    emit todoDeleted();
    reject();
  } else {
    reject();
  }
}

void KOTodoEditor::setDefaults(QDateTime due,Todo *relatedEvent,bool allDay)
{
  mRelatedTodo = relatedEvent;

  mGeneral->setDefaults(due,allDay);
  mDetails->setDefaults();
}

void KOTodoEditor::readTodo(Todo *todo)
{
  mGeneral->readTodo(todo);
  mDetails->readEvent(todo);

  // We should handle read-only events here.
}

void KOTodoEditor::writeTodo(Todo *event)
{
  mGeneral->writeTodo(event);
  mDetails->writeEvent(event);

  // set related event, i.e. parent to-do in this case.
  if (mRelatedTodo) {
    event->setRelatedTo(mRelatedTodo);
  }
}

bool KOTodoEditor::validateInput()
{
  if (!mGeneral->validateInput()) return false;
  if (!mDetails->validateInput()) return false;
  return true;
}
