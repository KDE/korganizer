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
    KOEventPopupMenu(MenuStyle menuStyle, QWidget *parent = nullptr);

public Q_SLOTS:
    /**
     * Display a pop-up menu that operates on the incidence contained in the
     * given item, and its occurrence displayed on the given date.
     */
    void showIncidencePopup(const Akonadi::CollectionCalendar::Ptr &calendar, const Akonadi::Item &, const QDate &);

    void createEvent(const Akonadi::Item &item);
    void createNote(const Akonadi::Item &item);

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
    void createTodo();
    void createEvent();
    void createNote();

    void slotCreateNote(const Akonadi::Item &noteItem, const Akonadi::Collection &collection);
    void slotCreateNewNoteJobFinished(KJob *);

Q_SIGNALS:
    void configChanged();
    void editIncidenceSignal(const Akonadi::Item &);
    void showIncidenceSignal(const Akonadi::Item &);
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

    void print(bool previous);

    Akonadi::CollectionCalendar::Ptr mCurrentCalendar;
    Akonadi::Item mCurrentIncidence;
    QDate mCurrentDate;

    bool mHasAdditionalItems = false;
    QList<QAction *> mEditOnlyItems;
    QList<QAction *> mEventOnlyItems;
    QList<QAction *> mTodoOnlyItems;
    QList<QAction *> mReminderOnlyItems;
    QList<QAction *> mRecurrenceOnlyItems;
    QList<QAction *> mShareOnlyItems;
    QAction *mToggleReminder = nullptr;
    QAction *mDissociateOccurrences = nullptr;
};
