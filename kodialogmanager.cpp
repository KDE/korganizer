/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "kodialogmanager.h"
#ifndef KORG_NOARCHIVE
#include "archivedialog.h"
#endif
#include "calendarview.h"
#include "koeventeditor.h"
#include "koglobals.h"
#include "kojournaleditor.h"
#include "koprefs.h"
#include "kotodoeditor.h"
#include "filtereditdialog.h"
#include "searchdialog.h"
#include "views/agendaview/koagendaview.h"

#include <libkdepim/categoryeditdialog.h>

#include <KCal/IncidenceBase>

#include <KCMultiDialog>
#include <KLocale>
#include <KMessageBox>

// FIXME: Handle KOEventViewerDialogs in dialog manager.
// Pass KOPrefs::mCompactDialog.

class KODialogManager::DialogManagerVisitor : public IncidenceBase::Visitor
{
  public:
    DialogManagerVisitor() : mDialogManager( 0 ) {}

    bool act( IncidenceBase *incidence, KODialogManager *manager )
    {
      mDialogManager = manager;
      return incidence->accept( *this );
    }

  protected:
    KODialogManager *mDialogManager;
};

class KODialogManager::EditorDialogVisitor :
      public KODialogManager::DialogManagerVisitor
{
  public:
    EditorDialogVisitor() : DialogManagerVisitor(), mEditor( 0 ) {}
    KOIncidenceEditor *editor() const { return mEditor; }

  protected:
    bool visit( Event * )
    {
      mEditor = mDialogManager->getEventEditor();
      return mEditor;
    }
    bool visit( Todo * )
    {
      mEditor = mDialogManager->getTodoEditor();
      return mEditor;
    }
    bool visit( Journal * )
    {
      mEditor = mDialogManager->getJournalEditor();
      return mEditor;
    }
    bool visit( FreeBusy * ) // to inhibit hidden virtual compile warning
    {
      return 0;
    }

    KOIncidenceEditor *mEditor;
};

KODialogManager::KODialogManager( CalendarView *mainView )
  : QObject(), mMainView( mainView )
{
  mOptionsDialog = 0;
  mSearchDialog = 0;
  mArchiveDialog = 0;
  mFilterEditDialog = 0;
  mCategoryEditDialog = 0;
}

KODialogManager::~KODialogManager()
{
  delete mOptionsDialog;
  delete mSearchDialog;
  delete mArchiveDialog;
  delete mFilterEditDialog;
  delete mCategoryEditDialog;
}

void KODialogManager::errorSaveIncidence( QWidget *parent, Incidence *incidence )
{
  KMessageBox::sorry(
    parent,
    i18n( "Unable to save %1 \"%2\".",
          i18n( incidence->type() ), incidence->summary() ) );
}

void KODialogManager::showOptionsDialog()
{
  if ( !mOptionsDialog ) {
    mOptionsDialog = new KCMultiDialog( mMainView );
    connect( mOptionsDialog, SIGNAL(configCommitted(const QByteArray&)),
             mMainView, SLOT(updateConfig(const QByteArray&)) );
    QStringList modules;

    modules.append( "korganizer_configmain.desktop" );
    modules.append( "korganizer_configtime.desktop" );
    modules.append( "korganizer_configviews.desktop" );
    modules.append( "korganizer_configcolorsandfonts.desktop" );
    modules.append( "korganizer_configgroupscheduling.desktop" );
    modules.append( "korganizer_configfreebusy.desktop" );
    modules.append( "korganizer_configplugins.desktop" );
    modules.append( "korganizer_configdesignerfields.desktop" );

    // add them all
    QStringList::iterator mit;
    for ( mit = modules.begin(); mit != modules.end(); ++mit ) {
      mOptionsDialog->addModule( *mit );
    }
  }

  mOptionsDialog->show();
  mOptionsDialog->raise();
}

void KODialogManager::showCategoryEditDialog()
{
  createCategoryEditor();
  mCategoryEditDialog->exec();
}

