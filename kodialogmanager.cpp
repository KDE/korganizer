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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kcmultidialog.h>
#include <ksettings/dialog.h>

#include <libkdepim/categoryeditdialog.h>

#include "calendarview.h"
#include "incomingdialog.h"
#include "outgoingdialog.h"
#include "koprefsdialog.h"
#include "koeventeditor.h"
#include "koprefs.h"
#include "kotodoeditor.h"
#include "searchdialog.h"
#include "filtereditdialog.h"
#ifndef KORG_NOPLUGINS
#include "plugindialog.h"
#endif
#ifndef KORG_NOARCHIVE
#include "archivedialog.h"
#endif
#include "koviewmanager.h"
#include "koagendaview.h"

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

  mCategoryEditDialog = new KPIM::CategoryEditDialog(KOPrefs::instance(),mMainView);
  KOGlobals::fitDialogToScreen( mCategoryEditDialog );
}

KODialogManager::~KODialogManager()
{
  delete mOutgoingDialog;
  delete mIncomingDialog;
  delete mOptionsDialog;
  delete mSearchDialog;
#ifndef KORG_NOARCHIVE
  delete mArchiveDialog;
#endif
  delete mFilterEditDialog;
#ifndef KORG_NOPLUGINS
  delete mPluginDialog;
#endif
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
    if (mIncomingDialog) mIncomingDialog->setOutgoingDialog(mOutgoingDialog);
    connect(mOutgoingDialog,SIGNAL(numMessagesChanged(int)),
            mMainView,SIGNAL(numOutgoingChanged(int)));
  }
}

void KODialogManager::showOptionsDialog()
{
  if (!mOptionsDialog) {
#if 0
    mOptionsDialog = new KConfigureDialog();
//    mOptionsDialog = new KConfigureDialog( KConfigureDialog::Configurable );
//    mOptionsDialog = new KConfigureDialog( mMainView );
    connect( mOptionsDialog->dialog(),
             SIGNAL( configCommitted( const QCString & ) ),
             mMainView, SLOT( updateConfig() ) );
#else
    mOptionsDialog = new KCMultiDialog( "PIM", mMainView,
                                        "KorganizerPreferences" );
    connect( mOptionsDialog, SIGNAL( configCommitted( const QCString & ) ),
             mMainView, SLOT( updateConfig() ) );
#if 0
    connect( mOptionsDialog, SIGNAL( applyClicked() ),
             mMainView, SLOT( updateConfig() ) );
    connect( mOptionsDialog, SIGNAL( okClicked() ),
             mMainView, SLOT( updateConfig() ) );
    // TODO Find a way to do this with KCMultiDialog
    connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
            mOptionsDialog,SLOT(updateCategories()));
#endif

    QStringList modules;

    modules.append( "configmain.desktop" );
    modules.append( "configtime.desktop" );
    modules.append( "configviews.desktop" );
    modules.append( "configfonts.desktop" );
    modules.append( "configcolors.desktop" );
    modules.append( "configprinting.desktop" );
    modules.append( "configgroupscheduling.desktop" );
    modules.append( "configgroupautomation.desktop" );

    // add them all
    QStringList::iterator mit;
    for ( mit = modules.begin(); mit != modules.end(); ++mit )
      mOptionsDialog->addModule( *mit );
#endif
  }

  mOptionsDialog->show();
  mOptionsDialog->raise();
}

void KODialogManager::showOutgoingDialog()
{
  createOutgoingDialog();
  mOutgoingDialog->show();
  mOutgoingDialog->raise();
}

IncomingDialog *KODialogManager::incomingDialog()
{
  createOutgoingDialog();
  if (!mIncomingDialog) {
    mIncomingDialog = new IncomingDialog(mMainView->calendar(),mOutgoingDialog,mMainView);
    connect(mIncomingDialog,SIGNAL(numMessagesChanged(int)),
            mMainView,SIGNAL(numIncomingChanged(int)));
    connect(mIncomingDialog,SIGNAL(calendarUpdated()),
            mMainView,SLOT(updateView()));
  }
  return mIncomingDialog;
}

