/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2005 Thomas Zander <zander@kde.org>

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

#include "filtereditdialog.h"
#include "koprefs.h"

#include <calendarsupport/categoryconfig.h>

#include <incidenceeditor-ng/categoryselectdialog.h>

#include <KCalCore/CalFilter>

#include <KMessageBox>

FilterEditDialog::FilterEditDialog( QList<KCalCore::CalFilter*> *filters, QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18nc( "@title::window", "Edit Calendar Filters" ) );
  setButtons( Ok | Apply | Cancel );
  setMainWidget( mFilterEdit = new FilterEdit( filters, this ) );

  connect( mFilterEdit, SIGNAL(dataConsistent(bool)), SLOT(setDialogConsistent(bool)) );
  updateFilterList();
  connect( mFilterEdit, SIGNAL(editCategories()), SIGNAL(editCategories()) );
  connect( mFilterEdit, SIGNAL(filterChanged()), SIGNAL(filterChanged()) );
  connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()) );
  connect( this, SIGNAL(applyClicked()), this, SLOT(slotApply()) );
}

FilterEditDialog::~FilterEditDialog()
{
  delete mFilterEdit;
  mFilterEdit = 0;
}

void FilterEditDialog::updateFilterList()
{
  mFilterEdit->updateFilterList();
}

void FilterEditDialog::updateCategoryConfig()
{
  mFilterEdit->updateCategoryConfig();
}

void FilterEditDialog::slotApply()
{
  mFilterEdit->saveChanges();
}

void FilterEditDialog::slotOk()
{
  slotApply();
  accept();
}

void FilterEditDialog::setDialogConsistent( bool consistent )
{
    enableButton( Ok, consistent );
    enableButtonApply( consistent );
}

FilterEdit::FilterEdit( QList<KCalCore::CalFilter*> *filters, QWidget *parent )
  : QWidget( parent ), mCurrent( 0 ), mCategorySelectDialog( 0 )
{
  setupUi( this );
  searchline->setListWidget( mRulesList );
  mDetailsFrame->setEnabled( false );
  mFilters = filters;
  mNewButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Press this button to define a new filter." ) );
  mDeleteButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Press this button to remove the currently active filter." ) );

  connect( mRulesList, SIGNAL(itemSelectionChanged()),
           this, SLOT(filterSelected()) );
  connect( mNewButton, SIGNAL(clicked()),
           SLOT(bNewPressed()) );
  connect( mDeleteButton, SIGNAL(clicked()),
           SLOT(bDeletePressed()) );
  connect( mNameLineEdit, SIGNAL(textChanged(QString)),
           SLOT(updateSelectedName(QString)) );
  connect( mCatEditButton, SIGNAL(clicked()), SLOT(editCategorySelection()) );
  connect( mCompletedCheck, SIGNAL(toggled(bool)),
           mCompletedTimeSpanLabel, SLOT(setEnabled(bool)) );
  connect( mCompletedCheck, SIGNAL(toggled(bool)),
           mCompletedTimeSpan, SLOT(setEnabled(bool)) );

}

FilterEdit::~FilterEdit()
{
}

void FilterEdit::updateFilterList()
{
  mRulesList->clear();
  qDebug() << "  FilterEdit::updateFilterList() :" << mFilters;
  if ( mFilters ) {
    qDebug() << " mFilters->empty() :" << mFilters->empty();
  }
  if ( !mFilters || mFilters->empty() ) {
    mDetailsFrame->setEnabled( false );
    emit( dataConsistent( false ) );
  } else {
    QList<KCalCore::CalFilter*>::iterator i;
    for ( i = mFilters->begin(); i != mFilters->end(); ++i ) {
      if ( *i ) {
        mRulesList->addItem( (*i)->name() );
      }
    }
    if ( mRulesList->currentRow() != -1 ) {
      KCalCore::CalFilter *f = mFilters->at( mRulesList->currentRow() );
      if ( f ) {
        filterSelected( f );
      }
    }
    emit( dataConsistent( true ) );
  }
  if ( mFilters && mFilters->count() > 0 && !mCurrent ) {
    filterSelected( mFilters->at( 0 ) );
  }
  if ( mFilters ) {
    mDeleteButton->setEnabled( !mFilters->isEmpty() );
  }
}

void FilterEdit::saveChanges()
{
  if ( !mCurrent ) {
    return;
  }

  mCurrent->setName( mNameLineEdit->text() );
  int criteria = 0;
  if ( mCompletedCheck->isChecked() ) {
    criteria |= KCalCore::CalFilter::HideCompletedTodos;
  }
  if ( mRecurringCheck->isChecked() ) {
    criteria |= KCalCore::CalFilter::HideRecurring;
  }
  if ( mCatShowCheck->isChecked() ) {
    criteria |= KCalCore::CalFilter::ShowCategories;
  }
  if ( mHideInactiveTodosCheck->isChecked() ) {
    criteria |= KCalCore::CalFilter::HideInactiveTodos;
  }
  if ( mHideTodosNotAssignedToMeCheck->isChecked() ) {
    criteria |= KCalCore::CalFilter::HideNoMatchingAttendeeTodos;
  }
  mCurrent->setCriteria( criteria );
  mCurrent->setCompletedTimeSpan( mCompletedTimeSpan->value() );

  QStringList categoryList;
  for ( int i = 0; i < mCatList->count(); ++i ) {
    QListWidgetItem *item = mCatList->item(i);
    if ( item ) {
      categoryList.append( item->text() );
    }
  }
  mCurrent->setCategoryList( categoryList );
  emit filterChanged();
}

