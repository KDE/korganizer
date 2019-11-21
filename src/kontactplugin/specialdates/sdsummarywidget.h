/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004,2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef SDSUMMARYWIDGET_H
#define SDSUMMARYWIDGET_H

#include <KCalendarCore/Event>

#include <KontactInterface/Summary>
#include <Akonadi/Calendar/ETMCalendar>

namespace KHolidays {
class HolidayRegion;
}

namespace KontactInterface {
class Plugin;
}

class QDate;
class QGridLayout;
class QLabel;
class SDEntry;
class KJob;

class SDSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT

public:
    SDSummaryWidget(KontactInterface::Plugin *plugin, QWidget *parent);
    ~SDSummaryWidget() override;

    Q_REQUIRED_RESULT QStringList configModules() const override;
    void configUpdated();
    void updateSummary(bool force = false) override
    {
        Q_UNUSED(force);
        updateView();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void updateView();
    void popupMenu(const QString &url);
    void mailContact(const QString &url);
    void viewContact(const QString &url);
    void slotBirthdayJobFinished(KJob *job);
    void slotItemFetchJobDone(KJob *job);

    int span(const KCalendarCore::Event::Ptr &event) const;
    int dayof(const KCalendarCore::Event::Ptr &event, const QDate &date) const;
    Q_REQUIRED_RESULT bool initHolidays();
    void dateDiff(const QDate &date, int &days, int &years) const;
    void createLabels();

    Akonadi::ETMCalendar::Ptr mCalendar;

    QGridLayout *mLayout = nullptr;
    QList<QLabel *> mLabels;
    KontactInterface::Plugin *mPlugin = nullptr;

    int mDaysAhead;
    bool mShowBirthdaysFromKAB = false;
    bool mShowBirthdaysFromCal = false;
    bool mShowAnniversariesFromKAB = false;
    bool mShowAnniversariesFromCal = false;
    bool mShowHolidays = false;
    bool mShowSpecialsFromCal = false;
    bool mShowMineOnly = false;
    bool mJobRunning = false;
    QList<SDEntry> mDates;

    KHolidays::HolidayRegion *mHolidays = nullptr;
};

#endif