void KODialogManager::createIncomingDialog()
{
  createOutgoingDialog();
  if (!mIncomingDialog) {
    mIncomingDialog = new IncomingDialog(mMainView->calendar(),mOutgoingDialog,mMainView);
    connect(mIncomingDialog,SIGNAL(numMessagesChanged(int)),
            mMainView,SIGNAL(numIncomingChanged(int)));
    connect(mIncomingDialog,SIGNAL(calendarUpdated()),
            mMainView,SLOT(updateView()));
  }
}

void KODialogManager::showIncomingDialog()
{
  createIncomingDialog();
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
    connect(mSearchDialog,SIGNAL(showIncidenceSignal(Incidence *)),
            mMainView,SLOT(showIncidence(Incidence *)));
    connect(mSearchDialog,SIGNAL(editIncidenceSignal(Incidence *)),
            mMainView,SLOT(editIncidence(Incidence *)));
    connect(mSearchDialog,SIGNAL(deleteIncidenceSignal(Incidence *)),
            mMainView, SLOT(deleteIncidence(Incidence *)));
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

void KODialogManager::showFilterEditDialog(QPtrList<CalFilter> *filters)
{
  if (!mFilterEditDialog) {
    mFilterEditDialog = new FilterEditDialog(filters,mMainView);
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
  KOEventEditor *eventEditor = new KOEventEditor( mMainView->calendar(),
                                                  mMainView );

  connect( eventEditor, SIGNAL( eventAdded( Event * ) ),
           mMainView, SLOT( eventAdded( Event * ) ) );
  connect( eventEditor, SIGNAL( eventChanged( Event *, Event * ) ),
           mMainView, SLOT( eventChanged( Event *, Event * ) ) );
  connect( eventEditor, SIGNAL( eventDeleted( Event * ) ),
           mMainView, SLOT( eventDeleted( Event * ) ) );
  connect( eventEditor, SIGNAL( deleteAttendee( Incidence * ) ),
           mMainView, SLOT( schedule_cancel( Incidence * ) ) );

  connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
           eventEditor, SLOT( updateCategoryConfig() ) );
  connect( eventEditor, SIGNAL( editCategories() ),
           mCategoryEditDialog, SLOT( show() ) );
  connect( eventEditor, SIGNAL( dialogClose( Incidence * ) ),
           mMainView, SLOT( dialogClosing( Incidence * ) ) );

  connect( mMainView, SIGNAL( closingDown() ), eventEditor, SLOT( reject() ) );

  return eventEditor;
}

void KODialogManager::connectTypeAhead( KOEventEditor *editor,
                                        KOAgendaView *agenda )
{
  if ( editor && agenda ) {
    agenda->setTypeAheadReceiver( editor->typeAheadReceiver() );
    connect( editor, SIGNAL( focusReceivedSignal() ),
             agenda, SLOT( finishTypeAhead() ) );
  }
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
  KOTodoEditor *todoEditor = new KOTodoEditor( mMainView->calendar(),
                                               mMainView );

  connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
           todoEditor, SLOT( updateCategoryConfig() ) );
  connect( todoEditor, SIGNAL( editCategories() ),
           mCategoryEditDialog, SLOT( show() ) );

  connect( todoEditor, SIGNAL( todoAdded( Todo * ) ),
           mMainView, SLOT( todoAdded( Todo * ) ) );
  connect( todoEditor, SIGNAL( todoChanged( Todo *, Todo * ) ),
           mMainView, SLOT( todoChanged( Todo *, Todo * ) ) );
  connect( todoEditor, SIGNAL( todoDeleted() ),
           mMainView, SLOT( updateTodoViews() ) );
  connect( todoEditor, SIGNAL( dialogClose( Incidence * ) ),
           mMainView, SLOT( dialogClosing( Incidence * ) ) );

  connect( mMainView, SIGNAL( closingDown() ), todoEditor, SLOT( reject() ) );

  return todoEditor;
}

void KODialogManager::updateSearchDialog()
{
  if (mSearchDialog) mSearchDialog->updateView();
}

void KODialogManager::setDocumentId( const QString &id )
{
  if (mOutgoingDialog) mOutgoingDialog->setDocumentId( id );
}
