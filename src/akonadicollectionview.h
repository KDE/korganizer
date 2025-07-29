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

    void setCollectionSelectionProxyModel(KCheckableProxyModel *);

    [[nodiscard]] Akonadi::Collection selectedCollection() const;
    [[nodiscard]] Akonadi::Collection::List checkedCollections() const;
    [[nodiscard]] bool isChecked(const Akonadi::Collection &) const;
    [[nodiscard]] Akonadi::Collection currentCalendar() const;

Q_SIGNALS:
    void collectionEnabled(const Akonadi::Collection &);
    void collectionDisabled(const Akonadi::Collection &);

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
    void onAction(const QModelIndex &index, int action);

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Akonadi::EntityTreeModel *entityTreeModel() const;
    // Returns the display name for the current default calendar
    [[nodiscard]] QString defaultCalendarDisplayName() const;
    // Returns true if the specified collection contains the default collection, in the non-folder case.
    // Also returns true if the specified collection is the default collection.
    [[nodiscard]] bool collectionContainsDefaultCalendar(const Akonadi::Collection &collection) const;
    // Returns true if the specified collection is the default collection.
    [[nodiscard]] bool collectionIsDefaultCalendar(const Akonadi::Collection &collection) const;
    // Returns true if the specified collection is a calendar folder, else it is a directory of collections.
    [[nodiscard]] bool isCalendarFolder(const Akonadi::Collection &collection) const;
    // Returns true if the specified collection for the mimetype can be marked as the default calendar folder.
    [[nodiscard]] bool isCollectionDefaultable(const Akonadi::Collection &collection, const QString &mimeType) const;
    // Returns a list of collections that could be marked as the default calendar folder.
    [[nodiscard]] Akonadi::Collection::List defaultableCollections(const QString &mimeType) const;
    // Prompts the user to select a new default calendar folder, or possibly sets one automatically.
    void requestDefaultCalendar(const QString &mimeType);
    // Resets the default event calendar folder.
    void resetDefaultEventCalendar();

    CalendarView *mCalendarView = nullptr;
    Akonadi::StandardCalendarActionManager *mActionManager = nullptr;
    Akonadi::EntityTreeView *mCollectionView = nullptr;
    QAbstractProxyModel *mBaseModel = nullptr;
    KCheckableProxyModel *mSelectionProxyModel = nullptr;
    QAction *mAssignColor = nullptr;
    QAction *mDefaultCalendar = nullptr;
    bool mNotSendAddRemoveSignal = false;
    bool mWasDefaultCalendar = false;
    const bool mHasContextMenu;
    NewNodeExpander *mNewNodeExpander = nullptr;
    ManageShowCollectionProperties *mManagerShowCollectionProperties = nullptr;
};
