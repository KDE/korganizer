/*
  Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#ifndef KORG_PERSON_H
#define KORG_PERSON_H

#include <QStringList>
#include <Akonadi/Collection>

struct Person
{
    Person(): rootCollection(-1), updateDisplayName(false) {};
    QString name;
    QString uid;
    QString ou;
    QString mail;
    Akonadi::Collection::Id rootCollection;
    bool updateDisplayName;
    
    //FIXME not sure we actually require those two
    QStringList folderPaths;
    QList<Akonadi::Collection::Id> collections;
};

Q_DECLARE_METATYPE(Person);

#endif
