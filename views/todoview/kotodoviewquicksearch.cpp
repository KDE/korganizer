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

#include <libkdepim/categoryhierarchyreader.h>

#include <kcal/calendar.h>
#include <kcal/calfilter.h>

#include "kcheckcombobox.h"
#include <KLineEdit>

#include <QString>
#include <QStringList>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QHBoxLayout>

using namespace KCal;
using namespace KPIM;

KOTodoViewQuickSearch::KOTodoViewQuickSearch( Calendar *calendar, QWidget *parent )
  : QWidget( parent ), mCalendar( calendar )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  // no special margin because it is added by the view
  layout->setContentsMargins( 0, 0, 0, 0 );

  mSearchLine = new KLineEdit( this );
  mSearchLine->setClickMessage( i18nc( "@label in QuickSearchLine", "Search" ) );
  mSearchLine->setClearButtonShown( true );
  connect( mSearchLine, SIGNAL(textChanged(const QString &)),
           this, SIGNAL(searchTextChanged(const QString &)) );

  layout->addWidget( mSearchLine, 3 );

  mCategoryCombo = new KCheckComboBox( this );
  mCategoryCombo->setDefaultText( i18nc( "@item:inlistbox", "Select Categories" ) );
  mCategoryCombo->setSeparator( i18nc( "@item delimiter for joining category names", "," ) );

  connect( mCategoryCombo, SIGNAL(checkedItemsChanged(const QStringList &)),
           this, SIGNAL(searchCategoryChanged(const QStringList &)) );

  layout->addWidget( mCategoryCombo, 1 );
  fillCategories();

  setLayout( layout );
}

void KOTodoViewQuickSearch::setCalendar( Calendar *calendar )
{
  mCalendar = calendar;
  fillCategories();
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
  QStringList currentCategories = mCategoryCombo->checkedItems();
  mCategoryCombo->clear();

  QStringList categories;

  if ( mCalendar ) {
    CalFilter *filter = mCalendar->filter();
    if ( filter->criteria() & CalFilter::ShowCategories ) {
      categories = filter->categoryList();
      categories.sort();
    } else {
      categories = KOPrefs::instance()->mCustomCategories;
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

  CategoryHierarchyReaderQComboBox( mCategoryCombo ).read( categories );

  QStandardItemModel *model =
      qobject_cast<QStandardItemModel *>( mCategoryCombo->model() );
  Q_ASSERT( model );
  for ( int r = 0; r < model->rowCount(); ++r ) {
    QStandardItem *item = model->item( r );
    item->setCheckable( true );
  }
  mCategoryCombo->setCheckedItems( currentCategories );
}

#include "kotodoviewquicksearch.moc"
