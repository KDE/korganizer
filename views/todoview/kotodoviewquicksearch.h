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

#ifndef KOTODOVIEWQUICKSEARCH_H
#define KOTODOVIEWQUICKSEARCH_H

#include <QWidget>

namespace KOrg {
  class AkonadiCalendar;
}

namespace KPIM {
  class KCheckComboBox;
}
using namespace KPIM;

class KLineEdit;

class QString;
class QStringList;

class KOTodoViewQuickSearch : public QWidget
{
  Q_OBJECT

  public:
    KOTodoViewQuickSearch( KOrg::AkonadiCalendar *calendar, QWidget *parent );
    virtual ~KOTodoViewQuickSearch() {}

    void setCalendar( KOrg::AkonadiCalendar *calendar );
    void updateCategories();

  Q_SIGNALS:
    void searchTextChanged( const QString & );
    void searchCategoryChanged( const QStringList & );

  public Q_SLOTS:
    void reset();

  private:
    /** Helper method for the filling of the category combo. */
    void fillCategories();

    KOrg::AkonadiCalendar *mCalendar;

    KLineEdit *mSearchLine;
    KCheckComboBox *mCategoryCombo;
};

#endif
