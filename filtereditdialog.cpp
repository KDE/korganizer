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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlistbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

#include <libkcal/calfilter.h>
#include <libkdepim/categoryselectdialog.h>

#include "koprefs.h"
#include "filteredit_base.h"

#include "filtereditdialog.h"
#include "filtereditdialog.moc"

// FIXME: Make dialog work on a copy of the filters objects.

FilterEditDialog::FilterEditDialog( QPtrList<CalFilter> *filters,
                                    QWidget *parent, const char *name)
  : KDialogBase( parent, name, false, i18n("Edit Calendar Filters"),
                 Ok | Apply | Cancel ),
    mCategorySelectDialog( 0 )
{
  mFilters = filters;

  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  mSelectionCombo = new QComboBox( mainWidget );
  connect( mSelectionCombo, SIGNAL( activated( int ) ),
           SLOT( filterSelected() ) );

  QPushButton *addButton = new QPushButton( i18n("Add Filter..."), mainWidget );
  connect( addButton, SIGNAL( clicked() ), SLOT( slotAdd() ) );

  mRemoveButton = new QPushButton( i18n("Remove"), mainWidget );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( slotRemove() ) );

  mEditor = new FilterEdit_base( mainWidget );

  QGridLayout *topLayout = new QGridLayout( mainWidget, 2, 2 );
  topLayout->setSpacing( spacingHint() );
  topLayout->addWidget( mSelectionCombo, 0, 0 );
  topLayout->addWidget( addButton, 0, 1 );
  topLayout->addWidget( mRemoveButton, 0, 2 );
  topLayout->addMultiCellWidget( mEditor, 1, 1, 0, 2 );

  connect( mEditor->mCatEditButton, SIGNAL( clicked() ),
           SLOT( editCategorySelection() ) );

  // Clicking cancel exits the dialog without saving
  connect( this, SIGNAL( cancelClicked() ), SLOT( reject() ) );

  updateFilterList();
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::updateFilterList()
{
  mSelectionCombo->clear();

  CalFilter *filter = mFilters->first();

  if ( !filter ) {
    enableButtonOK( false );
    enableButtonApply( false );
  } else {
    while( filter ) {
      mSelectionCombo->insertItem( filter->name() );
      filter = mFilters->next();
    }

    CalFilter *f = mFilters->at( mSelectionCombo->currentItem() );
    if ( f ) readFilter( f );

    enableButtonOK( true );
    enableButtonApply( true );
  }

  mRemoveButton->setEnabled( mFilters->count() > 1 );
}

void FilterEditDialog::updateCategoryConfig()
{
  if ( mCategorySelectDialog ) mCategorySelectDialog->updateCategoryConfig();
}

void FilterEditDialog::slotDefault()
{
}

void FilterEditDialog::slotApply()
{
  CalFilter *f = mFilters->at( mSelectionCombo->currentItem() );
  writeFilter( f );
  emit filterChanged();
}

void FilterEditDialog::slotOk()
{
  CalFilter *f = mFilters->at( mSelectionCombo->currentItem() );
  writeFilter( f );
  emit filterChanged();
  accept();
}

void FilterEditDialog::slotAdd()
{
  QString txt = KInputDialog::getText( i18n("Add Filter"),
                                       i18n("Enter filter name:"),
                                       QString::null, 0, this );
  if ( !txt.isEmpty() ) {
    mFilters->append( new CalFilter( txt ) );
    updateFilterList();
  }
}

void FilterEditDialog::slotRemove()
{
  int currentItem = mSelectionCombo->currentItem();
  if ( currentItem < 0 ) return;

  // We need at least a default filter object.
  if ( mFilters->count() <= 1 ) return;

  int result = KMessageBox::warningContinueCancel( this,
     i18n("This item will be permanently deleted."), i18n("Delete Confirmation"), KGuiItem(i18n("Delete"),"editdelete") );

  if ( result != KMessageBox::Continue ) {
    return;
  }

  mFilters->remove( currentItem );
  updateFilterList();
  emit filterChanged();
}

void FilterEditDialog::editCategorySelection()
{
  if ( !mCategorySelectDialog ) {
    mCategorySelectDialog = new KPIM::CategorySelectDialog(
        KOPrefs::instance(), this, "filterCatSelect" );
    mCategorySelectDialog->setSelected( mCategories );
    connect( mCategorySelectDialog,
             SIGNAL( categoriesSelected( const QStringList & ) ),
             SLOT( updateCategorySelection( const QStringList & ) ) );
    connect( mCategorySelectDialog, SIGNAL( editCategories() ),
             SIGNAL( editCategories() ) );
  }

  mCategorySelectDialog->show();
}

void FilterEditDialog::updateCategorySelection( const QStringList &categories )
{
  mCategories = categories;

  mEditor->mCatList->clear();
  mEditor->mCatList->insertStringList( mCategories );
}

void FilterEditDialog::filterSelected()
{
  CalFilter *f = mFilters->at( mSelectionCombo->currentItem() );
  kdDebug(5850) << "Selected filter " << f->name() << endl;
  if ( f ) readFilter( f );
}

void FilterEditDialog::readFilter( CalFilter *filter )
{
  int c = filter->criteria();

  mEditor->mCompletedCheck->setChecked( c & CalFilter::HideCompleted );
  mEditor->mRecurringCheck->setChecked( c & CalFilter::HideRecurring );

  if ( c & CalFilter::ShowCategories ) {
    mEditor->mCatShowCheck->setChecked( true );
  } else {
    mEditor->mCatHideCheck->setChecked( true );
  }

  mEditor->mCatList->clear();
  mEditor->mCatList->insertStringList( filter->categoryList() );
  mCategories = filter->categoryList();
}

void FilterEditDialog::writeFilter( CalFilter *filter )
{
  int c = 0;

  if ( mEditor->mCompletedCheck->isChecked() ) c |= CalFilter::HideCompleted;
  if ( mEditor->mRecurringCheck->isChecked() ) c |= CalFilter::HideRecurring;
  if ( mEditor->mCatShowCheck->isChecked() ) c |= CalFilter::ShowCategories;
  if ( mEditor->mHideInactiveTodosCheck->isChecked() ) c |= CalFilter::HideInactiveTodos;

  filter->setCriteria( c );

  QStringList categoryList;
  for( uint i = 0; i < mEditor->mCatList->count(); ++i ) {
    categoryList.append( mEditor->mCatList->text( i ) );
  }
  filter->setCategoryList( categoryList );
}
