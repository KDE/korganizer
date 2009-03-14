/*
  This file is part of KOrganizer.

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
#ifndef KOEVENTPOPUPMENU_H
#define KOEVENTPOPUPMENU_H
//
// Context menu for event views with standard event actions
//

#include <QMenu>
#include <QDateTime>

namespace KCal {
  class Calendar;
  class Incidence;
}
using namespace KCal;

class KOEventPopupMenu : public QMenu
{
  Q_OBJECT
  public:
    KOEventPopupMenu();

  public slots:
    void showIncidencePopup( Calendar *, Incidence *, const QDate & );

  protected slots:
    void popupShow();
    void popupEdit();
    void popupPaste();
    void print();
    void printPreview();
    void popupDelete();
    void popupCut();
    void popupCopy();
    void toggleAlarm();
    void toggleTodoCompleted();
    void dissociateOccurrences();
    void forward();

  signals:
    void configChanged();
    void editIncidenceSignal( Incidence * );
    void showIncidenceSignal( Incidence * );
    void deleteIncidenceSignal( Incidence * );
    void cutIncidenceSignal( Incidence * );
    void copyIncidenceSignal( Incidence * );
    void pasteIncidenceSignal();
    void toggleAlarmSignal( Incidence * );
    void toggleTodoCompletedSignal( Incidence * );
    void dissociateOccurrencesSignal( Incidence *, const QDate & );

  private:
    Calendar *mCalendar;
    Incidence *mCurrentIncidence;
    QDate mCurrentDate;

    bool mHasAdditionalItems;
    QList<QAction *> mEditOnlyItems;
    QList<QAction *> mTodoOnlyItems;
    QList<QAction *> mRecurrenceItems;
    QAction *mDissociateOccurrences;
};

#endif