void KODialogManager::showSearchDialog()
{
  if ( !mSearchDialog ) {
    mSearchDialog = new SearchDialog( mMainView->calendar(), mMainView );
    connect( mSearchDialog, SIGNAL(showIncidenceSignal(Incidence *)),
             mMainView, SLOT(showIncidence(Incidence *)) );
    connect( mSearchDialog, SIGNAL(editIncidenceSignal(Incidence *)),
             mMainView, SLOT(editIncidence(Incidence *)) );
    connect( mSearchDialog, SIGNAL(deleteIncidenceSignal(Incidence *)),
             mMainView, SLOT(deleteIncidence(Incidence *)) );
    connect( mMainView, SIGNAL(closingDown()),mSearchDialog,SLOT(reject()) );
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void KODialogManager::showArchiveDialog()
{
#ifndef KORG_NOARCHIVE
  if ( !mArchiveDialog ) {
    mArchiveDialog = new ArchiveDialog( mMainView->calendar(), mMainView );
    connect( mArchiveDialog, SIGNAL(eventsDeleted()),
             mMainView, SLOT(updateView()) );
    connect( mArchiveDialog, SIGNAL(autoArchivingSettingsModified()),
             mMainView, SLOT(slotAutoArchivingSettingsModified()) );
  }
  mArchiveDialog->show();
  mArchiveDialog->raise();

  // Workaround.
  QApplication::restoreOverrideCursor();
#endif
}

void KODialogManager::showFilterEditDialog( QList<CalFilter*> *filters )
{
  createCategoryEditor();
  if ( !mFilterEditDialog ) {
    mFilterEditDialog = new FilterEditDialog( filters, mMainView );
    connect( mFilterEditDialog, SIGNAL(filterChanged()),
             mMainView, SLOT(updateFilter()) );
    connect( mFilterEditDialog, SIGNAL(editCategories()),
             mCategoryEditDialog, SLOT(show()) );
    connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
             mFilterEditDialog, SLOT(updateCategoryConfig()) );
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

KOIncidenceEditor *KODialogManager::getEditor( Incidence *incidence )
{
  if ( !incidence ) {
    return 0;
  }

  EditorDialogVisitor v;
  if ( v.act( incidence, this ) ) {
    return v.editor();
  } else {
    return 0;
  }
}

KOEventEditor *KODialogManager::getEventEditor()
{
  KOEventEditor *eventEditor = new KOEventEditor( mMainView->calendar(), mMainView );
  connectEditor( eventEditor );
  return eventEditor;
}

void KODialogManager::connectTypeAhead( KOEventEditor *editor, KOEventView *view )
{
  if ( editor && view ) {
    view->setTypeAheadReceiver( editor->typeAheadReceiver() );
    connect( editor, SIGNAL(focusReceivedSignal()),
             view, SLOT(finishTypeAhead()) );
  }
}

void KODialogManager::connectEditor( KOIncidenceEditor *editor )
{
  createCategoryEditor();
  connect( editor, SIGNAL(deleteIncidenceSignal(Incidence *)),
           mMainView, SLOT(deleteIncidence(Incidence *)) );

  connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
           editor, SIGNAL(updateCategoryConfig()) );
  connect( editor, SIGNAL(editCategories()),
           mCategoryEditDialog, SLOT(show()) );

  connect( editor, SIGNAL(dialogClose(Incidence *)),
           mMainView, SLOT(dialogClosing(Incidence *)) );
  connect( editor, SIGNAL(editCanceled(Incidence *)),
           mMainView, SLOT(editCanceled(Incidence *)) );
  connect( mMainView, SIGNAL(closingDown()), editor, SLOT(reject()) );

  connect( editor, SIGNAL(deleteAttendee(Incidence *)),
           mMainView, SIGNAL(cancelAttendees(Incidence *)) );
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
  kDebug();
  KOTodoEditor *todoEditor = new KOTodoEditor( mMainView->calendar(), mMainView );
  connectEditor( todoEditor );
  return todoEditor;
}

KOJournalEditor *KODialogManager::getJournalEditor()
{
  KOJournalEditor *journalEditor = new KOJournalEditor( mMainView->calendar(), mMainView );
  connectEditor( journalEditor );
  return journalEditor;
}

void KODialogManager::updateSearchDialog()
{
  if ( mSearchDialog ) {
    mSearchDialog->updateView();
  }
}

void KODialogManager::createCategoryEditor()
{
  if ( mCategoryEditDialog == 0 ) {
    mCategoryEditDialog =
      new KPIM::CategoryEditDialog( KOPrefs::instance(), mMainView );
    mCategoryEditDialog->setModal( true );
    mCategoryEditDialog->setHelp( "categories-view", "korganizer" );
    connect( mMainView, SIGNAL(categoriesChanged()),
             mCategoryEditDialog, SLOT(reload()) );
    connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
             mMainView, SIGNAL(categoryConfigChanged()) );
    KOGlobals::fitDialogToScreen( mCategoryEditDialog );
  }
}

#include "kodialogmanager.moc"
