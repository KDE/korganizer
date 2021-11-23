/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "korganizerifaceimpl.h"
#include "actionmanager.h"
#include "korganizer_debug.h"
#include "korganizeradaptor.h"

KOrganizerIfaceImpl::KOrganizerIfaceImpl(ActionManager *actionManager, QObject *parent, const QString &name)
    : QObject(parent)
    , mActionManager(actionManager)
{
    setObjectName(name);
    new KorganizerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Korganizer"), this, QDBusConnection::ExportAdaptors);
}

KOrganizerIfaceImpl::~KOrganizerIfaceImpl() = default;

bool KOrganizerIfaceImpl::openURL(const QString &url)
{
    return mActionManager->openURL(url);
}

bool KOrganizerIfaceImpl::mergeURL(const QString &url)
{
    return mActionManager->mergeURL(url);
}

bool KOrganizerIfaceImpl::saveURL()
{
    return mActionManager->saveURL();
}

bool KOrganizerIfaceImpl::saveAsURL(const QString &url)
{
    return mActionManager->saveAsURL(url);
}

QString KOrganizerIfaceImpl::getCurrentURLasString() const
{
    return mActionManager->getCurrentURLasString();
}

bool KOrganizerIfaceImpl::deleteIncidence(const QString &uid, bool force)
{
    bool ok;
    const qint64 id = QVariant(uid).toLongLong(&ok);
    if (!ok) {
        qCWarning(KORGANIZER_LOG) << "Invalid uid" << uid;
        return false;
    }
    return mActionManager->deleteIncidence(id, force);
}

bool KOrganizerIfaceImpl::editIncidence(const QString &itemId)
{
    bool ok;
    const qint64 id = QVariant(itemId).toLongLong(&ok);
    if (!ok) {
        qCWarning(KORGANIZER_LOG) << "Invalid item id = " << itemId;
        return false;
    }
    return mActionManager->editIncidence(id);
}

bool KOrganizerIfaceImpl::addIncidence(const QString &uid)
{
    return mActionManager->addIncidence(uid);
}

bool KOrganizerIfaceImpl::showIncidence(const QString &uid)
{
    bool ok;
    const qint64 id = QVariant(uid).toLongLong(&ok);
    if (!ok) {
        qCWarning(KORGANIZER_LOG) << "Invalid uid" << uid;
        return false;
    }
    return mActionManager->showIncidence(id);
}

bool KOrganizerIfaceImpl::showIncidenceContext(const QString &uid)
{
    bool ok;
    const qint64 id = QVariant(uid).toLongLong(&ok);
    if (!ok) {
        qCWarning(KORGANIZER_LOG) << "Invalid uid" << uid;
        return false;
    }
    return mActionManager->showIncidenceContext(id);
}

bool KOrganizerIfaceImpl::handleCommandLine(const QStringList &args)
{
    return mActionManager->handleCommandLine(args);
}
