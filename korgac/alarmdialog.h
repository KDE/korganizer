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

#include <kcal/incidence.h>

#include <kdialog.h>

#include <QTimer>
#include <QDateTime>

using namespace KCal;

class ReminderListItem;
class KOEventViewer;
class KComboBox;
class QTreeWidget;
class QSpinBox;

class AlarmDialog : public KDialog
{
  Q_OBJECT
  public:
    AlarmDialog( QWidget *parent = 0 );
    virtual ~AlarmDialog();

    void addIncidence( Incidence *incidence, const QDateTime &reminderAt );
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

  private:
    bool startKOrganizer();
    void setTimer();
    int activeCount();
    QList<ReminderListItem *> selectedItems() const;
    void updateButtons();
    void showDetails();

    QTreeWidget *mIncidenceTree;
    KOEventViewer *mDetailView;

    QSpinBox *mSuspendSpin;
    KComboBox *mSuspendUnit;
    QTimer mSuspendTimer;
};

#endif
