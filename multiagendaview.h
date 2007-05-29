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

#ifndef KORG_MULTIAGENDAVIEW_H_H
#define KORG_MULTIAGENDAVIEW_H_H

#include "agendaview.h"

class QBoxLayout;
class KOAgendaView;

namespace KOrg {

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public AgendaView
{
  Q_OBJECT
  public:
    explicit MultiAgendaView( Calendar* cal, QWidget *parent = 0, const char *name = 0 );
    ~MultiAgendaView();

    Incidence::List selectedIncidences();
    DateList selectedDates();
    int currentDateCount();
    int maxDatesHint();

    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

    void setTypeAheadReceiver( QObject *o );

  public slots:
    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Incidence::List &incidenceList );
    void updateView();
    void changeIncidenceDisplay( Incidence *incidence, int mode );

    void finishTypeAhead();

  private:
    void deleteViews();
    void recreateViews();
    void setupViews();

  private slots:
    void slotSelectionChanged();
    void slotClearTimeSpanSelection();

  private:
    QValueList<KOAgendaView*> mAgendaViews;
    QValueList<QWidget*> mAgendaWidgets;
    QBoxLayout *mTopLevelLayout;
};

}

#endif
