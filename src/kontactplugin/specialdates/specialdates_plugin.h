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

#ifndef SPECIALDATES_PLUGIN_H
#define SPECIALDATES_PLUGIN_H

#include <KontactInterface/Plugin>
class KAboutData;

class SpecialdatesPlugin : public KontactInterface::Plugin
{
public:
    SpecialdatesPlugin(KontactInterface::Core *core, const QVariantList &);
    ~SpecialdatesPlugin();

    int weight() const Q_DECL_OVERRIDE
    {
        return 325;
    }

    const KAboutData aboutData() Q_DECL_OVERRIDE;

    KontactInterface::Summary *createSummaryWidget(QWidget *parentWidget) Q_DECL_OVERRIDE;

protected:
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE {
        return Q_NULLPTR;
    }

};

#endif
