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
#include "searchdialog.h"
#include "filtereditdialog.h"
#ifndef KORG_NOPLUGINS
#include "plugindialog.h"
#endif
#ifndef KORG_NOARCHIVE
#include "archivedialog.h"
#endif

#include "kodialogmanager.h"
#include "kodialogmanager.moc"

KODialogManager::KODialogManager( CalendarView *mainView ) :
  QObject(), mMainView( mainView )
{
  mOutgoingDialog = 0;
  mIncomingDialog = 0;
  mOptionsDialog = 0;
  mSearchDialog = 0;
  mArchiveDialog = 0;
  mFilterEditDialog = 0;
  mPluginDialog = 0;

  mCategoryEditDialog = new CategoryEditDialog(mMainView);
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

void KODialogManager::showSearchDialog()
{
  if (!mSearchDialog) {
    mSearchDialog = new SearchDialog(mMainView->calendar(),mMainView);
    connect(mSearchDialog,SIGNAL(showEventSignal(Event *)),
            mMainView,SLOT(showEvent(Event *)));
    connect(mSearchDialog,SIGNAL(editEventSignal(Event *)),
            mMainView,SLOT(editEvent(Event *)));
    connect(mSearchDialog,SIGNAL(deleteEventSignal(Event *)),
            mMainView, SLOT(deleteEvent(Event *)));
    connect(mMainView,SIGNAL(closingDown()),mSearchDialog,SLOT(reject()));
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void KODialogManager::showArchiveDialog()
{
#ifndef KORG_NOARCHIVE
  if (!mArchiveDialog) {
    mArchiveDialog = new ArchiveDialog(mMainView->calendar(),mMainView);
    connect(mArchiveDialog,SIGNAL(eventsDeleted()),
            mMainView,SLOT(updateView()));
  }
  mArchiveDialog->show();
  mArchiveDialog->raise();

  // Workaround.
  QApplication::restoreOverrideCursor();
#endif
}

void KODialogManager::showFilterEditDialog(QPtrList<CalFilter> filters)
{
  if (!mFilterEditDialog) {
    mFilterEditDialog = new FilterEditDialog(&filters,mMainView);
    connect(mFilterEditDialog,SIGNAL(filterChanged()),
            mMainView,SLOT(filterEdited()));
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

void KODialogManager::showPluginDialog()
{
#ifndef KORG_NOPLUGINS
  if (!mPluginDialog) {
    mPluginDialog = new PluginDialog(mMainView);
    connect(mPluginDialog,SIGNAL(configChanged()),
            mMainView,SLOT(updateConfig()));
  }
  mPluginDialog->show();
  mPluginDialog->raise();
#endif
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

void KODialogManager::updateSearchDialog()
{
  if (mSearchDialog) mSearchDialog->updateView();
}
