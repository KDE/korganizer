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
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_VIEWS_KOTODOVIEWQUICKSEARCH_H
#define KORG_VIEWS_KOTODOVIEWQUICKSEARCH_H

#include <akonadi/calendar/etmcalendar.h>
#include <QWidget>

namespace KPIM {
  class KCheckComboBox;
}

class KLineEdit;

class KOTodoViewQuickSearch : public QWidget
{
  Q_OBJECT
  public:
    KOTodoViewQuickSearch( const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent );
    virtual ~KOTodoViewQuickSearch() {}

    void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar );
    void updateCategories();

  Q_SIGNALS:
    void searchTextChanged( const QString & );

    /**
     * The string list contains the new categories which are set on the filter.
     * All values belong to the Qt::UserRole of the combo box, not the Qt::DisplayRole,
     * so, if someone checks a subcategory, the value will be "ParentCategory:subCategory"
     * and not " subcategory".
     * */
    void filterCategoryChanged( const QStringList & );
    void filterPriorityChanged( const QStringList & );

  public Q_SLOTS:
    void reset();

  private Q_SLOTS:
    void emitFilterCategoryChanged();
    void emitFilterPriorityChanged();

  private:
    /** Helper method for the filling of the category combo. */
    void fillCategories();
    /** Helper method for the filling of the priority combo. */
    void fillPriorities();

    Akonadi::ETMCalendar::Ptr mCalendar;

    KLineEdit *mSearchLine;
    KPIM::KCheckComboBox *mCategoryCombo;
    KPIM::KCheckComboBox *mPriorityCombo;
};

#endif
