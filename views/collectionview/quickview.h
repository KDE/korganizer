/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KORG_QUICKVIEW_H
#define KORG_QUICKVIEW_H

#include "controller.h"

#include <calendarviews/agenda/viewcalendar.h>

#include <KCalCore/FreeBusy>
#include <KDialog>

#include <QStringList>
#include <QStringListModel>

class  Ui_quickview;

namespace EventViews
{
    class AgendaView;
}

class Quickview : public KDialog
{
    Q_OBJECT
public:
    Quickview(const Person &person, const Akonadi::Collection &col);
    virtual ~Quickview();

private slots:
    void onTodayClicked();
    void onNextClicked();
    void onPreviousClicked();

private:
    Ui_quickview *mUi;
    EventViews::AgendaView *mAgendaView;
    Person mPerson;
    Akonadi::Collection mCollection;
    int mDayRange;
};

#endif // QUICKVIEW_H
