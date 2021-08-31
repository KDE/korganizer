/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "korganizerprivate_export.h"

#include <QDialog>
class QPushButton;

namespace CalendarSupport
{
class IncidenceViewer;
}

namespace Akonadi
{
class Item;
class ETMCalendar;
}

/**
  Viewer dialog for events.
*/
class KORGANIZERPRIVATE_EXPORT KOEventViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KOEventViewerDialog(Akonadi::ETMCalendar *calendar, QWidget *parent = nullptr);
    ~KOEventViewerDialog() override;

    void setIncidence(const Akonadi::Item &incidence, const QDate &date);

    void addText(const QString &text);

    QPushButton *editButton() const;

private:
    void editIncidence();
    void showIncidenceContext();
    void delayedDestruct();
    CalendarSupport::IncidenceViewer *mEventViewer = nullptr;
    QPushButton *const mUser1Button;
};

