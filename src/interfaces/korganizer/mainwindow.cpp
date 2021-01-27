/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"

namespace KOrg
{
MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::init(bool hasDocument)
{
    Q_UNUSED(hasDocument)
}

void MainWindow::setHasDocument(bool d)
{
    mDocument = d;
}

bool MainWindow::hasDocument() const
{
    return mDocument;
}
} // namespace KOrg
