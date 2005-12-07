/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#include "kotodoviewquicksearch.h"

#include "kotodoviewitem.h"
#include "kotodoview.h"

#include <libkcal/calendar.h>
#include <libkcal/calfilter.h>
#include <libkdepim/categoryhierarchyreader.h>

#include <korganizer/mainwindow.h>

#include "koprefs.h"

#include <kaction.h>
#include <klistviewsearchline.h>
#include <ktoolbar.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qsizepolicy.h>
#include <qtimer.h>

using namespace KPIM;
using namespace KCal;

KAction *KOTodoListViewQuickSearch::action = 0;

KOTodoListViewQuickSearch::KOTodoListViewQuickSearch( QWidget *parent,
                                            QList<KListView*> listViews,
                                            KActionCollection *actionCollection,
                                            Calendar *calendar,
                                            const char *name )
  : KToolBar( parent, name ), mCategoryCombo( 0 ), mCalendar( calendar ),
    mQuickSearchLine( 0 )
{
  if ( !action ) {
    action = new KAction( i18n( "Reset To-do Quick Search" ),
                          QApplication::isRightToLeft() ? "clear_left" :
                          "locationbar_erase", 0, this, SLOT( reset() ),
                          actionCollection, "reset_quicksearch" );
    action->setWhatsThis( i18n( "Reset Quick Search\n"
                                  "Resets the quick search so that "
                                  "all to-dos are shown again." ) );
  }

  action->plug( this );

  boxLayout()->setSpacing( KDialog::spacingHint() );

  mSearchLabel = new QLabel( i18n("Sea&rch:"), this,
                              "kde toolbar widget" );

  mQuickSearchLine = new KOTodoListViewQuickSearchLine( this, listViews );
  setStretchableWidget( mQuickSearchLine );

  mSearchLabel->setBuddy( mQuickSearchLine );

  mCategoryLabel = new QLabel( i18n("&Category:"), this, "kde toolbar widget" );

  mCategoryCombo = new QComboBox( this, "quick search category combo box" );
  fillCategories();

  mCategoryCombo->setCurrentItem( 0 );
  connect( mCategoryCombo, SIGNAL ( activated( int ) ),
           this, SLOT( slotCategoryChanged( int ) ) );

  mCategoryLabel->setBuddy( mCategoryCombo );
}

KOTodoListViewQuickSearch::~KOTodoListViewQuickSearch()
{
}

bool KOTodoListViewQuickSearchLine::itemMatches(const Q3ListViewItem *item,
                                                const QString &s)
const
{
  while ( item ) {
    const Todo *todo = static_cast<const KOTodoViewItem *>( item )->todo();
    if ( ( mCategory.isNull() ||
           !todo->categories().grep( QRegExp( QString( "^" ) +
           QRegExp::escape( mCategory ) ) ).isEmpty() ) &&
           KListViewSearchLine::itemMatches(item, s) )
      return true;
    else
      item = item->parent(); // children of passed items also pass
  }
  return false;
}

//-----------------------------------------------------------------------------
void KOTodoListViewQuickSearch::reset()
{
  mQuickSearchLine->clear();
  mCategoryCombo->setCurrentItem( 0 );
  slotCategoryChanged( 0 );
}

void KOTodoListViewQuickSearch::slotCategoryChanged( int index )
{
  if ( index == 0 )
    mQuickSearchLine->setCategory( QString::null );
  else
    mQuickSearchLine->setCategory( categoryList[index - 1] );
  mQuickSearchLine->updateSearch();
}

