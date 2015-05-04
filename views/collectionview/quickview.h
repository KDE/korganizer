/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef KORG_QUICKVIEW_H
#define KORG_QUICKVIEW_H

#include "controller.h"

#include <calendarviews/agenda/viewcalendar.h>

#include <KCalCore/FreeBusy>
#include <QDialog>

#include <QStringList>

class  Ui_quickview;

namespace EventViews
{
class AgendaView;
}

class Quickview : public QDialog
{
    Q_OBJECT
public:
    Quickview(const Person &person, const Akonadi::Collection &col);
    virtual ~Quickview();

private Q_SLOTS:
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
