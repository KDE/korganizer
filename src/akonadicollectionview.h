/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "calendarview.h"
#include "views/collectionview/reparentingmodel.h"
#include <Akonadi/Collection>

class AkonadiCollectionView;
class ManageShowCollectionProperties;

namespace Akonadi
{
class EntityTreeView;
class EntityTreeModel;
class StandardCalendarActionManager;
}

class QAction;
class KJob;
class QAbstractProxyModel;
class QModelIndex;

/**
 * The factory for AkonadiCollectionView instances.
 */
class AkonadiCollectionViewFactory : public CalendarViewExtension::Factory
{
public:
    explicit AkonadiCollectionViewFactory(CalendarView *view);

    CalendarView *view() const;
    AkonadiCollectionView *collectionView() const;

    CalendarViewExtension *create(QWidget *) override;

private:
    CalendarView *mView = nullptr;
    AkonadiCollectionView *mAkonadiCollectionView = nullptr;
};

class NewNodeExpander;

/**
 * This class provides a view of calendar resources.
 */
class AkonadiCollectionView : public CalendarViewExtension
{
    Q_OBJECT
public:
    explicit AkonadiCollectionView(CalendarView *view, bool hasContextMenu = true, QWidget *parent = nullptr);
    ~AkonadiCollectionView() override;

    Akonadi::EntityTreeView *view() const;

    KCheckableProxyModel *collectionSelectionProxyModel() const;
    void setCollectionSelectionProxyModel(KCheckableProxyModel *);

    Q_REQUIRED_RESULT Akonadi::Collection selectedCollection() const;
    Q_REQUIRED_RESULT Akonadi::Collection::List checkedCollections() const;
    Q_REQUIRED_RESULT bool isChecked(const Akonadi::Collection &) const;
    Q_REQUIRED_RESULT Akonadi::Collection currentCalendar() const;

Q_SIGNALS:
    void resourcesChanged(bool enabled);
    void resourcesAddedRemoved();
    void defaultResourceChanged(const Akonadi::Collection &);
    void colorsChanged();

private Q_SLOTS:
    void updateView();
    void updateMenu();

    void newCalendar();
    void newCalendarDone(KJob *);

    void deleteCalendar();
    void deleteCalendarDone(KJob *);
    void rowsInserted(const QModelIndex &, int, int);
    void assignColor();
    void setDefaultCalendar();
    void onSearchIsActive(bool);
    void onAction(const QModelIndex &index, int action);
    void slotServerSideSubscription();

private:
    Akonadi::EntityTreeModel *entityTreeModel() const;

    Akonadi::StandardCalendarActionManager *mActionManager = nullptr;
    Akonadi::EntityTreeView *mCollectionView = nullptr;
    QStackedWidget *mStackedWidget = nullptr;
    QAbstractProxyModel *mBaseModel = nullptr;
    KCheckableProxyModel *mSelectionProxyModel = nullptr;
    QAction *mAssignColor = nullptr;
    QAction *mDefaultCalendar = nullptr;
    QAction *mServerSideSubscription = nullptr;
    bool mNotSendAddRemoveSignal = false;
    bool mWasDefaultCalendar = false;
    const bool mHasContextMenu;
    NewNodeExpander *mNewNodeExpander = nullptr;
    ManageShowCollectionProperties *mManagerShowCollectionProperties = nullptr;
};