void KOTodoListViewQuickSearch::fillCategories()
{
  QString current = mCategoryCombo->currentItem() > 0 ?
    categoryList[mCategoryCombo->currentItem() - 1] : QString::null;
  QStringList categories;

  CalFilter *filter = mCalendar->filter();
  if ( filter && ( filter->criteria() & CalFilter::ShowCategories ) ) {
    categories = filter->categoryList();
    categories.sort();
  } else {
    categories = KOPrefs::instance()->mCustomCategories;
    QStringList filterCategories = filter->categoryList();
    categories.sort();
    filterCategories.sort();

    QStringList::Iterator it = categories.begin();
    QStringList::Iterator jt = filterCategories.begin();
    while ( it != categories.end() && jt != filterCategories.end() )
      if ( *it == *jt ) {
        QStringList::Iterator next = it;
        next++;
        categories.remove( it );
        jt++;
        it = next;
      } else if ( *it < *jt )
        it++;
      else if ( *it > *jt )
        jt++;
  }

  CategoryHierarchyReaderQComboBox( mCategoryCombo ).read( categories );
  mCategoryCombo->insertItem( i18n( "Any category" ), 0 );

  categoryList.resize( categories.count() );
  qCopy( categories.begin(), categories.end(), categoryList.begin() );

  if ( current.isNull() ) {
    mCategoryCombo->setCurrentItem( 0 );
  } else {
    for ( int i = 0; i < categoryList.count(); ++i )
      if ( categoryList[i] == current ) {
        mCategoryCombo->setCurrentItem( i + 1 );
        break;
      }
  }

}

void KOTodoListViewQuickSearch::setCalendar( Calendar *calendar )
{
  mCalendar = calendar;
  mQuickSearchLine->updateSearch();
}

void KOTodoListViewQuickSearch::resizeEvent( QResizeEvent *e )
{
  int w = width() - mCategoryCombo->sizeHint().width()
                  - mCategoryLabel->sizeHint().width()
                  - mSearchLabel->sizeHint().width();
  int halfw = width() / 2;

  if ( w < halfw ) {
    w += mCategoryLabel->sizeHint().width();
    mCategoryLabel->hide();
  } else
    mCategoryLabel->show();
  if ( w < halfw ) {
    w += mSearchLabel->sizeHint().width();
    mSearchLabel->hide();
  } else
    mSearchLabel->show();
  if ( w < halfw ) {
    slotCategoryChanged( 0 );
    mCategoryCombo->hide();
  } else {
    slotCategoryChanged( mCategoryCombo->currentItem() );
    mCategoryCombo->show();
  }

  KToolBar::resizeEvent( e );
}

void KOTodoListViewQuickSearch::showEvent( QShowEvent *e )
{
  connect( action, SIGNAL( activated() ), this, SLOT( reset() ) );

  KToolBar::showEvent( e );
}

void KOTodoListViewQuickSearch::hideEvent( QHideEvent *e )
{
  disconnect( action, SIGNAL( activated() ), this, SLOT( reset() ) );

  KToolBar::hideEvent( e );
}

KOTodoListViewQuickSearchContainer::KOTodoListViewQuickSearchContainer(
                               QWidget *parent,
                               QList<KListView*> listViews,
                               KActionCollection *actionCollection,
                               Calendar *calendar)
     : QWidget( parent ), mQuickSearch( new KOTodoListViewQuickSearch(
         this, listViews, actionCollection, calendar, "search toolbar" ) )
{
  setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
}

KOTodoListViewQuickSearchContainer::~KOTodoListViewQuickSearchContainer()
{
}

QSize KOTodoListViewQuickSearchContainer::sizeHint() const
{
  int width = KDialog::spacingHint();
  QList<QObject*> list = mQuickSearch->children();
  for ( QList<QObject*>::Iterator it = list.begin(); it != list.end(); ++it ) {
    QWidget *child = dynamic_cast<QWidget *>( *it );
    if ( child ) {
      width += child->sizeHint().width() + KDialog::spacingHint();
    }
  }

  return QSize( width, mQuickSearch->sizeHint().height() );
}

QSize KOTodoListViewQuickSearchContainer::minimumSizeHint() const
{
  return QSize( mQuickSearch->iconSize() +
                mQuickSearch->mQuickSearchLine->minimumSizeHint().width() +
                3 * KDialog::spacingHint(),
                mQuickSearch->minimumSizeHint().height() );
}

KOTodoListViewQuickSearch *KOTodoListViewQuickSearchContainer::quickSearch()
                                                                          const
{
  return mQuickSearch;
}

void KOTodoListViewQuickSearchContainer::resizeEvent ( QResizeEvent */*e*/ )
{
  mQuickSearch->setGeometry( QRect( QPoint( 0, 0 ), size() ) );
}


#include "kotodoviewquicksearch.moc"
