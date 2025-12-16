/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "koeventview.h"

#include <EventViews/ConfigDialogInterface>

#include <QDialog>

#include <QAbstractItemModel>

#include <memory>

namespace KOrg
{
class CalendarViewBase;
class MultiAgendaViewPrivate;

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public KOEventView
{
    Q_OBJECT
public:
    explicit MultiAgendaView(CalendarViewBase *calendarView, QWidget *parent = nullptr);
    ~MultiAgendaView() override;

    void setModel(QAbstractItemModel *model) override;

    [[nodiscard]] Akonadi::Item::List selectedIncidences() override;
    [[nodiscard]] KCalendarCore::DateList selectedIncidenceDates() override;
    [[nodiscard]] int currentDateCount() const override;
    [[nodiscard]] int maxDatesHint() const override;

    [[nodiscard]] bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    void setCollectionSelectionProxyModel(KCheckableProxyModel *model);

    /**
     * reimplemented from KOrg::BaseView
     */
    [[nodiscard]] bool hasConfigurationDialog() const override;

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

    [[nodiscard]] Akonadi::Collection::Id collectionId() const override;

    [[nodiscard]] bool showSideBar() override;
    void setShowSideBar(bool show) override;

public Q_SLOTS:
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;
    void updateView() override;
    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;
    void updateConfig() override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

private:
    std::unique_ptr<MultiAgendaViewPrivate> const d;
};

class MultiAgendaViewConfigDialogPrivate;

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
    std::unique_ptr<MultiAgendaViewConfigDialogPrivate> const d;
};
}
