/*
  This file is part of KOrganizer.

  Copyright (c) 2004 Till Adam <adam@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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
  51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotodoviewquicksearch.h"
#include "koprefs.h"

#include "incidenceeditor-ng/categoryhierarchyreader.h"

#include "libkdepim/kcheckcombobox.h"
using namespace KPIM;

#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/calendar.h>
using namespace CalendarSupport;

#include <KCalCore/CalFilter>
using namespace KCalCore;

#include <KLineEdit>

#include <QString>
#include <QStringList>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QHBoxLayout>

KOTodoViewQuickSearch::KOTodoViewQuickSearch( CalendarSupport::Calendar *calendar, QWidget *parent )
  : QWidget( parent ), mCalendar( calendar )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  // no special margin because it is added by the view
  layout->setContentsMargins( 0, 0, 0, 0 );

  mSearchLine = new KLineEdit( this );
  mSearchLine->setClickMessage( i18nc( "@label in QuickSearchLine", "Search" ) );
  mSearchLine->setClearButtonShown( true );
  connect( mSearchLine, SIGNAL(textChanged(QString)),
           this, SIGNAL(searchTextChanged(QString)) );

  layout->addWidget( mSearchLine, 3 );

  mCategoryCombo = new KCheckComboBox( this );
  mCategoryCombo->setDefaultText( i18nc( "@item:inlistbox", "Select Categories" ) );
  mCategoryCombo->setSeparator( i18nc( "@item:intext delimiter for joining category names", "," ) );

  connect( mCategoryCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(emitSearchCategoryChanged()) );

  layout->addWidget( mCategoryCombo, 1 );
  fillCategories();

  { // Make the combo big enough so that "Select Categories" fits.
    QFontMetrics fm = mCategoryCombo->lineEdit()->fontMetrics();

    // QLineEdit::sizeHint() returns a nice size to fit 17 'x' chars.
    const int currentPreferedWidth = mCategoryCombo->lineEdit()->sizeHint().width();

    // Calculate a nice size for "Select Categories"
    const int newPreferedWidth = currentPreferedWidth - fm.width( QLatin1Char('x') )*17 + fm.width( mCategoryCombo->defaultText() );

    const int pixelsToAdd = newPreferedWidth - mCategoryCombo->lineEdit()->width();
    mCategoryCombo->setMinimumWidth( mCategoryCombo->width() + pixelsToAdd );
  }

  setLayout( layout );
}

void KOTodoViewQuickSearch::setCalendar( CalendarSupport::Calendar *calendar )
{
  if ( calendar != mCalendar ) {
    mCalendar = calendar;
    fillCategories();
  }
}

void KOTodoViewQuickSearch::updateCategories()
{
  fillCategories();
}

void KOTodoViewQuickSearch::reset()
{
  mSearchLine->clear();
  mCategoryCombo->setCurrentIndex( 0 );
}

void KOTodoViewQuickSearch::fillCategories()
{
  QStringList currentCategories = mCategoryCombo->checkedItems( Qt::UserRole );
  mCategoryCombo->clear();

  QStringList categories;

  if ( mCalendar ) {
    CalFilter *filter = mCalendar->filter();
    if ( filter->criteria() & CalFilter::ShowCategories ) {
      categories = filter->categoryList();
      categories.sort();
    } else {
      CategoryConfig cc( KOPrefs::instance() );
      categories = cc.customCategories();
      QStringList filterCategories = filter->categoryList();
      categories.sort();
      filterCategories.sort();

      QStringList::Iterator it = categories.begin();
      QStringList::Iterator jt = filterCategories.begin();
      while ( it != categories.end() && jt != filterCategories.end() ) {
        if ( *it == *jt ) {
          it = categories.erase( it );
          jt++;
        } else if ( *it < *jt ) {
          it++;
        } else if ( *it > *jt ) {
          jt++;
        }
      }
    }
  }

  IncidenceEditorNG::CategoryHierarchyReaderQComboBox( mCategoryCombo ).read( categories );
  mCategoryCombo->setCheckedItems( currentCategories, Qt::UserRole );
}

void KOTodoViewQuickSearch::emitSearchCategoryChanged()
{
  /* The display role doesn't work because it represents subcategories as " subcategory", and we want
   * "ParentCollection:subCategory" */
  emit searchCategoryChanged( mCategoryCombo->checkedItems( Qt::UserRole ) );
}

#include "kotodoviewquicksearch.moc"
