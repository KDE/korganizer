/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOEDITORFREEBUSY_H
#define KOEDITORFREEBUSY_H

#include <qwidget.h>
#include <qdatetime.h>
#include <qtimer.h>

class QLabel;
class KDGanttView;
class KDGanttViewItem;
class FreeBusyItem;

namespace KCal {
  class FreeBusy;
  class Attendee;
};


class KOEditorFreeBusy : public QWidget
{
    Q_OBJECT
  public:
    KOEditorFreeBusy( int spacing = 8, QWidget *parent = 0,
                      const char *name = 0 );
    virtual ~KOEditorFreeBusy();

    void setUpdateEnabled( bool enabled );
    bool updateEnabled() const;

    void insertAttendee( KCal::Attendee * );
    void removeAttendee( KCal::Attendee * );
    void updateAttendee( KCal::Attendee * );
    void clearAttendees();

    void readEvent( KCal::Event * );

    void triggerReload();
    void cancelReload();

  signals:
    void dateTimesChanged( QDateTime, QDateTime );

  public slots:
    void slotInsertFreeBusy( KCal::FreeBusy *fb, const QString &email );

    void setDateTimes( QDateTime, QDateTime );

    void editFreeBusyUrl( KDGanttViewItem *item );

  protected slots:
    void slotUpdateGanttView( QDateTime, QDateTime );
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();

    void reload();

  private:
    void updateFreeBusyData( KCal::Attendee * );

    bool findFreeSlot( QDateTime &dtFrom, QDateTime &dtTo );
    bool tryDate( QDateTime &tryFrom, QDateTime &tryTo );
    bool tryDate( FreeBusyItem *attendee,
                  QDateTime &tryFrom, QDateTime &tryTo );
    void updateStatusSummary();

    KDGanttView *mGanttView;
    QLabel *mStatusSummaryLabel;
    bool mIsOrganizer;
    QComboBox *scaleCombo;

    QDateTime mDtStart, mDtEnd;

    QTimer mReloadTimer;
};

#endif
