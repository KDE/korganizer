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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"

#include "version.h"

using namespace KOrg;

AboutData *AboutData::mSelf = 0;

AboutData::AboutData()
  : KAboutData( "korganizer", I18N_NOOP("KOrganizer"), korgVersion,
                I18N_NOOP("A Personal Organizer for KDE"),
                KAboutData::License_GPL,
                "(c) 1997-1999 Preston Brown\n"
                "(c) 2000-2004 Cornelius Schumacher", 0,
                "http://korganizer.kde.org" )
{
  addAuthor("Reinhold Kainhofer",I18N_NOOP("Current Maintainer"),
            "reinhold@kainhofer.com");
  addAuthor("Cornelius Schumacher",I18N_NOOP("Co-Maintainer"),
            "schumacher@kde.org");
  addAuthor("Preston Brown",I18N_NOOP("Original Author"),
            "pbrown@kde.org");
  addCredit("Richard Apodaca");
  addCredit("Jan-Pascal van Best");
  addCredit("Laszlo Boloni");
  addCredit("Barry Benowitz");
  addCredit("Christopher Beard");
  addCredit("Ian Dawes");
  addCredit("Thomas Eitzenberger");
  addCredit("Neil Hart");
  addCredit("Declan Houlihan");
  addCredit("Hans-Jürgen Husel");
  addCredit("Tim Jansen");
  addCredit("Christian Kirsch");
  addCredit("Tobias König");
  addCredit("Martin Koller");
  addCredit("Uwe Koloska");
  addCredit("Glen Parker");
  addCredit("Dan Pilone");
  addCredit("Roman Rohr");
  addCredit("Don Sanders");
  addCredit("Bram Schoenmakers");
  addCredit("Günter Schwann");
  addCredit("Herwin Jan Steehouwer");
  addCredit("Nick Thompson");
  addCredit("Bo Thorsen");
  addCredit("Larry Wright");
  addCredit("Thomas Zander");
  addCredit("Fester Zigterman");
  addCredit("Mario Teijeiro");
}
    
AboutData *AboutData::self()
{
  if ( !mSelf ) mSelf = new AboutData;
  return mSelf;
}
