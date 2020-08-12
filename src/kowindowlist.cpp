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
    , mDefaultWindow(nullptr)
{
}

KOWindowList::~KOWindowList()
{
}

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

bool KOWindowList::lastInstance()
{
    const int countWindow = mWindowList.count();
    if (countWindow == 1 && !mDefaultWindow) {
        return true;
    }

    if (countWindow == 0 && mDefaultWindow) {
        return true;
    } else {
        return false;
    }
}

KOrg::MainWindow *KOWindowList::findInstance(const QUrl &url)
{
    for (KOrg::MainWindow *inst : qAsConst(mWindowList)) {
        if (inst && inst->getCurrentURL() == url) {
            return inst;
        }
    }
    return nullptr;
}

KOrg::MainWindow *KOWindowList::defaultInstance()
{
    return mDefaultWindow;
}
