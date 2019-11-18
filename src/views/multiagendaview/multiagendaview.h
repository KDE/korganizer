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

#include <EventViews/ConfigDialogInterface>

#include <QDialog>

#include <QAbstractItemModel>

namespace KOrg {
/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public KOEventView
{
    Q_OBJECT
public:
    explicit MultiAgendaView(QWidget *parent = nullptr);
    ~MultiAgendaView() override;

    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;
    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override;
    Q_REQUIRED_RESULT int currentDateCount() const override;
    Q_REQUIRED_RESULT int maxDatesHint() const override;

    Q_REQUIRED_RESULT bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;
    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;

    /**
     * reimplemented from KOrg::BaseView
     */
    Q_REQUIRED_RESULT bool hasConfigurationDialog() const override;

    /**
     * reimplemented from KOrg::BaseView
     */
    void showConfigurationDialog(QWidget *parent) override;

    void setChanges(EventViews::EventView::Changes changes) override;

    KCheckableProxyModel *takeCustomCollectionSelectionProxyModel();
    void setCustomCollectionSelectionProxyModel(KCheckableProxyModel *model);

    void restoreConfig(const KConfigGroup &configGroup) override;
    void saveConfig(KConfigGroup &configGroup) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

    Q_REQUIRED_RESULT Akonadi::Collection::Id collectionId() const override;

public Q_SLOTS:
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;
    void updateView() override;
    void changeIncidenceDisplay(const Akonadi::Item &,
                                Akonadi::IncidenceChanger::ChangeType) override;
    void updateConfig() override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

private:
    class Private;
    Private *const d;
};

class MultiAgendaViewConfigDialog : public QDialog, public EventViews::ConfigDialogInterface
{
    Q_OBJECT
public:
    explicit MultiAgendaViewConfigDialog(QAbstractItemModel *baseModel, QWidget *parent = nullptr);
    ~MultiAgendaViewConfigDialog() override;

    bool useCustomColumns() const override;
    void setUseCustomColumns(bool);

    int numberOfColumns() const override;
    void setNumberOfColumns(int n);

    QString columnTitle(int column) const override;
    void setColumnTitle(int column, const QString &title);
    KCheckableProxyModel *takeSelectionModel(int column) override;
    void setSelectionModel(int column, KCheckableProxyModel *model);

public Q_SLOTS:
    /**
     * reimplemented from QDialog
     */
    void accept() override;

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
