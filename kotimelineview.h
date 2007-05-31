/*
    This file is part of KOrganizer.

    Copyright (c) 2007 Till Adam <adam@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOTIMELINEVIEW_H
#define KOTIMELINEVIEW_H

#include <koeventview.h>

#include <qmap.h>

class KDGanttView;
class KDGanttViewItem;

namespace KCal {
  class ResourceCalendar;
}

namespace KOrg {
  class TimelineItem;
}

/**
  This class provides a view ....
*/
class KOTimelineView : public KOEventView
{
    Q_OBJECT
  public:
    KOTimelineView(Calendar *calendar, QWidget *parent = 0,
                    const char *name = 0);
    ~KOTimelineView();

    virtual KCal::ListBase<KCal::Incidence> selectedIncidences();
    virtual KCal::DateList selectedDates();
    virtual int currentDateCount();
    virtual void showDates(const QDate&, const QDate&);
    virtual void showIncidences(const KCal::ListBase<KCal::Incidence>&);
    virtual void updateView();
    virtual void changeIncidenceDisplay(KCal::Incidence*, int);
    virtual int maxDatesHint() { return 0; }

    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

  private slots:
    void itemSelected( KDGanttViewItem *item );
    void itemDoubleClicked( KDGanttViewItem *item );
    void itemRightClicked( KDGanttViewItem *item );

  private:
    KDGanttView* mGantt;
    QMap<KCal::ResourceCalendar*, QMap<QString, KOrg::TimelineItem*> > mCalendarItemMap;
    KOEventPopupMenu *mEventPopup;
    QDateTime mRmbDate;
};

#endif
