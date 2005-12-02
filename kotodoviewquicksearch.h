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

#ifndef KOTODOVIEWQUICKSEARCH_H
#define KOTODOVIEWQUICKSEARCH_H

#include <klistviewsearchline.h>
#include <ktoolbar.h>
#include <qvector.h>

class KActionCollection;
class KAction;
class KListView;
class QComboBox;
class QListViewItem;
class QResizeEvent;
class QLabel;
class QShowEvent;
namespace KCal { class Calendar; }

using namespace KCal;

class KOTodoListViewQuickSearchLine : public KListViewSearchLine
{
  Q_OBJECT
  Q_PROPERTY( QString category READ category WRITE setCategory )
  public:
    KOTodoListViewQuickSearchLine( QWidget *parent=0, QList<KListView*> listViews )
        : KListViewSearchLine( parent, listViews ) { }
    virtual ~KOTodoListViewQuickSearchLine() { }

    void setCategory( const QString &category ) { mCategory = category; }
    const QString &category() const { return mCategory; }
  protected:
    virtual bool itemMatches(const Q3ListViewItem *item, const QString &s) const;
  private:
    QString mCategory;
};

class KOTodoListViewQuickSearchContainer;

class KOTodoListViewQuickSearch : public KToolBar
{
  Q_OBJECT
  friend class KOTodoListViewQuickSearchContainer;
  public:
    KOTodoListViewQuickSearch( QWidget *parent,
                               QList<KListView*> listViews,
                               KActionCollection *actionCollection,
                               Calendar *calendar,
                               const char *name = 0 );
    virtual ~KOTodoListViewQuickSearch();

    void setCalendar( Calendar *calendar );

  public slots:
    void reset();
    void fillCategories();

  private slots:
    void slotCategoryChanged( int index );

  protected:
    void resizeEvent ( QResizeEvent *e );
    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );

  private:
    /** Helper method for the filling of the category combo. */
    void insertCategory(QString which);
    QComboBox *mCategoryCombo;
    QVector<QString> categoryList;
    Calendar *mCalendar;
    KOTodoListViewQuickSearchLine *mQuickSearchLine;
    QLabel *mCategoryLabel, *mSearchLabel;
    static KAction *action;
};

class KOTodoListViewQuickSearchContainer : public QWidget
{
  Q_OBJECT
  public:
    KOTodoListViewQuickSearchContainer( QWidget *parent,
                               QList<KListView*> listViews,
                               KActionCollection *actionCollection,
                               Calendar *calendar);
    virtual ~KOTodoListViewQuickSearchContainer();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    KOTodoListViewQuickSearch *quickSearch() const;

  protected:
    void resizeEvent ( QResizeEvent *e );

  private:
    KOTodoListViewQuickSearch *mQuickSearch;
};

#endif
