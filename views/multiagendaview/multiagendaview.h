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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KORG_MULTIAGENDAVIEW_H_H
#define KORG_MULTIAGENDAVIEW_H_H

#include "views/agendaview/agendaview.h"

class KOAgendaView;
class TimeLabelsZone;

class KHBox;

class Q3ScrollView;
class QResizeEvent;
class QScrollBar;
class QSplitter;

namespace KCal {
  class ResourceCalendar;
}

namespace KOrg {

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public AgendaView
{
  Q_OBJECT
  public:
    explicit MultiAgendaView( Calendar *cal, QWidget *parent = 0 );
    ~MultiAgendaView();

    Incidence::List selectedIncidences();
    DateList selectedDates();
    int currentDateCount();
    int maxDatesHint();

    bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );


  public slots:
    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Incidence::List &incidenceList );
    void updateView();
    void changeIncidenceDisplay( Incidence *incidence, int mode );
    void updateConfig();

    void setIncidenceChanger( IncidenceChangerBase *changer );

    void setUpdateNeeded();

  protected:
    void resizeEvent( QResizeEvent *ev );
    void showEvent( QShowEvent *event );

  private:
    void addView( const QString &label, KCal::ResourceCalendar *res,
                  const QString &subRes = QString() );
    void deleteViews();
    void recreateViews();
    void setupViews();
    void resizeScrollView( const QSize &size );

  private slots:
    void slotSelectionChanged();
    void slotClearTimeSpanSelection();
    void resizeSplitters();
    void setupScrollBar();
    void zoomView( const int delta, const QPoint &pos, const Qt::Orientation ori );
    void slotResizeScrollView();

  private:
    QList<KOAgendaView*> mAgendaViews;
    QList<QWidget*> mAgendaWidgets;
    KHBox *mTopBox;
    Q3ScrollView *mScrollView;
    TimeLabelsZone *mTimeLabelsZone;
    QSplitter *mLeftSplitter, *mRightSplitter;
    QScrollBar *mScrollBar;
    QWidget *mLeftBottomSpacer, *mRightBottomSpacer;
    QDate mStartDate, mEndDate;
    bool mUpdateOnShow;
    bool mPendingChanges;
};

}

#endif
