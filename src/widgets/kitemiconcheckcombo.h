/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Libkdepim/KCheckComboBox>

#include <EventViews/EventView>

class KItemIconCheckCombo : public KPIM::KCheckComboBox
{
    Q_OBJECT
public:
    enum ViewType { AgendaType = 0, MonthType };

    explicit KItemIconCheckCombo(ViewType viewType, QWidget *parent = nullptr);
    ~KItemIconCheckCombo() override;

    void setCheckedIcons(const QSet<EventViews::EventView::ItemIcon> &icons);
    Q_REQUIRED_RESULT QSet<EventViews::EventView::ItemIcon> checkedIcons() const;

private:
    const KItemIconCheckCombo::ViewType mViewType;
};

