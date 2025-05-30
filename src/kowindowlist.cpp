/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kowindowlist.h"
#include "mainwindow.h"

#include <QUrl>

KOWindowList::KOWindowList()
    : QObject(nullptr)
{
}

KOWindowList::~KOWindowList() = default;

void KOWindowList::addWindow(KOrg::MainWindow *korg)
{
    if (!korg->hasDocument()) {
        mDefaultWindow = korg;
    } else {
        mWindowList.append(korg);
    }
}

void KOWindowList::removeWindow(KOrg::MainWindow *korg)
{
    if (korg == mDefaultWindow) {
        mDefaultWindow = nullptr;
    } else {
        mWindowList.removeAll(korg);
    }
}

KOrg::MainWindow *KOWindowList::findInstance(const QUrl &url)
{
    const auto it = std::ranges::find_if(mWindowList, [url](const KOrg::MainWindow *inst) {
        return (inst && inst->getCurrentURL() == url);
    });
    return (it == mWindowList.end()) ? nullptr : (*it);
}

KOrg::MainWindow *KOWindowList::defaultInstance()
{
    return mDefaultWindow;
}

#include "moc_kowindowlist.cpp"
