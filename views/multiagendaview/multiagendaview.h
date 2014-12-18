/*
  This file is part of KOrganizer.

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

#ifndef KORG_VIEWS_MULTIAGENDAVIEW_H
#define KORG_VIEWS_MULTIAGENDAVIEW_H

#include "koeventview.h"

#include <calendarviews/multiagenda/configdialoginterface.h>

#include <QDialog>

#include <QAbstractItemModel>

namespace KOrg
{

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public KOEventView
{
    Q_OBJECT
public:
    explicit MultiAgendaView(QWidget *parent = Q_NULLPTR);
    ~MultiAgendaView();

    Akonadi::Item::List selectedIncidences() Q_DECL_OVERRIDE;
    KCalCore::DateList selectedIncidenceDates() Q_DECL_OVERRIDE;
    int currentDateCount() const Q_DECL_OVERRIDE;
    int maxDatesHint() const Q_DECL_OVERRIDE;

    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) Q_DECL_OVERRIDE;
    /* reimp */void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) Q_DECL_OVERRIDE;

    /**
     * reimplemented from KOrg::BaseView
     */
    bool hasConfigurationDialog() const Q_DECL_OVERRIDE;

    /**
     * reimplemented from KOrg::BaseView
     */
    void showConfigurationDialog(QWidget *parent) Q_DECL_OVERRIDE;

    void setChanges(EventViews::EventView::Changes changes) Q_DECL_OVERRIDE;

    KCheckableProxyModel *takeCustomCollectionSelectionProxyModel();
    void setCustomCollectionSelectionProxyModel(KCheckableProxyModel *model);

    void restoreConfig(const KConfigGroup &configGroup) Q_DECL_OVERRIDE;
    void saveConfig(KConfigGroup &configGroup) Q_DECL_OVERRIDE;

    void setDateRange(const KDateTime &start, const KDateTime &end,
                      const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;

    Akonadi::Collection::Id collectionId() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate());
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);
    void updateView();
    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType);
    void updateConfig();

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

private:
    class Private;
    Private *const d;

};

class MultiAgendaViewConfigDialog : public QDialog,
    public EventViews::ConfigDialogInterface
{
    Q_OBJECT
public:
    explicit MultiAgendaViewConfigDialog(QAbstractItemModel *baseModel,
                                         QWidget *parent = Q_NULLPTR);
    ~MultiAgendaViewConfigDialog();

    bool useCustomColumns() const;
    void setUseCustomColumns(bool);

    int numberOfColumns() const;
    void setNumberOfColumns(int n);

    QString columnTitle(int column) const;
    void setColumnTitle(int column, const QString &title);
    KCheckableProxyModel *takeSelectionModel(int column);
    void setSelectionModel(int column, KCheckableProxyModel *model);

public Q_SLOTS:
    /**
     * reimplemented from QDialog
     */
    void accept() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void useCustomToggled(bool);
    void numberOfColumnsChanged(int);
    void currentChanged(const QModelIndex &index);
    void titleEdited(const QString &text);

private:
    class Private;
    Private *const d;
};

}

#endif
