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

#include "../../koeventview.h"

#include <calendarviews/eventviews/multiagenda/configdialoginterface.h>

#include <Akonadi/Item>

#include <QAbstractItemModel>
#include <KDialog>


namespace EventViews {
  class AgendaView;
  class TimeLabelsZone;
}

namespace KOrg {

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public KOEventView
{
  Q_OBJECT
  public:
    explicit MultiAgendaView( QWidget *parent = 0 );
    ~MultiAgendaView();

    Akonadi::Item::List selectedIncidences();
    KCalCore::DateList selectedIncidenceDates();
    int currentDateCount() const;
    int maxDatesHint() const;

    bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );
    /* reimp */void setCalendar( CalendarSupport::Calendar *cal );

    /**
     * reimplemented from KOrg::BaseView
     */
    bool hasConfigurationDialog() const;

    /**
     * reimplemented from KOrg::BaseView
     */
    void showConfigurationDialog( QWidget *parent );

    void setChanges( EventViews::EventView::Changes changes );

    Future::KCheckableProxyModel *takeCustomCollectionSelectionProxyModel();
    void setCustomCollectionSelectionProxyModel( Future::KCheckableProxyModel* model );

    void restoreConfig( const KConfigGroup &configGroup );
    void saveConfig( KConfigGroup &configGroup );

    void setDateRange( const KDateTime &start, const KDateTime &end );

  public slots:
    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );
    void updateView();
    void changeIncidenceDisplay( const Akonadi::Item &, int mode );
    void updateConfig();

    void setIncidenceChanger( CalendarSupport::IncidenceChanger *changer );

  private:
    class Private;
    Private * const d;

};

class MultiAgendaViewConfigDialog : public KDialog,
                                    public EventViews::ConfigDialogInterface
{
  Q_OBJECT
  public:
    explicit MultiAgendaViewConfigDialog( QAbstractItemModel *baseModel,
                                          QWidget *parent = 0 );
    ~MultiAgendaViewConfigDialog();

    bool useCustomColumns() const;
    void setUseCustomColumns( bool );

    int numberOfColumns() const;
    void setNumberOfColumns( int n );

    QString columnTitle( int column ) const;
    void setColumnTitle( int column, const QString &title );
    Future::KCheckableProxyModel *takeSelectionModel( int column );
    void setSelectionModel( int column, Future::KCheckableProxyModel *model );

  public Q_SLOTS:
    /**
     * reimplemented from QDialog
     */
    void accept();

  private Q_SLOTS:
    void useCustomToggled( bool );
    void numberOfColumnsChanged( int );
    void currentChanged( const QModelIndex &index );
    void titleEdited( const QString &text );

  private:
    class Private;
    Private *const d;
};

}

#endif
