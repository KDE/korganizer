/*
  This file is part of the KDE reminder daemon.

  Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <KDialog>

#include <QPoint>
#include <QTimer>

namespace KCal {
  class Calendar;
  class Incidence;
}

namespace Akonadi {
  class Calendar;
  class IncidenceViewer;
  class Item;
}

class ReminderListItem;

class KComboBox;

class QDateTime;
class QTreeWidget;
class QTreeWidgetItem;
class QSpinBox;
class QVBoxLayout;

class AlarmDialog : public KDialog
{

  Q_OBJECT
  public:
    explicit AlarmDialog( Akonadi::Calendar *calendar, QWidget *parent = 0 );
    virtual ~AlarmDialog();

    void addIncidence( const Akonadi::Item &incidence, const QDateTime &reminderAt,
                       const QString &displayText );
    void setRemindAt( const QDateTime &dt );
    void eventNotification();

  public slots:
    void slotOk();    // suspend
    void slotUser1(); // edit
    void slotUser2(); // dismiss all
    void slotUser3(); // dismiss selected
    void slotSave();
    void wakeUp();
    void show();
    void edit();
    void suspend();
    void suspendAll();
    void dismissAll();
    void dismissCurrent();
    /*reimp*/
    void accept();

  signals:
    void reminderCount( int count );

  private Q_SLOTS:
    void update();
    void toggleDetails( QTreeWidgetItem *item, int column );

  private:
    bool startKOrganizer();
    ReminderListItem *searchByItem( const Akonadi::Item &incidence );
    void setTimer();
    void dismiss( QList<ReminderListItem *> selections );
    int activeCount();
    QList<ReminderListItem *> selectedItems() const;
    void updateButtons();
    void showDetails();

    Akonadi::Calendar *mCalendar;
    QVBoxLayout *mTopLayout;
    QTreeWidget *mIncidenceTree;
    Akonadi::IncidenceViewer *mDetailView;

    QPoint mPos;
    QSpinBox *mSuspendSpin;
    KComboBox *mSuspendUnit;
    QTimer mSuspendTimer;
    QTreeWidgetItem *mLastItem;
};

#endif
