/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <AkonadiCore/Item>

#include <QDialog>

class QPushButton;
class CalendarView;
class KOEventPopupMenu;

namespace Ui
{
class SearchDialog;
}

namespace EventViews
{
class ListView;
}

namespace KCalendarCore
{
class Incidence;
}

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchDialog(CalendarView *calendarview);
    ~SearchDialog() override;

    void updateView();

private Q_SLOTS:
    void popupMenu(const QPoint &point);
    void slotDeleteSelection();
    void slotEditSelection();
    void slotHelpRequested();

Q_SIGNALS:
    void showIncidenceSignal(const Akonadi::Item &);
    void editIncidenceSignal(const Akonadi::Item &);
    void deleteIncidenceSignal(const Akonadi::Item &);

protected:
    /*reimp*/
    void showEvent(QShowEvent *event) override;

private:
    void doSearch();
    void searchPatternChanged(const QString &pattern);
    void search(const QRegExp &re);
    void readConfig();
    void writeConfig();
    void updateMatchesText();

    Ui::SearchDialog *const m_ui;
    CalendarView *const m_calendarview; // parent
    KOEventPopupMenu *m_popupMenu = nullptr;
    Akonadi::Item::List mMatchedEvents;
    EventViews::ListView *listView = nullptr;
    QPushButton *mUser1Button = nullptr;
};

