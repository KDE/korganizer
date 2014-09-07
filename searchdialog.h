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

#ifndef KORG_SEARCHDIALOG_H
#define KORG_SEARCHDIALOG_H

#include <QDialog>
class QPushButton;
class CalendarView;

namespace Ui {
  class SearchDialog;
}

namespace EventViews {
  class ListView;
}

namespace Akonadi {
  class Item;
}

namespace KCalCore {
  class Incidence;
}

class SearchDialog : public QDialog
{
  Q_OBJECT

  public:
    explicit SearchDialog( CalendarView *calendarview );
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeIncidenceDisplay( KCalCore::Incidence *, int )
    {
      updateView();
    }

  protected slots:
    void doSearch();
    void searchTextChanged( const QString &_text );

  signals:
    void showIncidenceSignal( const Akonadi::Item & );
    void editIncidenceSignal( const Akonadi::Item & );
    void deleteIncidenceSignal( const Akonadi::Item & );

  protected:
    /*reimp*/
   virtual void showEvent( QShowEvent *event );
  private:
    void search( const QRegExp & );
    void readConfig();
    void writeConfig();

    Ui::SearchDialog *m_ui;
    CalendarView *m_calendarview; // parent
    QList<Akonadi::Item> mMatchedEvents;
    EventViews::ListView *listView;
    QPushButton *mUser1Button;
};

#endif
