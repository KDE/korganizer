/***************************************************************************
 *   Copyright (C) 2003 by Jonathan Singer                                             *
 *   jsinger@leeta.net                                                                                *
 *   Calendar routines from Hebrew Calendar by Frank Yellin                     *
 *                                                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                                       *
 ***************************************************************************/
#ifndef PARSHA_H
#define PARSHA_H

#include <qstring.h>
#include <qstringlist.h>

/**
@author Jonathan Singer
*/
class Parsha
{
public:

  Parsha();
  ~Parsha();
  static QString FindParshaName(int daynumber, int kvia, bool leap_p,
				bool israel_p);

private:
  static QStringList parshiot_names;
};

#endif
