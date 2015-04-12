/*
  This file is part of KDE Kontact.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORGANIZER_PLUGIN_H
#define KORGANIZER_PLUGIN_H

#include <KontactInterface/Plugin>

class OrgKdeKorganizerCalendarInterface;

namespace KontactInterface
{
class UniqueAppWatcher;
}

class KOrganizerPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    KOrganizerPlugin(KontactInterface::Core *core, const QVariantList &);
    ~KOrganizerPlugin();

    bool createDBUSInterface(const QString &serviceType) Q_DECL_OVERRIDE;
    bool isRunningStandalone() const Q_DECL_OVERRIDE;
    int weight() const Q_DECL_OVERRIDE
    {
        return 400;
    }

    bool canDecodeMimeData(const QMimeData *) const Q_DECL_OVERRIDE;
    void processDropEvent(QDropEvent *) Q_DECL_OVERRIDE;

    KontactInterface::Summary *createSummaryWidget(QWidget *parent) Q_DECL_OVERRIDE;

    QString tipFile() const Q_DECL_OVERRIDE;
    QStringList invisibleToolbarActions() const Q_DECL_OVERRIDE;

    void select() Q_DECL_OVERRIDE;

    OrgKdeKorganizerCalendarInterface *interface();

protected:
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotNewEvent();
    void slotSyncEvents();

private:
    OrgKdeKorganizerCalendarInterface *mIface;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
