/*
  This file is part of KOrganizer.

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

#ifndef KLISTVIEWNEWSEARCHLINE_H
#define KLISTVIEWNEWSEARCHLINE_H

#include "korganizer_export.h"
#include <klineedit.h>

class K3ListView;
class Q3ListViewItem;
class QContextMenuEvent;

/* FIXME push changes into kdelibs */

/**
 * This class makes it easy to add a search line for filtering the items in
 * listviews based on a simple text search.
 *
 * No changes to the application other than instantiating this class with
 * appropriate KListViews should be needed.
 */

class KORGANIZER_EXPORT KListViewNewSearchLine : public KLineEdit
{
  Q_OBJECT
  public:

    /**
     * Constructs a KListViewNewSearchLine with \a listView being the K3ListView to
     * be filtered.
     *
     * If \a listView is null then the widget will be disabled until listviews
     * are set with setListView(), setListViews() or added with addListView().
     * @param parent The parent widget
     * @param listView The list view that is filtered by this line
     */
    explicit KListViewNewSearchLine(QWidget *parent = 0, K3ListView *listView = 0);

    /**
     * Constructs a KListViewNewSearchLine with \a listViews being the list of
     * pointers to KListViews to be filtered.
     *
     * If \a listViews is empty then the widget will be disabled until listviews
     * are set with setListView(), setListViews() or added with addListView().
     * @param parent The parent widget
     * @param listViews List of ListViews that are filtered by this line
     */
    KListViewNewSearchLine(QWidget *parent,
                           const QList<K3ListView *> &listViews);

    /**
     * Constructs a KListViewNewSearchLine without any K3ListView to filter. The
     * K3ListView objects have to be set later with setListView(), setListViews()
     * or added with addListView().
     * @param parent The parent widget
     */
    KListViewNewSearchLine(QWidget *parent);

    /**
     * Destroys the KListViewNewSearchLine.
     */
    virtual ~KListViewNewSearchLine();

    /**
     * Returns true if the search is case sensitive.  This defaults to false.
     *
     * @see setCaseSensitive()
     */
    Qt::CaseSensitivity caseSensitive() const;

    /**
     * Returns the current list of columns that will be searched.  If the
     * returned list is empty all visible columns will be searched.
     *
     * @see setSearchColumns
     */
    QList<int> searchColumns() const;

    /**
     * If this is true (the default) then the parents of matched items will also
     * be shown.
     *
     * @see setKeepParentsVisible()
     */
    bool keepParentsVisible() const;

    /**
     * Returns the listview that is currently filtered by the search.
     * If there are multiple listviews filtered, it returns 0.
     *
     * @see setListView(), listViews()
     */
    K3ListView *listView() const;

    /**
     * Returns the list of pointers to listviews that are currently filtered by
     * the search.
     *
     * @see setListViews(), addListView(), listView()
     */
    const QList<K3ListView *> &listViews() const;

public slots:
    /**
     * Adds a K3ListView to the list of listviews filtered by this search line.
     * If \a lv is null then the widget will be disabled.
     *
     * @param lv The list view to be added to the views that are filtered
     * @see listView(), setListViews(), removeListView()
     */
    void addListView(K3ListView *lv);

    /**
     * Removes a K3ListView from the list of listviews filtered by this search
     * line. Does nothing if \a lv is 0 or is not filtered by the quick search
     * line.
     *
     * @param lv The list view on which this line should no longer act
     * @see listVew(), setListViews(), addListView()
     */
    void removeListView(K3ListView *lv);

    /**
     * Updates search to only make visible the items that match \a s.  If
     * \a s is null then the line edit's text will be used.
     *
     * @param s The new search text
     */
    virtual void updateSearch(const QString &s = QString());

    /**
     * Make the search case sensitive or case insensitive.
     *
     * @param cs Whether to use case sensitive of insensitive comparisons
     * @see caseSenstive()
     */
    void setCaseSensitive(Qt::CaseSensitivity cs);

    /**
     * When a search is active on a list that's organized into a tree view if
     * a parent or ancesestor of an item is does not match the search then it
     * will be hidden and as such so too will any children that match.
     *
     * If this is set to true (the default) then the parents of matching items
     * will be shown.
     *
     * @param v If true, parents of unfiltered items will also be unfiltered.
                If false, only the unfiltered child will be visible when the
                parent is filtered out.
     * @see keepParentsVisible
     */
    void setKeepParentsVisible(bool v);

