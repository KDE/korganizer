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
#include "koprefsdialog.h"
#include "koprefs.h"
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "kojournaleditor.h"
#include "searchdialog.h"
#include "filtereditdialog.h"
#ifndef KORG_NOARCHIVE
#include "archivedialog.h"
#endif
#include "koviewmanager.h"
#include "koagendaview.h"
#include "koglobals.h"

#include "kodialogmanager.h"
#include "kodialogmanager.moc"


// FIXME: Handle KOEventViewerDialogs in dialog manager. Pass
// KOPrefs::mCompactDialog.

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
    bool visit( Event * ) { mEditor = mDialogManager->getEventEditor(); return mEditor; }
    bool visit( Todo * ) { mEditor = mDialogManager->getTodoEditor(); return mEditor; }
    bool visit( Journal * ) { mEditor = mDialogManager->getJournalEditor(); return mEditor; }
  protected:
    KOIncidenceEditor *mEditor;
};


KODialogManager::KODialogManager( CalendarView *mainView ) :
  QObject(), mMainView( mainView )
{
  mOptionsDialog = 0;
  mSearchDialog = 0;
  mArchiveDialog = 0;
  mFilterEditDialog = 0;

  mCategoryEditDialog = new KPIM::CategoryEditDialog( KOPrefs::instance(), mMainView );
  connect( mainView, SIGNAL( categoriesChanged() ),
           mCategoryEditDialog, SLOT( reload() ) );
  KOGlobals::fitDialogToScreen( mCategoryEditDialog );
}

KODialogManager::~KODialogManager()
{
  delete mOptionsDialog;
  delete mSearchDialog;
#ifndef KORG_NOARCHIVE
  delete mArchiveDialog;
#endif
  delete mFilterEditDialog;
}

void KODialogManager::errorSaveIncidence( QWidget *parent, Incidence *incidence )
{
  KMessageBox::sorry( parent, i18n("Unable to save %1 \"%2\".")
                      .arg( i18n( incidence->type() ) )
                      .arg( incidence->summary() ) );
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
    // @TODO Find a way to do this with KCMultiDialog
    connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
            mOptionsDialog,SLOT(updateCategories()));
#endif

    QStringList modules;

    modules.append( "korganizer_configmain.desktop" );
    modules.append( "korganizer_configtime.desktop" );
    modules.append( "korganizer_configviews.desktop" );
    modules.append( "korganizer_configfonts.desktop" );
    modules.append( "korganizer_configcolors.desktop" );
    modules.append( "korganizer_configgroupscheduling.desktop" );
    modules.append( "korganizer_configgroupautomation.desktop" );
    modules.append( "korganizer_configfreebusy.desktop" );
    modules.append( "korganizer_configplugins.desktop" );
    modules.append( "korganizer_configdesignerfields.desktop" );

    // add them all
    QStringList::iterator mit;
    for ( mit = modules.begin(); mit != modules.end(); ++mit )
      mOptionsDialog->addModule( *mit );
#endif
  }

  mOptionsDialog->show();
  mOptionsDialog->raise();
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
    connect(mArchiveDialog,SIGNAL(autoArchivingSettingsModified()),
            mMainView,SLOT(slotAutoArchivingSettingsModified()));
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
             mMainView, SLOT( updateFilter() ) );
    connect( mFilterEditDialog, SIGNAL( editCategories() ),
             mCategoryEditDialog, SLOT( show() ) );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             mFilterEditDialog, SLOT( updateCategoryConfig() ) );
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

KOIncidenceEditor *KODialogManager::getEditor( Incidence *incidence )
{
  if ( !incidence ) return 0;
  EditorDialogVisitor v;
  if ( v.act( incidence, this ) ) {
    return v.editor();
  } else
    return 0;
}

KOEventEditor *KODialogManager::getEventEditor()
{
  KOEventEditor *eventEditor = new KOEventEditor( mMainView->calendar(),
                                                  mMainView );
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
/*  connect( editor, SIGNAL( deleteIncidenceSignal( Incidence * ) ),
           mMainView, SLOT( deleteIncidence( Incidence * ) ) );*/

  connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
           editor, SLOT( updateCategoryConfig() ) );
  connect( editor, SIGNAL( editCategories() ),
           mCategoryEditDialog, SLOT( show() ) );

  connect( editor, SIGNAL( dialogClose( Incidence * ) ),
           mMainView, SLOT( dialogClosing( Incidence * ) ) );
  connect( editor, SIGNAL( editCanceled( Incidence * ) ),
           mMainView, SLOT( editCanceled( Incidence * ) ) );
  connect( mMainView, SIGNAL( closingDown() ), editor, SLOT( reject() ) );

  connect( editor, SIGNAL( deleteAttendee( Incidence * ) ),
           mMainView, SLOT( deleteAttendee( Incidence * ) ) );
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
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
  if (mSearchDialog) mSearchDialog->updateView();
}

