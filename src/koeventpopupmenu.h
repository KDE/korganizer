/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QDate>
#include <QMenu>

#include <Akonadi/CollectionCalendar>
#include <Akonadi/Item>

/**
 * Context menu with standard Incidence actions.
 */
class KOEventPopupMenu : public QMenu
{
    Q_OBJECT
public:
    /**
     * The different types of preset menus available.
     */
    enum MenuStyle {
        NormalView = 0, /**< typical menu shown in most views. this is the default menu style */
        MiniList = 1, /**< mini-menu with just the basic items, typical in lists */
    };
    Q_ENUM(MenuStyle)

    /**
     * Create an Incidence menu instance with the NormalView style.
     */
    explicit KOEventPopupMenu(QWidget *parent = nullptr);

    /**
     * Create an Incidence menu instance with a specified style.
     */
    explicit KOEventPopupMenu(MenuStyle menuStyle, QWidget *parent = nullptr);

public Q_SLOTS:
    /**
     * Display a pop-up menu that operates on the incidence contained in the
     * given item, and its occurrence displayed on the given date.
     */
    void showIncidencePopup(const Akonadi::CollectionCalendar::Ptr &calendar, const Akonadi::Item &, const QDate &);

    void createEvent(const Akonadi::Item &item);

protected Q_SLOTS:
    void popupShow();
    void popupEdit();
    void popupPaste();
    void slotPrint();
    void printPreview();
    void popupDelete();
    void popupCut();
    void popupCopy();
    void toggleAlarm();
    void toggleTodoCompleted();
    void dissociateOccurrences();
    void forward();

    /**
     * Start the incidenceeditor to create a new To-do using a few settings from the currently selected incidence
     * where the current incidence can be a to-do or event.
     */
    void createNewTodo();

    /**
     * Start the incidenceeditor to create a new To-do using a few settings from the currently selected incidence
     * 1. if current incidence is a to-do, then reuse the start, all-day and due
     * 2. if current incidence is an event, then reuse almost settings (i.e morph event into to-do)
     */
    void createTodo();

    /**
     * Start the incidenceeditor to create a new Event using a few settings from the currently selected incidence
     * where the current incidence can be a to-do or event.
     */
    void createNewEvent();

    /**
     * Start the incidenceeditor to create a new Event using a few settings from the currently selected incidence
     * 1. if current incidence is an event, then reuse the start, all-day and end
     * 2. if current incidence is a to-do, then reuse almost settings (i.e morph to-do into event)
     */
    void createEvent();

Q_SIGNALS:
    void configChanged();
    void editIncidenceSignal(const Akonadi::Item &);
    void showIncidenceSignal(const Akonadi::Item &);
    void showOccurrenceSignal(const Akonadi::Item &, const QDate &);
    void deleteIncidenceSignal(const Akonadi::Item &);
    void cutIncidenceSignal(const Akonadi::Item &);
    void copyIncidenceSignal(const Akonadi::Item &);
    void pasteIncidenceSignal();
    void toggleAlarmSignal(const Akonadi::Item &);
    void toggleOccurrenceCompletedSignal(const Akonadi::Item &, const QDate &);
    void copyIncidenceToResourceSignal(const Akonadi::Item &, const Akonadi::Collection &);
    void moveIncidenceToResourceSignal(const Akonadi::Item &, const Akonadi::Collection &);
    void dissociateOccurrencesSignal(const Akonadi::Item &, const QDate &);

private:
    void init(MenuStyle menuStyle);
    void appendEditOnlyItems();
    void appendEventOnlyItems();
    void appendTodoOnlyItems();
    void appendReminderOnlyItems();
    void appendRecurrenceOnlyItems();
    void appendShareOnlyItems();

    void print(bool preview);

    Akonadi::CollectionCalendar::Ptr mCurrentCalendar;
    Akonadi::Item mCurrentIncidence;
    QDate mCurrentDate;

    QList<QAction *> mEditOnlyItems;
    QList<QAction *> mEventOnlyItems;
    QList<QAction *> mTodoOnlyItems;
    QList<QAction *> mReminderOnlyItems;
    QList<QAction *> mRecurrenceOnlyItems;
    QList<QAction *> mShareOnlyItems;
    QAction *mToggleReminder = nullptr;
    QAction *mDissociateOccurrences = nullptr;
};
