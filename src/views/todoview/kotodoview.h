/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KORG_VIEWS_KOTODOVIEW_H
#define KORG_VIEWS_KOTODOVIEW_H

#include "baseview.h"

#include <CalendarSupport/Utils>

#include <EventViews/TodoView>

using namespace KOrg;

class KOTodoView : public BaseView
{
    Q_OBJECT
public:
    KOTodoView(bool sidebarView, QWidget *parent);
    ~KOTodoView() override;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &) override;

    Akonadi::Item::List selectedIncidences() override;
    KCalCore::DateList selectedIncidenceDates() override;
    int currentDateCount() const override
    {
        return 0;
    }

    void setDocumentId(const QString &)
    {
    }

    void saveLayout(KConfig *config, const QString &group) const;
    void restoreLayout(KConfig *config, const QString &group, bool minimalDefaults);

    /** documentation in baseview.h */
    void getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals) override;

    bool usesFullWindow() override;

    void saveViewState();
    void restoreViewState();

    bool supportsDateRangeSelection() override
    {
        return false;
    }

    CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;
    virtual void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;
    void updateView() override;
    void changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType changeType) override;
    void updateConfig() override;
    void clearSelection() override;

private Q_SLOTS:
    void printTodo(bool);
    void printTodo();
    void printPreviewTodo();

Q_SIGNALS:
    void purgeCompletedSignal();
    void unSubTodoSignal();
    void unAllSubTodoSignal();
    void configChanged();
    void fullViewChanged(bool enabled);

private:
    EventViews::TodoView *mView = nullptr;
};

#endif /*KOTODOVIEW_H*/
