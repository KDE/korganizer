/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KNEWSTUFFGENERIC_H
#define KNEWSTUFFGENERIC_H

#include "knewstuff.h"

class KConfig;

/**
* Basic KNewStuff class with predefined actions.
* This class is used for data uploads and installation.
* \code
* QString payload, preview;
* KNewStuffGeneric *ns = new KNewStuffGeneric("kamikaze/level", this);
* ns->upload(payload, preview);
* \endcode
*/
class KNewStuffGeneric : public KNewStuff
{
  public:
    /**
      Constructor.

      @param type Hotstuff data type such as "korganizer/calendar".
      @param parent The parent window.
    */
    KNewStuffGeneric( const QString &type, QWidget *parent = 0 );
    ~KNewStuffGeneric();

    /**
      Installs a downloaded file according to the application's configuration.

      @param Filename of the donwloaded file.
      @return Installation success.
    */
    bool install( const QString &fileName );

    /**
      Creates a file suitable for upload.
      Note that this method always fails, since using KNewStuffGeneric
      means that the provided file must already be in a usable format.

      @param fileName Filename
      @return Creation success.
    */
    bool createUploadFile( const QString &fileName );

    /**
      Queries the preferred destination file for a download.

      @param entry Hotstuff data entry.
      @return Destination filename.
    */
    QString downloadDestination( KNS::Entry *entry );

  private:
    KConfig *mConfig;
};

#endif
