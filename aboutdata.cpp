/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "aboutdata.h"
#include "version.h"
#include <klocale.h>

using namespace KOrg;

AboutData::AboutData()
  : KAboutData( "korganizer", 0, ki18n( "KOrganizer" ), korgVersion,
                ki18n( "A Personal Organizer" ),
                KAboutData::License_GPL,
                ki18n( "(c) 1997-1999 Preston Brown\n"
                       "(c) 2000-2004,2007 Cornelius Schumacher\n"
                       "(c) 2004-2005 Reinhold Kainhofer" ),
                KLocalizedString(),
                "http://korganizer.kde.org" )
{
  addAuthor( ki18n( "Reinhold Kainhofer"),ki18n( "Former Maintainer" ),
             "reinhold@kainhofer.com" );
  addAuthor( ki18n( "Cornelius Schumacher"),ki18n( "Former Maintainer" ),
             "schumacher@kde.org" );
  addAuthor( ki18n( "Allen Winter"),ki18n( "Janitorial Staff" ),
             "winter@kde.org" );
  addAuthor( ki18n( "Preston Brown"),ki18n( "Original Author" ),
             "pbrown@kde.org" );
  addCredit( ki18n( "Richard Apodaca" ) );
  addCredit( ki18n( "Mike Arthur" ) );
  addCredit( ki18n( "Jan-Pascal van Best" ) );
  addCredit( ki18n( "Laszlo Boloni" ) );
  addCredit( ki18n( "Barry Benowitz" ) );
  addCredit( ki18n( "Christopher Beard" ) );
  addCredit( ki18n( "Kalle Dalheimer" ) );
  addCredit( ki18n( "Ian Dawes" ) );
  addCredit( ki18n( "Thomas Eitzenberger" ) );
  addCredit( ki18n( "Neil Hart" ) );
  addCredit( ki18n( "Declan Houlihan" ) );
  addCredit( ki18n( "Hans-Jürgen Husel" ) );
  addCredit( ki18n( "Tim Jansen" ) );
  addCredit( ki18n( "Christian Kirsch" ) );
  addCredit( ki18n( "Tobias König" ) );
  addCredit( ki18n( "Martin Koller" ) );
  addCredit( ki18n( "Uwe Koloska" ) );
  addCredit( ki18n( "Glen Parker" ) );
  addCredit( ki18n( "Dan Pilone" ) );
  addCredit( ki18n( "Roman Rohr" ) );
  addCredit( ki18n( "Rafał Rzepecki" ),
             ki18n( "Part of work sponsored by Google with Summer of Code 2005" ) );
  addCredit( ki18n( "Don Sanders" ) );
  addCredit( ki18n( "Bram Schoenmakers" ) );
  addCredit( ki18n( "Günter Schwann" ) );
  addCredit( ki18n( "Herwin Jan Steehouwer" ) );
  addCredit( ki18n( "Mario Teijeiro" ) );
  addCredit( ki18n( "Nick Thompson" ) );
  addCredit( ki18n( "Bo Thorsen" ) );
  addCredit( ki18n( "Larry Wright" ) );
  addCredit( ki18n( "Thomas Zander" ) );
  addCredit( ki18n( "Fester Zigterman" ) );
}
