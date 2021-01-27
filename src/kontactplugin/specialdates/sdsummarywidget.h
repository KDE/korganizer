/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef SDSUMMARYWIDGET_H
#define SDSUMMARYWIDGET_H

#include <KCalendarCore/Event>

#include <Akonadi/Calendar/ETMCalendar>
#include <KontactInterface/Summary>

namespace KHolidays
{
class HolidayRegion;
}

namespace KontactInterface
{
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
        Q_UNUSED(force)
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
    KontactInterface::Plugin *const mPlugin;

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
