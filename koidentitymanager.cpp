/*
    This file is part of KOrganizer.

    Copyright (c) 2004 David Faure <faure@kde.org>

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

#include "koidentitymanager.h"
#include "koprefs.h"

// This is called to create a default identity in case emailidentities has none
// (i.e. the user never used KMail before)
// We provide the values from KOPrefs, since those are configurable in korganizer.
void KOrg::IdentityManager::createDefaultIdentity( QString& fullName, QString& emailAddress )
{
  fullName = KOPrefs::instance()->fullName();
  emailAddress = KOPrefs::instance()->email();
}
