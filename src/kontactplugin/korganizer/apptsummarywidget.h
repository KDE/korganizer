/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <Akonadi/Calendar/ETMCalendar>
#include <KontactInterface/Summary>

class KOrganizerPlugin;

namespace Akonadi
{
class Item;
class IncidenceChanger;
}

class QGridLayout;
class QLabel;

class ApptSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT

public:
    ApptSummaryWidget(KOrganizerPlugin *plugin, QWidget *parent);
    ~ApptSummaryWidget() override;

    int summaryHeight() const override
    {
        return 3;
    }

    void configUpdated();
    void updateSummary(bool force = false) override
    {
        Q_UNUSED(force)
        updateView();
    }

protected:
    Q_REQUIRED_RESULT bool eventFilter(QObject *obj, QEvent *e) override;

private Q_SLOTS:
    void updateView();
    void popupMenu(const QString &uid);
    void viewEvent(const QString &uid);
    void removeEvent(const Akonadi::Item &item);

private:
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::IncidenceChanger *mChanger = nullptr;

    QGridLayout *mLayout = nullptr;
    QList<QLabel *> mLabels;
    KOrganizerPlugin *mPlugin = nullptr;
    int mDaysAhead;
    bool mShowBirthdaysFromCal = false;
    bool mShowAnniversariesFromCal = false;
    bool mShowMineOnly = false;
};

