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

#ifndef KORG_KOEVENTPOPUPMENU_H
#define KORG_KOEVENTPOPUPMENU_H

#include <QDate>
#include <QMenu>

#include <Akonadi/Item>
#include <Akonadi/Calendar/ETMCalendar>

/**
 * Context menu for event views with standard event actions.
*/
class KOEventPopupMenu : public QMenu
{
  Q_OBJECT
  public:
    explicit KOEventPopupMenu( Akonadi::ETMCalendar * , QWidget *parent = 0 );
    void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar );

  public slots:
    void showIncidencePopup( const Akonadi::Item &, const QDate & );

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
    void editIncidenceSignal( const Akonadi::Item & );
    void showIncidenceSignal( const Akonadi::Item & );
    void deleteIncidenceSignal( const Akonadi::Item & );
    void cutIncidenceSignal( const Akonadi::Item & );
    void copyIncidenceSignal( const Akonadi::Item & );
    void pasteIncidenceSignal();
    void toggleAlarmSignal( const Akonadi::Item & );
    void toggleTodoCompletedSignal( const Akonadi::Item & );
    void copyIncidenceToResourceSignal( const Akonadi::Item &, const QString & );
    void moveIncidenceToResourceSignal( const Akonadi::Item &, const QString & );
    void dissociateOccurrencesSignal( const Akonadi::Item &, const QDate & );

  private:
    void print( bool previous );

    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::Item mCurrentIncidence;
    QDate mCurrentDate;

    bool mHasAdditionalItems;
    QList<QAction *> mEditOnlyItems;
    QList<QAction *> mTodoOnlyItems;
    QList<QAction *> mRecurrenceItems;
    QAction *mDissociateOccurrences;
    QAction *mToggleReminder;
};

#endif
