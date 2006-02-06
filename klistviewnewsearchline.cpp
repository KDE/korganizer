/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>
   Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/* FIXME push changes into kdelibs */

#include "klistviewnewsearchline.h"

#include <klistview.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kdebug.h>
#include <klocale.h>

#include <qapplication.h>
#include <qtimer.h>
#include <q3popupmenu.h>
#include <qlabel.h>
#include <q3header.h>
//Added by qt3to4:
#include <QContextMenuEvent>

#define KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID 2004

class KListViewNewSearchLine::KListViewNewSearchLinePrivate
{
public:
    KListViewNewSearchLinePrivate() :
        caseSensitive(false),
        activeSearch(false),
        keepParentsVisible(true),
        canChooseColumns(true),
        queuedSearches(0) {}

    QList<KListView *> listViews;
    bool caseSensitive;
    bool activeSearch;
    bool keepParentsVisible;
    bool canChooseColumns;
    QString search;
    int queuedSearches;
    QList<int> searchColumns;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

KListViewNewSearchLine::KListViewNewSearchLine(QWidget *parent, KListView *listView) :
    KLineEdit(parent)
{
    d = new KListViewNewSearchLinePrivate;

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setListView( listView );
}

KListViewNewSearchLine::KListViewNewSearchLine(QWidget *parent,
                                       const QList<KListView *> &listViews) :
     KLineEdit(parent)
{
    d = new KListViewNewSearchLinePrivate;

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setListViews( listViews );
}


KListViewNewSearchLine::KListViewNewSearchLine(QWidget *parent) :
    KLineEdit(parent)
{
    d = new KListViewNewSearchLinePrivate;

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setEnabled(false);
}

KListViewNewSearchLine::~KListViewNewSearchLine()
{
    delete d;
}

bool KListViewNewSearchLine::caseSensitive() const
{
    return d->caseSensitive;
}

QList<int> KListViewNewSearchLine::searchColumns() const
{
    if (d->canChooseColumns)
        return d->searchColumns;
    else
        return QList<int>();
}

bool KListViewNewSearchLine::keepParentsVisible() const
{
    return d->keepParentsVisible;
}

KListView *KListViewNewSearchLine::listView() const
{
    if ( d->listViews.count() == 1 )
        return d->listViews.first();
    else
        return 0;
}

const QList<KListView *> &KListViewNewSearchLine::listViews() const
{
    return d->listViews;
}


////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void KListViewNewSearchLine::addListView(KListView *lv)
{
    if (lv) {
        connectListView(lv);
        
        d->listViews.append(lv);
        setEnabled(!d->listViews.isEmpty());
        
        checkColumns();
    }
}

void KListViewNewSearchLine::removeListView(KListView *lv)
{
    if (lv) {
        QList<KListView *>::Iterator it = d->listViews.find(lv);
        
        if ( it != d->listViews.end() ) {
            d->listViews.remove( it );
            checkColumns();

            disconnectListView(lv);
            
            setEnabled(!d->listViews.isEmpty());
        }
    }
}

void KListViewNewSearchLine::updateSearch(const QString &s)
{
    d->search = s.isNull() ? text() : s;

    for (QList<KListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
        updateSearch( *it );
}

void KListViewNewSearchLine::updateSearch(KListView *listView)
{
    if(!listView)
        return;


    // If there's a selected item that is visible, make sure that it's visible
    // when the search changes too (assuming that it still matches).

    Q3ListViewItem *currentItem = 0;

    switch(listView->selectionMode())
    {
    case KListView::NoSelection:
        break;
    case KListView::Single:
        currentItem = listView->selectedItem();
        break;
    default:
    {
        int flags = Q3ListViewItemIterator::Selected | Q3ListViewItemIterator::Visible;
        for(Q3ListViewItemIterator it(listView, flags);
            it.current() && !currentItem;
            ++it)
        {
            if(listView->itemRect(it.current()).isValid())
                currentItem = it.current();
        }
    }
    }

    if(d->keepParentsVisible)
        checkItemParentsVisible(listView->firstChild());
    else
        checkItemParentsNotVisible(listView);

    if(currentItem)
        listView->ensureItemVisible(currentItem);
}

void KListViewNewSearchLine::setCaseSensitive(bool cs)
{
    d->caseSensitive = cs;
}

void KListViewNewSearchLine::setKeepParentsVisible(bool v)
{
    d->keepParentsVisible = v;
}

void KListViewNewSearchLine::setSearchColumns(const QList<int> &columns)
{
    if (d->canChooseColumns)
        d->searchColumns = columns;
}

void KListViewNewSearchLine::setListView(KListView *lv)
{
    setListViews(QList<KListView *>());
    addListView(lv);
}

void KListViewNewSearchLine::setListViews(const QList<KListView *> &lv)
{
    for (QList<KListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
             disconnectListView(*it);
    
    d->listViews = lv;

    for (QList<KListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
        connectListView(*it);

    checkColumns();
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

bool KListViewNewSearchLine::itemMatches(const Q3ListViewItem *item, const QString &s) const
{
    if(s.isEmpty())
        return true;

    // If the search column list is populated, search just the columns
    // specifified.  If it is empty default to searching all of the columns.

    if(!d->searchColumns.isEmpty()) {
        QList<int>::ConstIterator it = d->searchColumns.begin();
        for(; it != d->searchColumns.end(); ++it) {
            if(*it < item->listView()->columns() &&
               item->text(*it).find(s, 0, d->caseSensitive) >= 0)
                return true;
        }
    }
    else {
        for(int i = 0; i < item->listView()->columns(); i++) {
            if(item->listView()->columnWidth(i) > 0 &&
               item->text(i).find(s, 0, d->caseSensitive) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

void KListViewNewSearchLine::contextMenuEvent( QContextMenuEvent *e )
{
  QMenu *popup = KLineEdit::createStandardContextMenu();

  if (d->canChooseColumns) {
    QMenu *subMenu = new QMenu(popup);
    connect(subMenu, SIGNAL(activated(int)), this, SLOT(searchColumnsMenuActivated(int)));

    popup->insertSeparator();
    popup->insertItem(i18n("Search Columns"), subMenu);
    
    subMenu->insertItem(i18n("All Visible Columns"), KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID);
    subMenu->insertSeparator();
    
    bool allColumnsAreSearchColumns = true;
    // TODO Make the entry order match the actual column order
    Q3Header* const header = d->listViews.first()->header();
    int visibleColumns = 0;
    for(int i = 0; i < d->listViews.first()->columns(); i++) {
      if(d->listViews.first()->columnWidth(i)>0) {
        QString columnText = d->listViews.first()->columnText(i);
        if(columnText.isEmpty()) {
          int visiblePosition=1;
          for(int j = 0; j < header->mapToIndex(i); j++)
            if(d->listViews.first()->columnWidth(header->mapToSection(j))>0)
              visiblePosition++;
          columnText = i18n("Column number %1","Column No. %1").arg(visiblePosition);
        }
        subMenu->insertItem(columnText, visibleColumns);
        if(d->searchColumns.isEmpty() || d->searchColumns.find(i) != d->searchColumns.end())
          subMenu->setItemChecked(visibleColumns, true);
        else
          allColumnsAreSearchColumns = false;
        visibleColumns++;
      }
    }
    subMenu->setItemChecked(KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID, allColumnsAreSearchColumns);
    
        // searchColumnsMenuActivated() relies on one possible "all" representation
    if(allColumnsAreSearchColumns && !d->searchColumns.isEmpty())
      d->searchColumns.clear();
  }
    
  popup->exec( e->globalPos() );
  delete popup;
}

void KListViewNewSearchLine::connectListView(KListView *lv)
{
    connect(lv, SIGNAL(destroyed( QObject * )),
            this, SLOT(listViewDeleted( QObject *)));
    connect(lv, SIGNAL(itemAdded(Q3ListViewItem *)),
            this, SLOT(itemAdded(Q3ListViewItem *)));
}

void KListViewNewSearchLine::disconnectListView(KListView *lv)
{
    disconnect(lv, SIGNAL(destroyed( QObject * )),
            this, SLOT(listViewDeleted( QObject *)));
    disconnect(lv, SIGNAL(itemAdded(Q3ListViewItem *)),
            this, SLOT(itemAdded(Q3ListViewItem *)));
}

bool KListViewNewSearchLine::canChooseColumnsCheck()
{
    // This is true if either of the following is true:

    // there are no listviews connected
    if (d->listViews.isEmpty())
        return false;

    const KListView *first = d->listViews.first();
    
    const unsigned int numcols = first->columns();
    // the listviews have only one column,
    if (numcols < 2)
        return false;

    QStringList headers;
    for (unsigned int i = 0; i < numcols; ++i)
        headers.append(first->columnText(i));

    QList<KListView *>::ConstIterator it = d->listViews.constBegin();
    for (++it /* skip the first one */; it !=d->listViews.constEnd(); ++it) {
        // the listviews have different numbers of columns,
        if ((unsigned int) (*it)->columns() != numcols)
            return false;

        // the listviews differ in column labels.
        QStringList::ConstIterator jt;
        unsigned int i;
        for (i = 0, jt = headers.constBegin(); i < numcols; ++i, ++jt) {
                Q_ASSERT(jt != headers.constEnd());
                if ((*it)->columnText(i) != *jt)
                    return false;
            }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// protected slots
////////////////////////////////////////////////////////////////////////////////

void KListViewNewSearchLine::queueSearch(const QString &search)
{
    d->queuedSearches++;
    d->search = search;
    QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void KListViewNewSearchLine::activateSearch()
{
    --(d->queuedSearches);

    if(d->queuedSearches == 0)
        updateSearch(d->search);
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void KListViewNewSearchLine::itemAdded(Q3ListViewItem *item) const
{
    item->setVisible(itemMatches(item, text()));
}

void KListViewNewSearchLine::listViewDeleted(QObject *o)
{
    KListView *lv = dynamic_cast<KListView *>(o);
    if (!lv) {
        kWarning() << k_funcinfo << "an object other than KListView passed"
                << endl;
        return;
    }
    
    d->listViews.remove(lv);
    setEnabled(d->listViews.isEmpty());
}

void KListViewNewSearchLine::searchColumnsMenuActivated(int id)
{
    if(id == KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID) {
        if(d->searchColumns.isEmpty())
            d->searchColumns.append(0);
        else
            d->searchColumns.clear();
    }
    else {
        if(d->searchColumns.find(id) != d->searchColumns.end())
            d->searchColumns.remove(id);
        else {
            if(d->searchColumns.isEmpty()) {
                for(int i = 0; i < d->listViews.first()->columns(); i++) {
                    if(i != id)
                        d->searchColumns.append(i);
                }
            }
            else
                d->searchColumns.append(id);
        }
    }
    updateSearch();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void KListViewNewSearchLine::checkColumns()
{
    d->canChooseColumns = canChooseColumnsCheck();
}

void KListViewNewSearchLine::checkItemParentsNotVisible(KListView *listView)
{
    Q3ListViewItemIterator it(listView);
    for(; it.current(); ++it)
    {
        Q3ListViewItem *item = it.current();
        if(itemMatches(item, d->search))
            item->setVisible(true);
        else
            item->setVisible(false);
    }
}

#include <kdebug.h>
#include <kvbox.h>

/** Check whether \p item, its siblings and their descendents should be shown. Show or hide the items as necessary.
 *
 *  \p item  The list view item to start showing / hiding items at. Typically, this is the first child of another item, or the
 *              the first child of the list view.
 *  \p highestHiddenParent  The highest (closest to root) ancestor of \p item which is hidden. If 0, all parents of
 *                           \p item must be visible.
 *  \return \c true if an item which should be visible is found, \c false if all items found should be hidden. If this function
 *             returns true and \p highestHiddenParent was not 0, highestHiddenParent will have been shown.
 */
bool KListViewNewSearchLine::checkItemParentsVisible(Q3ListViewItem *item, Q3ListViewItem *highestHiddenParent)
{
    bool visible = false;
    Q3ListViewItem * first = item;
    for(; item; item = item->nextSibling())
    {
        //What we pass to our children as highestHiddenParent:
        Q3ListViewItem * hhp = highestHiddenParent ? highestHiddenParent : item->isVisible() ? 0L : item;
        bool childMatch = false;
        if(item->firstChild() && checkItemParentsVisible(item->firstChild(), hhp))
            childMatch = true;
        // Should this item be shown? It should if any children should be, or if it matches.
        if(childMatch || itemMatches(item, d->search))
        {
            visible = true;
            if (highestHiddenParent)
            {
                highestHiddenParent->setVisible(true);
                // Calling setVisible on our ancestor will unhide all its descendents. Hide the ones
                // before us that should not be shown.
                for(Q3ListViewItem *hide = first; hide != item; hide = hide->nextSibling())
                    hide->setVisible(false);
                highestHiddenParent = 0;
                // If we matched, than none of our children matched, yet the setVisible() call on our
                // ancestor unhid them, undo the damage:
                if(!childMatch)
                    for(Q3ListViewItem *hide = item->firstChild(); hide; hide = hide->nextSibling())
                        hide->setVisible(false);
            }
            else
                item->setVisible(true);
        }
        else
            item->setVisible(false);
    }
    return visible;
}

#if 0 // FIXME uncomment in libs, of course
////////////////////////////////////////////////////////////////////////////////
// KListViewSearchLineWidget
////////////////////////////////////////////////////////////////////////////////

class KListViewSearchLineWidget::KListViewSearchLineWidgetPrivate
{
public:
    KListViewSearchLineWidgetPrivate() : listView(0), searchLine(0), clearButton(0) {}
    KListView *listView;
    KListViewSearchLine *searchLine;
    QToolButton *clearButton;
};

KListViewSearchLineWidget::KListViewSearchLineWidget(KListView *listView,
                                                     QWidget *parent,
                                                     const char *name) :
    KHBox(parent, name)
{
    d = new KListViewSearchLineWidgetPrivate;
    d->listView = listView;

    setSpacing(5);

    QTimer::singleShot(0, this, SLOT(createWidgets()));
}

KListViewSearchLineWidget::~KListViewSearchLineWidget()
{
    delete d;
}

KListViewSearchLine *KListViewSearchLineWidget::createSearchLine(KListView *listView)
{
    if(!d->searchLine)
        d->searchLine = new KListViewSearchLine(this, listView);
    return d->searchLine;
}

void KListViewSearchLineWidget::createWidgets()
{
    positionInToolBar();

    if(!d->clearButton) {
        d->clearButton = new QToolButton(this);
        QIcon icon = SmallIconSet(QApplication::isRightToLeft() ? "clear_left" : "locationbar_erase");
        d->clearButton->setIconSet(icon);
    }

    d->clearButton->show();

    QLabel *label = new QLabel(i18n("S&earch:"), this, "kde toolbar widget");

    d->searchLine = createSearchLine(d->listView);
    d->searchLine->show();

    label->setBuddy(d->searchLine);
    label->show();

    connect(d->clearButton, SIGNAL(clicked()), d->searchLine, SLOT(clear()));
}

KListViewSearchLine *KListViewSearchLineWidget::searchLine() const
{
    return d->searchLine;
}

void KListViewSearchLineWidget::positionInToolBar()
{
    KToolBar *toolBar = dynamic_cast<KToolBar *>(parent());

    if(toolBar) {

        // Here we have The Big Ugly.  Figure out how many widgets are in the
        // and do a hack-ish iteration over them to find this widget so that we
        // can insert the clear button before it.

        int widgetCount = toolBar->count();

        for(int index = 0; index < widgetCount; index++) {
            int id = toolBar->idAt(index);
            if(toolBar->getWidget(id) == this) {
                toolBar->setItemAutoSized(id);
                if(!d->clearButton) {
                    QString icon = QApplication::isRightToLeft() ? "clear_left" : "locationbar_erase";
                    d->clearButton = new KToolBarButton(icon, 2005, toolBar);
                }
                toolBar->insertWidget(2005, d->clearButton->width(), d->clearButton, index);
                break;
            }
        }
    }

    if(d->searchLine)
        d->searchLine->show();
}
#endif

#include "klistviewnewsearchline.moc"
