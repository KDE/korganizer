/*
  This file is part of KOrganizer.

  Copyright (c) 2001
  Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include "calendarview.h"

#include "incomingdialog.h"
#include "outgoingdialog.h"
#include "koprefsdialog.h"
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "categoryeditdialog.h"

#include "kodialogmanager.h"
#include "kodialogmanager.moc"

KODialogManager::KODialogManager( CalendarView *mainView ) :
  QObject(), mMainView( mainView )
{
  mOutgoingDialog = 0;
  mIncomingDialog = 0;
  mOptionsDialog = 0;

  mCategoryEditDialog = new CategoryEditDialog();
}

KODialogManager::~KODialogManager()
{
}

OutgoingDialog *KODialogManager::outgoingDialog()
{
  createOutgoingDialog();
  return mOutgoingDialog;
}

void KODialogManager::createOutgoingDialog()
{
  if (!mOutgoingDialog) {
    mOutgoingDialog = new OutgoingDialog(mMainView->calendar(),mMainView);
    connect(mOutgoingDialog,SIGNAL(numMessagesChanged(int)),
            mMainView,SIGNAL(numOutgoingChanged(int)));
  }
}

void KODialogManager::showOptionsDialog()
{
  if (!mOptionsDialog) {
    mOptionsDialog = new KOPrefsDialog(mMainView);
    mOptionsDialog->readConfig();
    connect(mOptionsDialog,SIGNAL(configChanged()),
            mMainView,SLOT(updateConfig()));
    connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
            mOptionsDialog,SLOT(updateCategories()));
  }

  mOptionsDialog->readConfig();
  mOptionsDialog->show();
}

void KODialogManager::showOutgoingDialog()
{
  createOutgoingDialog();
  mOutgoingDialog->show();
  mOutgoingDialog->raise();
}

void KODialogManager::showIncomingDialog()
{
  if (!mIncomingDialog) {
    mIncomingDialog = new IncomingDialog(mMainView->calendar(),mMainView);
    connect(mIncomingDialog,SIGNAL(numMessagesChanged(int)),
            mMainView,SIGNAL(numIncomingChanged(int)));
    connect(mIncomingDialog,SIGNAL(calendarUpdated()),
            mMainView,SLOT(updateView()));
  }

  mIncomingDialog->show();
  mIncomingDialog->raise();
}

void KODialogManager::showCategoryEditDialog()
{
  mCategoryEditDialog->show();
}

KOEventEditor *KODialogManager::getEventEditor()
{
  KOEventEditor *eventEditor = new KOEventEditor(mMainView->calendar());

  connect(eventEditor,SIGNAL(eventAdded(Event *)),
          mMainView,SLOT(eventAdded(Event *)));
  connect(eventEditor,SIGNAL(eventChanged(Event *)),
          mMainView,SLOT(eventChanged(Event *)));
  connect(eventEditor,SIGNAL(eventDeleted()),
          mMainView,SLOT(eventDeleted()));

  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          eventEditor,SLOT(updateCategoryConfig()));
  connect(eventEditor,SIGNAL(editCategories()),
          mCategoryEditDialog,SLOT(show()));

  connect(mMainView,SIGNAL(closingDown()),eventEditor,SLOT(reject()));

  return eventEditor;
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
  KOTodoEditor *todoEditor = new KOTodoEditor(mMainView->calendar());

  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          todoEditor,SLOT(updateCategoryConfig()));
  connect(todoEditor,SIGNAL(editCategories()),mCategoryEditDialog,SLOT(show()));

  connect(todoEditor,SIGNAL(todoAdded(Todo *)),
          mMainView,SLOT(updateTodoViews()));
  connect(todoEditor,SIGNAL(todoChanged(Todo *)),
          mMainView,SLOT(updateTodoViews()));
  connect(todoEditor,SIGNAL(todoDeleted()),
          mMainView,SLOT(updateTodoViews()));

  connect(mMainView,SIGNAL(closingDown()),todoEditor,SLOT(reject()));

  return todoEditor;
}
