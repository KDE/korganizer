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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KLISTVIEWNEWSEARCHLINE_H
#define KLISTVIEWNEWSEARCHLINE_H

#include <klineedit.h>
#include <qhbox.h>
#include <qvaluelist.h>

class KListView;
class QListViewItem;
class QToolButton;

/* FIXME push changes into kdelibs */

/**
 * This class makes it easy to add a search line for filtering the items in
 * listviews based on a simple text search.
 *
 * No changes to the application other than instantiating this class with
 * appropriate KListViews should be needed.
 *
 * @since 3.3
 */

class KDEUI_EXPORT KListViewNewSearchLine : public KLineEdit
{
    Q_OBJECT

public:

    /**
     * Constructs a KListViewNewSearchLine with \a listView being the KListView to
     * be filtered.
     *
     * If \a listView is null then the widget will be disabled until listviews
     * are set with setListView(), setListViews() or added with addListView().
     */
    KListViewNewSearchLine(QWidget *parent = 0, KListView *listView = 0,
                           const char *name = 0);

    /**
     * Constructs a KListViewNewSearchLine with \a listViews being the list of
     * pointers to KListViews to be filtered.
     *
     * If \a listViews is empty then the widget will be disabled until listviews
     * are set with setListView(), setListViews() or added with addListView().
     *
     * @since 4.0
     */
    KListViewNewSearchLine(QWidget *parent,
                           const QValueList<KListView *> &listViews,
                           const char *name = 0);

    /**
     * Constructs a KListViewNewSearchLine without any KListView to filter. The
     * KListView objects have to be set later with setListView(), setListViews()
     * or added with addListView().
     */
    KListViewNewSearchLine(QWidget *parent, const char *name);

    /**
     * Destroys the KListViewNewSearchLine.
     */
    virtual ~KListViewNewSearchLine();

    /**
     * Returns true if the search is case sensitive.  This defaults to false.
     *
     * @see setCaseSensitive()
     */
    bool caseSensitive() const;

    /**
     * Returns the current list of columns that will be searched.  If the
     * returned list is empty all visible columns will be searched.
     *
     * @see setSearchColumns
     */
    QValueList<int> searchColumns() const;

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
    KListView *listView() const;

    /**
     * Returns the list of pointers to listviews that are currently filtered by
     * the search.
     *
     * @see setListViews(), addListView(), listView()
     * @since 4.0
     */
    const QValueList<KListView *> &listViews() const;

public slots:
    /**
     * Adds a KListView to the list of listviews filtered by this search line.
     * If \a lv is null then the widget will be disabled.
     *
     * @see listView(), setListViews(), removeListView()
     * @since 4.0
     */
    void addListView(KListView *lv);

    /**
     * Removes a KListView from the list of listviews filtered by this search
     * line. Does nothing if \a lv is 0 or is not filtered by the quick search
     * line.
     *
     * @see listVew(), setListViews(), addListView()
     * @since 4.0
     */
    void removeListView(KListView *lv);

    /**
     * Updates search to only make visible the items that match \a s.  If
     * \a s is null then the line edit's text will be used.
     */
    virtual void updateSearch(const QString &s = QString::null);

    /**
     * Make the search case sensitive or case insensitive.
     *
     * @see caseSenstive()
     */
    void setCaseSensitive(bool cs);

    /**
     * When a search is active on a list that's organized into a tree view if
     * a parent or ancesestor of an item is does not match the search then it
     * will be hidden and as such so too will any children that match.
     *
     * If this is set to true (the default) then the parents of matching items
     * will be shown.
     *
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
     * @see searchColumns
     */
    void setSearchColumns(const QValueList<int> &columns);

    /**
     * Sets the KListView that is filtered by this search line, replacing any
     * previously filtered listviews.  If \a lv is null then the widget will be
     * disabled.
     *
     * @see listView(), setListViews()
     */
    void setListView(KListView *lv);

    /**
     * Sets KListViews that are filtered by this search line, replacing any
     * previously filtered listviews.  If \a lvs is empty then the widget will
     * be disabled.
     *
     * @see listViews(), addListView(), setListView()
     * @since 4.0
     */
    void setListViews(const QValueList<KListView *> &lv);


  protected:

    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     */
    virtual bool itemMatches(const QListViewItem *item, const QString &s) const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::mousePressEvent().
    */
    virtual QPopupMenu *createPopupMenu();

    /**
     * Updates search to only make visible appropriate items in \a listView.  If
     * \a listView is null then nothing is done.
     */
    virtual void updateSearch(KListView *listView);

    /**
     * Connects signals of this listview to the appropriate slots of the search
     * line.
     *
     * @since 4.0
     */
    virtual void connectListView(KListView *);
    /**
     * Disconnects signals of a listviews from the search line.
     *
     * @since 4.0
     */
    virtual void disconnectListView(KListView *);

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
     * @since 4.0
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
     * @since 4.0
     */
    void checkColumns();
    
    /**
     * This is used in case parent items of matching items shouldn't be
     * visible.  It hides all items that don't match the search string.
     */
    void checkItemParentsNotVisible(KListView *listView);

    /**
     * This is used in case parent items of matching items should be visible.
     * It makes a recursive call to all children.  It returns true if at least
     * one item in the subtree with the given root item is visible.
     */
    bool checkItemParentsVisible(QListViewItem *item, QListViewItem *highestHiddenParent = 0);

private slots:
    void itemAdded(QListViewItem *item) const;
    void listViewDeleted( QObject *listView );
    void searchColumnsMenuActivated(int);

private:
    class KListViewNewSearchLinePrivate;
    KListViewNewSearchLinePrivate *d;
};

// FIXME put KListViewSearchLineWidget for kdelibs here of course
// (needed to remove because moc doesn't understand #if 0

#endif
