/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KORG_INTERFACES_PART_H
#define KORG_INTERFACES_PART_H

#include "mainwindow.h"

#include <KPluginFactory>
#include <KParts/Part>

#include <QWidget>

namespace KOrg {
class Part : public KParts::Part
{
    Q_OBJECT
public:
    static int interfaceVersion()
    {
        return 2;
    }

    static QString serviceType()
    {
        return QStringLiteral("KOrganizer/Part");
    }

    using List = QList<Part *>;

    explicit Part(MainWindow *parent)
        : KParts::Part(parent ? (parent->topLevelWidget()) : nullptr)
        , mMainWindow(parent)
    {
    }

    virtual ~Part()
    {
    }

    virtual QString info() = 0;

    /** short name of the part, used as category in the keybindings dialog */
    virtual QString shortInfo() = 0;

    MainWindow *mainWindow()
    {
        return mMainWindow;
    }

private:
    MainWindow *mMainWindow = nullptr;
};

class PartFactory : public KPluginFactory
{
public:
    virtual Part *createPluginFactory(MainWindow *parent) = 0;

protected:
    QObject *createObject(QObject *, const char *, const QStringList &) override
    {
        return nullptr;
    }
};
}

#endif
