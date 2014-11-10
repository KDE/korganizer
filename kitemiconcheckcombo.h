/*
  This file is part of KOrganizer.

  Copyright (C) 2011 Sérgio Martins <iamsergio@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef KORG_KITEMICONCHECKCOMBO_H
#define KORG_KITEMICONCHECKCOMBO_H

#include <libkdepim/widgets/kcheckcombobox.h>

#include <calendarviews/eventview.h>

class KItemIconCheckCombo : public KPIM::KCheckComboBox
{
    Q_OBJECT
public:
    enum ViewType {
        AgendaType = 0,
        MonthType
    };

    explicit KItemIconCheckCombo(ViewType viewType, QWidget *parent = 0);
    ~KItemIconCheckCombo();

    void setCheckedIcons(const QSet<EventViews::EventView::ItemIcon> &icons);
    QSet<EventViews::EventView::ItemIcon> checkedIcons() const;

private:
    class Private;
    Private *const d;
};

#endif // KITEMICONCHECKCOMBO_H
