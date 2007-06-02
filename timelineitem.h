/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KORG_TIMELINEITEM_H
#define KORG_TIMELINEITEM_H

#define private protected
#include <kdgantt/KDGanttViewTaskItem.h>
#undef protected

#include <qmap.h>
#include <qvaluelist.h>

class KDGanttView;
class KDCanvasPolygon;

namespace KCal {
  class Calendar;
  class ResourceCalendar;
  class Incidence;
}

namespace KOrg {

class TimelineSubItem;

class TimelineItem : public KDGanttViewTaskItem
{
  public:
    TimelineItem( const QString &label, KDGanttView* parent );

    void insertIncidence( KCal::Incidence *incidence,
                          const QDateTime &start = QDateTime(),
                          const QDateTime &end = QDateTime() );
    void removeIncidence( KCal::Incidence *incidence );

    void moveItems( KCal::Incidence* incidence, int delta, int duration );

  private:
    QMap<KCal::Incidence*, QValueList<TimelineSubItem*> > mItemMap;
};

class TimelineSubItem : public KDGanttViewTaskItem
{
  public:
    TimelineSubItem( KCal::Incidence *incidence, TimelineItem *parent );
    ~TimelineSubItem();

    KCal::Incidence* incidence() const { return mIncidence; }

    QDateTime originalStart() const { return mStart; }
    void setOriginalStart( const QDateTime &dt ) { mStart = dt; }

  private:
    void showItem( bool show = true, int coordY = 0 );

  private:
    KCal::Incidence *mIncidence;
    QDateTime mStart;
    KDCanvasPolygon *mLeft, *mRight;
    int mMarkerWidth;
};

}

#endif
