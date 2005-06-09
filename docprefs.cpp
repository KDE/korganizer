/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "docprefs.h"

KSimpleConfig *DocPrefs::mConfig = 0;

DocPrefs::DocPrefs( const QString &type )
{
    if ( !mConfig ) {
        mConfig = new KSimpleConfig( locateLocal( "data", "korganizer/docprefs." + type + ".kconfig" ) );
    }
}

DocPrefs::~DocPrefs()
{
    mConfig->sync();
}

void DocPrefs::setDoc( const QString &identifier )
{
    mDocId = identifier;
}

QString DocPrefs::doc() const
{
    return mDocId;
}

bool DocPrefs::readBoolEntry( const QString &id ) const
{
    mConfig->setGroup( mDocId );
    bool result = mConfig->readBoolEntry( id, false );
//    kdDebug(5850) << "DocPrefs::readEntry(): " << id << " : " << (result ? "True" : "False" ) << endl;
    return result;
}

void DocPrefs::writeEntry( const QString &id, bool value )
{
//    kdDebug(5850) << "DocPrefs::writeEntry(): " << id << " : " << (value ? "True" : "False" ) << endl;
    mConfig->setGroup( mDocId );
    mConfig->writeEntry( id, value );
}
