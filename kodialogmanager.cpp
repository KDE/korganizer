/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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


// TODO: Handle KOEventViewerDialogs in dialog manager. Pass
// KOPrefs::mCompactDialog.

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

void KODialogManager::errorSaveEvent( QWidget *parent )
{
  KMessageBox::sorry( parent, i18n("Unable to save event.") );
}

void KODialogManager::errorSaveTodo( QWidget *parent )
{
  KMessageBox::sorry( parent, i18n("Unable to save todo item.") );
}

void KODialogManager::errorSaveJournal( QWidget *parent )
{
  KMessageBox::sorry( parent, i18n("Unable to save journal entry.") );
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
    mOptionsDialog = new KCMultiDialog( mMainView, "KorganizerPreferences" );
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

    modules.append( "korganizer_configmain.desktop" );
    modules.append( "korganizer_configtime.desktop" );
    modules.append( "korganizer_configviews.desktop" );
    modules.append( "korganizer_configfonts.desktop" );
    modules.append( "korganizer_configcolors.desktop" );
    modules.append( "korganizer_configprinting.desktop" );
    modules.append( "korganizer_configgroupscheduling.desktop" );
    modules.append( "korganizer_configgroupautomation.desktop" );
    modules.append( "korganizer_configfreebusy.desktop" );

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
  createIncomingDialog();
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

void KODialogManager::showFilterEditDialog( QPtrList<CalFilter> *filters )
{
  if ( !mFilterEditDialog ) {
    mFilterEditDialog = new FilterEditDialog( filters, mMainView );
    connect( mFilterEditDialog, SIGNAL( filterChanged() ),
             mMainView, SLOT( filterEdited() ) );
    connect( mFilterEditDialog, SIGNAL( editCategories() ),
             mCategoryEditDialog, SLOT( show() ) );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             mFilterEditDialog, SLOT( updateCategoryConfig() ) );
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
  KOEventEditor *eventEditor = new KOEventEditor( mMainView->calendar(), mMainView );
  connectEditor( eventEditor );
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

void KODialogManager::connectEditor( KOIncidenceEditor*editor )
{
  connect( editor, SIGNAL( incidenceAdded( Incidence * ) ),
           mMainView, SLOT( incidenceAdded( Incidence * ) ) );
  connect( editor, SIGNAL( incidenceChanged( Incidence *, Incidence * ) ),
           mMainView, SLOT( incidenceChanged( Incidence *, Incidence * ) ) );
  connect( editor, SIGNAL( incidenceToBeDeleted( Incidence * ) ),
           mMainView, SLOT( incidenceToBeDeleted( Incidence * ) ) );
  connect( editor, SIGNAL( incidenceDeleted( Incidence * ) ),
           mMainView, SLOT( incidenceDeleted( Incidence * ) ) );

  connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
           editor, SLOT( updateCategoryConfig() ) );
  connect( editor, SIGNAL( editCategories() ),
           mCategoryEditDialog, SLOT( show() ) );

  connect( editor, SIGNAL( dialogClose( Incidence * ) ),
           mMainView, SLOT( dialogClosing( Incidence * ) ) );
  connect( editor, SIGNAL( editCanceled( Incidence * ) ),
           mMainView, SLOT( editCanceled( Incidence * ) ) );
  connect( mMainView, SIGNAL( closingDown() ), editor, SLOT( reject() ) );

  // TODO_RK: Check this. I'm afraid the deleteAttendee is emitted on wrong conditions.
  // Also, we don't have the addresses of the deleted attendees available here!
  // We should probably do this on the incidenceChanged signal.
  connect( editor, SIGNAL( deleteAttendee( Incidence * ) ),
           mMainView, SLOT( schedule_cancel( Incidence * ) ) );
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
  KOTodoEditor *todoEditor = new KOTodoEditor( mMainView->calendar(), mMainView );
  connectEditor( todoEditor );
  connect( todoEditor, SIGNAL( todoCompleted( Todo * ) ),
           mMainView, SLOT( recurTodo( Todo *) ) ) ;
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
