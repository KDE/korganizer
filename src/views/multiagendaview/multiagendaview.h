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
    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;
    void updateConfig() override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

private:
    class Private;
    std::unique_ptr<Private> const d;
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
    std::unique_ptr<Private> const d;
};
}

