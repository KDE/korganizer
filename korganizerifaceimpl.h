/*
    This file is part of KOrganizer
    Copyright (c) 2004  Bo Thorsen <bo@sonofthor.dk>

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
 */

#ifndef KORGANIZER_SHARED_H
#define KORGANIZER_SHARED_H

#include "korganizeriface.h"

#include <qobject.h>
#include <kdepimmacros.h>

class ActionManager;


class KDE_EXPORT KOrganizerIfaceImpl : public QObject, virtual public KOrganizerIface {
public:
  KOrganizerIfaceImpl( ActionManager* mActionManager,
                       QObject* parent=0, const char* name=0 );
  ~KOrganizerIfaceImpl();

  bool openURL( const QString &url );
  bool mergeURL( const QString &url );
  void closeURL();
  bool saveURL();
  bool saveAsURL( const QString &url );
  QString getCurrentURLasString() const;

  bool editIncidence( const QString &uid );
  bool deleteIncidence( const QString &uid );

private:
  ActionManager* mActionManager;
};


#endif // KORGANIZER_SHARED_H

