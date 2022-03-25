/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KontactInterface/PimUniqueApplication>

class QUrl;

class KOrganizerApp : public KontactInterface::PimUniqueApplication
{
    Q_OBJECT
public:
    KOrganizerApp(int &argc, char **argv[]);
    ~KOrganizerApp() override;

    /**
      Create new instance of KOrganizer. If there is already running a
      KOrganizer only an additional main window is opened.
    */
    int activate(const QStringList &args, const QString &workingDir) override;

private:
    /**
      Process calendar from URL \arg url. If url is empty open the default
      calendar based on the resource framework.
    */
    void processCalendar(const QUrl &url, bool show = true);
};
