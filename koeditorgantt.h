/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORGANTT_H
#define _KOEDITORGANTT_H

#include <qwidget.h>
#include <qdatetime.h>

class QLabel;
class KDGanttView;
class GanttItem;
namespace KCal {
  class FreeBusy;
  class Attendee;
};


class KOEditorGantt : public QWidget
{
    Q_OBJECT
  public:
    KOEditorGantt( int spacing=8, QWidget* parent=0, const char* name=0 );
    virtual ~KOEditorGantt();

    void setUpdateEnabled( bool enabled );
    bool updateEnabled() const;

    void insertAttendee( KCal::Attendee* );
    void removeAttendee( KCal::Attendee* );
    void updateAttendee( KCal::Attendee* );
    void clearAttendees();

    void readEvent( KCal::Event* );

  signals:
    void dateTimesChanged( QDateTime, QDateTime );

  public slots:
    void slotInsertFreeBusy( const QString& email, KCal::FreeBusy* fb );

    void setDateTimes(QDateTime, QDateTime );

  protected slots:
    void slotUpdateGanttView( QDateTime, QDateTime );
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();

  private:
    void updateFreeBusyData( KCal::Attendee* );

    bool findFreeSlot( QDateTime& dtFrom, QDateTime& dtTo );
    bool tryDate( QDateTime& tryFrom, QDateTime& tryTo );
    bool tryDate( GanttItem* attendee,
                  QDateTime& tryFrom, QDateTime& tryTo );
    void updateStatusSummary();

    KDGanttView* mGanttView;
    QLabel* mOrganizerLabel;
    QLabel* mStatusSummaryLabel;
    bool mIsOrganizer;
    QComboBox* scaleCombo;

    QDateTime mDtStart, mDtEnd;
};

#endif
