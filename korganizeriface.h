/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KORGANIZERIFACE_H
#define KORGANIZERIFACE_H

#include <dcopobject.h>


class KOrganizerIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual bool openURL(QString url) = 0;
    virtual bool mergeURL(QString url) = 0;
    virtual void closeURL() = 0;
    virtual bool saveURL() = 0;
    virtual bool saveAsURL(QString url) = 0;
    virtual QString getCurrentURLasString() const = 0;
    virtual bool editIncidence(QString uid) = 0;
    virtual bool deleteIncidence(QString uid) = 0;
};

#endif
