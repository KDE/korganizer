/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "ui_searchdialog_base.h"
#include <kcalcore/incidence.h>
#include <KDialog>

namespace Akonadi {
  class Item;
}

using namespace KCalCore;

class CalendarView;
class KOListView;
class QRegExp;

class SearchDialog : public KDialog, private Ui::SearchDialog
{
  Q_OBJECT
  public:
    explicit SearchDialog( CalendarView *calendarview );
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeIncidenceDisplay( Incidence *, int ) { updateView(); }

  protected slots:
    void doSearch();
    void searchTextChanged( const QString &_text );

  signals:
    void showIncidenceSignal( const Akonadi::Item& );
    void editIncidenceSignal( const Akonadi::Item& );
    void deleteIncidenceSignal( const Akonadi::Item& );

  private:
    void search( const QRegExp & );

    CalendarView *m_calendarview;
    QList<Akonadi::Item> mMatchedEvents;
    KOListView *listView;
};

#endif
