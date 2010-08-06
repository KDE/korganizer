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
#include "categoryconfig.h"
#include "categoryeditdialog.h"
#include "koglobals.h"
#include "koprefs.h"
#include "filtereditdialog.h"
#include "searchdialog.h"

#include <Akonadi/Item>

#include <kcalcore/visitor.h>
#include <kcalcore/incidencebase.h>

#include <incidenceeditors/journaleditor.h>
#include <incidenceeditors/todoeditor.h>

#include <incidenceeditors/incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditors/incidenceeditor-ng/incidencedialogfactory.h>

#include <akonadi/kcal/utils.h>

#include <KCMultiDialog>
#include <KLocale>
#include <KMessageBox>

using namespace Akonadi;
using namespace KOrg;
using namespace KPIM;
using namespace KCalCore;
using namespace IncidenceEditors;

// FIXME: Handle KOEventViewerDialogs in dialog manager.

class KODialogManager::DialogManagerVisitor : public Visitor
{
  public:
    DialogManagerVisitor() : mDialogManager( 0 ) {}

    bool act( IncidenceBase::Ptr incidence, KODialogManager *manager )
    {
      mDialogManager = manager;
      return incidence->accept( *this, incidence );
    }

  protected:
    KODialogManager *mDialogManager;
};

class KODialogManager::EditorDialogVisitor :
      public KODialogManager::DialogManagerVisitor
{
  public:
    EditorDialogVisitor() : DialogManagerVisitor(), mEditor( 0 ) {}
    IncidenceEditor *editor() const { return mEditor; }

  protected:
    bool visit( Event::Ptr )
    {
      return false;
    }
    bool visit( Todo::Ptr )
    {
      mEditor = mDialogManager->getTodoEditor();
      return mEditor;
    }
    bool visit( Journal::Ptr )
    {
      mEditor = mDialogManager->getJournalEditor();
      return mEditor;
    }
    bool visit( FreeBusy::Ptr ) // to inhibit hidden virtual compile warning
    {
      return false;
    }

    IncidenceEditor *mEditor;
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
    mSearchDialog = new SearchDialog( mMainView );
    //mSearchDialog->setCalendar( mMainView->calendar() );
    connect( mSearchDialog, SIGNAL(showIncidenceSignal(Akonadi::Item)),
             mMainView, SLOT(showIncidence(Akonadi::Item)) );
    connect( mSearchDialog, SIGNAL(editIncidenceSignal(Akonadi::Item)),
             mMainView, SLOT(editIncidence(Akonadi::Item)) );
    connect( mSearchDialog, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
             mMainView, SLOT(deleteIncidence(Akonadi::Item)) );
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
    mArchiveDialog = new ArchiveDialog( mMainView->calendar(), mMainView->incidenceChanger() );
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

IncidenceEditorsNG::IncidenceDialog *KODialogManager::createDialog( const Akonadi::Item& item )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    return 0;
  }

  IncidenceEditorsNG::IncidenceDialog *dialog = IncidenceEditorsNG::IncidenceDialogFactory::create( incidence->type() );
//   connectEditor( dialog );
  return dialog;
}

// TODO: Get rid of this when there is an IncidenceDialog based JournalDialog.
IncidenceEditor *KODialogManager::getEditor( const Item &item )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );
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

void KODialogManager::connectTypeAhead( IncidenceEditorsNG::IncidenceDialog *dialog, KOEventView *view )
{
  if ( dialog && view ) {
    view->setTypeAheadReceiver( dialog->typeAheadReceiver() );
  }
}

void KODialogManager::connectEditor( IncidenceEditor *editor )
{
  createCategoryEditor();
  connect( editor, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(deleteIncidence(Akonadi::Item)) );

  connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
           editor, SIGNAL(updateCategoryConfig()) );
  connect( editor, SIGNAL(editCategories()),
           mCategoryEditDialog, SLOT(show()) );
  connect( editor, SIGNAL(dialogClose(Akonadi::Item)),
           mMainView, SLOT(dialogClosing(Akonadi::Item)) );
  connect( mMainView, SIGNAL(closingDown()), editor, SLOT(reject()) );
  connect( editor, SIGNAL(deleteAttendee(Akonadi::Item)),
           mMainView, SIGNAL(cancelAttendees(Akonadi::Item)) );
}

TodoEditor *KODialogManager::getTodoEditor()
{
  kDebug();
  TodoEditor *todoEditor = new TodoEditor( mMainView );
  connectEditor( todoEditor );
  return todoEditor;
}

JournalEditor *KODialogManager::getJournalEditor()
{
  JournalEditor *journalEditor = new JournalEditor( mMainView );
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

    IncidenceEditors::CategoryConfig* cc = new IncidenceEditors::CategoryConfig( KOPrefs::instance(), this );
    mCategoryEditDialog =
      new CategoryEditDialog( cc, mMainView );
    mCategoryEditDialog->setModal( true );
    mCategoryEditDialog->setHelp( "categories-view", "korganizer" );
    connect( mMainView, SIGNAL(categoriesChanged()),
             mCategoryEditDialog, SLOT(reload()) );
    connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
             mMainView, SIGNAL(categoryConfigChanged()) );
  }
}

#include "kodialogmanager.moc"