void FilterEdit::filterSelected()
{
  if ( mRulesList->currentRow() < mFilters->count() ) {
    mDetailsFrame->setEnabled( true );
    filterSelected( mFilters->at( mRulesList->currentRow() ) );
  }
}

void FilterEdit::filterSelected( KCalCore::CalFilter *filter )
{
  if( !filter || filter == mCurrent ) {
    return;
  }
  kDebug() << "Selected filter" << filter->name();
  saveChanges();

  mCurrent = filter;
  mNameLineEdit->blockSignals( true );
  mNameLineEdit->setText( mCurrent->name() );
  mNameLineEdit->blockSignals( false );
  mDetailsFrame->setEnabled( true );
  mCompletedCheck->setChecked( mCurrent->criteria() & KCalCore::CalFilter::HideCompletedTodos );
  mCompletedTimeSpan->setValue( mCurrent->completedTimeSpan() );
  mRecurringCheck->setChecked( mCurrent->criteria() & KCalCore::CalFilter::HideRecurring );
  mHideInactiveTodosCheck->setChecked(
    mCurrent->criteria() & KCalCore::CalFilter::HideInactiveTodos );
  mHideTodosNotAssignedToMeCheck->setChecked(
    mCurrent->criteria() & KCalCore::CalFilter::HideNoMatchingAttendeeTodos );

  if ( mCurrent->criteria() & KCalCore::CalFilter::ShowCategories ) {
    mCatShowCheck->setChecked( true );
  } else {
    mCatHideCheck->setChecked( true );
  }
  mCatList->clear();
  mCatList->addItems( mCurrent->categoryList() );
}

void FilterEdit::bNewPressed()
{
  mDetailsFrame->setEnabled( true );
  saveChanges();
  KCalCore::CalFilter *newFilter =
    new KCalCore::CalFilter( i18nc( "@label default filter name",
                                    "New Filter %1", mFilters->count() ) );
  mFilters->append( newFilter );
  updateFilterList();
  mRulesList->setCurrentRow( mRulesList->count() - 1 );
  emit filterChanged();
}

void FilterEdit::bDeletePressed()
{
  if ( !mRulesList->currentItem() ) { // nothing selected
    return;
  }
  if ( mFilters->isEmpty() ) { // We need at least a default filter object.
    return;
  }

  if ( KMessageBox::warningContinueCancel(
         this,
         i18nc( "@info",
                "Do you really want to permanently remove the filter \"%1\"?", mCurrent->name() ),
         i18nc( "@title:window", "Delete Filter?" ),
         KStandardGuiItem::del() ) == KMessageBox::Cancel ) {
    return;
  }

  int selected = mRulesList->currentRow();
  KCalCore::CalFilter *filter = mFilters->at( selected );
  mFilters->removeAll( filter );
  delete filter;
  mCurrent = 0;
  updateFilterList();
  mRulesList->setCurrentRow( qMin( mRulesList->count() - 1, selected ) );
  emit filterChanged();
}

void FilterEdit::updateSelectedName( const QString &newText )
{
  mRulesList->blockSignals( true );
  QListWidgetItem *item = mRulesList->currentItem();
  if ( item ) {
    item->setText( newText );
  }
  mRulesList->blockSignals( false );
  bool allOk = true;

  foreach ( KCalCore::CalFilter *i, *mFilters ) {
    if ( i && i->name().isEmpty() ) {
      allOk = false;
    }
  }

  emit dataConsistent( allOk );
}

void FilterEdit::editCategorySelection()
{
  if( !mCurrent ) {
    return;
  }

  if ( !mCategorySelectDialog ) {
    CalendarSupport::CategoryConfig *cc =
      new CalendarSupport::CategoryConfig( KOPrefs::instance(), this );
    mCategorySelectDialog = new IncidenceEditorNG::CategorySelectDialog( cc, this );
    mCategorySelectDialog->setHelp( QLatin1String("categories-view"), QLatin1String("korganizer") );
    mCategorySelectDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help );
    connect( mCategorySelectDialog, SIGNAL(categoriesSelected(QStringList)),
             SLOT(updateCategorySelection(QStringList)) );
    connect( mCategorySelectDialog, SIGNAL(editCategories()),
             SIGNAL(editCategories()) );

  }
  // we need the children not to autoselect or else some unselected
  // children can also become selected
  mCategorySelectDialog->setAutoselectChildren( false );
  mCategorySelectDialog->setSelected( mCurrent->categoryList() );
  mCategorySelectDialog->setAutoselectChildren( true );

  mCategorySelectDialog->show();
}

void FilterEdit::updateCategorySelection( const QStringList &categories )
{
  mCatList->clear();
  mCatList->addItems( categories );
  mCurrent->setCategoryList( categories );
}

void FilterEdit::updateCategoryConfig()
{
  if ( mCategorySelectDialog ) {
    mCategorySelectDialog->updateCategoryConfig();
  }
}

