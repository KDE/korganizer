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
#include "calendarview.h"
#include "filtereditdialog.h"
#include "koprefs.h"
#include "searchdialog.h"

#include <calendarsupport/archivedialog.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/utils.h>

#include <incidenceeditor-ng/categoryeditdialog.h>
#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidencedialogfactory.h>

#include <Akonadi/Item>
#include <Akonadi/TagManagementDialog>

#include <KCalCore/Visitor>

#include <KCMultiDialog>

using namespace KOrg;

// FIXME: Handle KOEventViewerDialogs in dialog manager.

class KODialogManager::DialogManagerVisitor : public KCalCore::Visitor
{
  public:
    DialogManagerVisitor() : mDialogManager( 0 ) {}

    bool act( KCalCore::IncidenceBase::Ptr incidence, KODialogManager *manager )
    {
      mDialogManager = manager;
      return incidence->accept( *this, incidence );
    }

  protected:
    KODialogManager *mDialogManager;
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
    connect( mOptionsDialog, SIGNAL(configCommitted(QByteArray)),
             mMainView, SLOT(updateConfig(QByteArray)) );
    QStringList modules;

    modules.append( QLatin1String("korganizer_configmain.desktop") );
    modules.append( QLatin1String("korganizer_configtime.desktop") );
    modules.append( QLatin1String("korganizer_configviews.desktop") );
    modules.append( QLatin1String("korganizer_configcolorsandfonts.desktop") );
    modules.append( QLatin1String("korganizer_configgroupscheduling.desktop") );
    modules.append( QLatin1String("korganizer_configfreebusy.desktop") );
    modules.append( QLatin1String("korganizer_configplugins.desktop") );
    modules.append( QLatin1String("korganizer_configdesignerfields.desktop") );

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
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void KODialogManager::showArchiveDialog()
{
  if ( !mArchiveDialog ) {
    mArchiveDialog =
      new CalendarSupport::ArchiveDialog( mMainView->calendar(), mMainView->incidenceChanger() );
    connect( mArchiveDialog, SIGNAL(eventsDeleted()),
             mMainView, SLOT(updateView()) );
    connect( mArchiveDialog, SIGNAL(autoArchivingSettingsModified()),
             mMainView, SLOT(slotAutoArchivingSettingsModified()) );
  }
  mArchiveDialog->show();
  mArchiveDialog->raise();

  // Workaround.
  QApplication::restoreOverrideCursor();
}

void KODialogManager::showFilterEditDialog( QList<KCalCore::CalFilter*> *filters )
{
  createCategoryEditor();
  if ( !mFilterEditDialog ) {
    mFilterEditDialog = new FilterEditDialog( filters, mMainView );
    connect( mFilterEditDialog, SIGNAL(filterChanged()),
             mMainView, SLOT(updateFilter()) );
    connect( mFilterEditDialog, SIGNAL(editCategories()),
             mCategoryEditDialog, SLOT(show()) );
#if 0
    connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
             mFilterEditDialog, SLOT(updateCategoryConfig()) );
#endif
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

IncidenceEditorNG::IncidenceDialog *KODialogManager::createDialog( const Akonadi::Item &item )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    return 0;
  }

  IncidenceEditorNG::IncidenceDialog *dialog =
    IncidenceEditorNG::IncidenceDialogFactory::create(
      /*needs initial saving=*/false,
      incidence->type(), mMainView->incidenceChanger(), mMainView );

  return dialog;
}

void KODialogManager::connectTypeAhead( IncidenceEditorNG::IncidenceDialog *dialog,
                                        KOEventView *view )
{
  if ( dialog && view ) {
    view->setTypeAheadReceiver( dialog->typeAheadReceiver() );
  }
}

void KODialogManager::connectEditor( IncidenceEditorNG::IncidenceDialog *editor )
{
  createCategoryEditor();
  connect( editor, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(deleteIncidence(Akonadi::Item)) );

  connect( editor, SIGNAL(dialogClose(Akonadi::Item)),
           mMainView, SLOT(dialogClosing(Akonadi::Item)) );
  connect( editor, SIGNAL(deleteAttendee(Akonadi::Item)),
           mMainView, SIGNAL(cancelAttendees(Akonadi::Item)) );
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
    mCategoryEditDialog = new Akonadi::TagManagementDialog(mMainView);

    mCategoryEditDialog->setModal( true );
    mCategoryEditDialog->setHelp( QLatin1String("categories-view"), QLatin1String("korganizer") );
  }
}