    /**
     * Sets the list of columns to be searched.  The default is to search all,
     * visible columns which can be restored by passing \a columns as an empty
     * list.
     * If listviews to be filtered have different numbers or labels of columns
     * this method has no effect.
     *
     * @param columns The list of columns which should be used for matching
     * @see searchColumns
     */
    void setSearchColumns(const QList<int> &columns);

    /**
     * Sets the K3ListView that is filtered by this search line, replacing any
     * previously filtered listviews.  If \a lv is null then the widget will be
     * disabled.
     *
     * @param lv The list vieww on which this line should act
     * @see listView(), setListViews()
     */
    void setListView(K3ListView *lv);

    /**
     * Sets KListViews that are filtered by this search line, replacing any
     * previously filtered listviews.  If \a lvs is empty then the widget will
     * be disabled.
     *
     * @param lv The list of listviews on which this line should act
     * @see listViews(), addListView(), setListView()
     */
    void setListViews(const QList<K3ListView *> &lv);


  protected:

    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     *  @param item The item to be compared to the string
     *  @param s The pattern to which the item is compared
     */
    virtual bool itemMatches(const Q3ListViewItem *item, const QString &s) const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::mousePressEvent().
    * @param e the context menu event
    */
    virtual void contextMenuEvent( QContextMenuEvent *e );

    /**
     * Updates search to only make visible appropriate items in \a listView.  If
     * \a listView is null then nothing is done.
     *   @param listView the List view that should be updated.
     */
    virtual void updateSearch(K3ListView *listView);

    /**
     * Connects signals of this listview to the appropriate slots of the search
     * line.
     */
    virtual void connectListView(K3ListView *);
    /**
     * Disconnects signals of a listviews from the search line.
     */
    virtual void disconnectListView(K3ListView *);

    /**
     * Checks columns in all listviews and decides whether choosing columns to
     * filter on makes any sense.
     *
     * Returns false if either of the following is true:
     * * there are no listviews connected,
     * * the listviews have different numbers of columns,
     * * the listviews have only one column,
     * * the listviews differ in column labels.
     *
     * Otherwise it returns true.
     *
     * @see setSearchColumns()
     */
    virtual bool canChooseColumnsCheck();

protected slots:
    /**
     * When keys are pressed a new search string is created and a timer is
     * activated.  The most recent search is activated when this timer runs out
     * if another key has not yet been pressed.
     *
     * This method makes @param search the most recent search and starts the
     * timer.
     *
     * Together with activateSearch() this makes it such that searches are not
     * started until there is a short break in the users typing.
     *
     * @see activateSearch()
     */
    void queueSearch(const QString &search);

    /**
     * When the timer started with queueSearch() expires this slot is called.
     * If there has been another timer started then this slot does nothing.
     * However if there are no other pending searches this starts the list view
     * search.
     *
     * @see queueSearch()
     */
    void activateSearch();

private:

    /**
     * This is used after changing the list of listviews. If choosing columns
     * doesn't make sense, it forces filtering over all columns.
     *
     * @see canChooseColumnsCheck()
     */
    void checkColumns();

    /**
     * This is used in case parent items of matching items shouldn't be
     * visible.  It hides all items that don't match the search string.
     */
    void checkItemParentsNotVisible(K3ListView *listView);

    /**
     * This is used in case parent items of matching items should be visible.
     * It makes a recursive call to all children.  It returns true if at least
     * one item in the subtree with the given root item is visible.
     */
    bool checkItemParentsVisible(Q3ListViewItem *item, Q3ListViewItem *highestHiddenParent = 0);

private slots:
    void itemAdded(Q3ListViewItem *item) const;
    void listViewDeleted( QObject *listView );
    void searchColumnsMenuActivated(int);

private:
    class KListViewNewSearchLinePrivate;
    KListViewNewSearchLinePrivate *d;
};

// FIXME put KListViewSearchLineWidget for kdelibs here of course
// (needed to remove because moc doesn't understand #if 0

#endif
