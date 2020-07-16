/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KOPREFSDESIGNERFIELDS_H
#define KOPREFSDESIGNERFIELDS_H

#include "kcmdesignerfields.h"
#include "kcm_korganizer_export.h"

class KCM_KORGANIZER_EXPORT KOPrefsDesignerFields : public KCMDesignerFields
{
public:
    explicit KOPrefsDesignerFields(QWidget *parent = nullptr);

protected:
    Q_REQUIRED_RESULT QString localUiDir() override;
    Q_REQUIRED_RESULT QString uiPath() override;
    void writeActivePages(const QStringList &) override;
    Q_REQUIRED_RESULT QStringList readActivePages() override;
    Q_REQUIRED_RESULT QString applicationName() override;
};


#endif // KOPREFSDESIGNERFIELDS_H
