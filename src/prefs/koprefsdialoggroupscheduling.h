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

#ifndef KOPREFSDIALOGGROUPSCHEDULING_H
#define KOPREFSDIALOGGROUPSCHEDULING_H
#include "kcm_korganizer_export.h"

#include <KCModule>
class QCheckBox;
class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupScheduling : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogGroupScheduling(QWidget *parent);

    void save() override;
    void load() override;

private:
    void slotConfigChanged();
    QCheckBox *mUseGroupwareCommunicationCheckBox = nullptr;
    QCheckBox *mBccBox = nullptr;
};


#endif // KOPREFSDIALOGGROUPSCHEDULING_H
