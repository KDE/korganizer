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
#include "parsha.h"
#include <klocale.h>

QStringList Parsha::parshiot_names;

Parsha::Parsha()
{

}

Parsha::~Parsha()
{
}

QString
  Parsha::FindParshaName(int daynumber, int kvia, bool leap_p,
			 bool israel_p)
{
// The names of the Parshiot.
  parshiot_names <<
    i18n
    ("These are weekly readings and don't have translations. They may have different  spellings in your language; otherwise just translate the sound to your characters",
     "Bereshit") << i18n("Noach") << i18n("Lech L'cha") <<
    i18n("Vayera") << i18n("Chaye Sarah") << i18n("Toldot") <<
    i18n("Vayetze") << i18n("Vayishlach") << i18n("Vayeshev") <<
    i18n("Miketz") << i18n("Vayigash") << i18n("Vayechi") <<
    i18n("Shemot") << i18n("Vaera") << i18n("Bo") << i18n("Beshalach")
    << i18n("Yitro") << i18n("Mishpatim") << i18n("Terumah") <<
    i18n("Tetzaveh") << i18n("Ki Tisa") << i18n("Vayakhel") <<
    i18n("Pekudei") << i18n("Vayikra") << i18n("Tzav") <<
    i18n("Shemini") << i18n("Tazria") << i18n("Metzora") <<
    i18n("Acharei Mot") << i18n("Kedoshim") << i18n("Emor") <<
    i18n("Behar") << i18n("Bechukotai") << i18n("Bemidbar") <<
    i18n("Naso") << i18n("Behaalotcha") << i18n("Shelach") <<
    i18n("Korach") << i18n("Chukat") << i18n("Balak") <<
    i18n("Pinchas") << i18n("Matot") << i18n("Masei") <<
    i18n("Devarim") << i18n("Vaetchanan") << i18n("Ekev") <<
    i18n("Reeh") << i18n("Shoftim") << i18n("Ki Tetze") <<
    i18n("Ki Tavo") << i18n("Nitzavim") << i18n("Vayelech") <<
    i18n("Haazinu");

// Tables for each of the year types.  XX indicates that it is a Holiday, and
// a special parsha is read that week.  For some year types, Israel is different
// than the diaspora.
//
// The names indicate the day of the week on which Rosh Hashanah fell, whether
// it is a short/normal/long year (kvia=0,1,2), and whether it is a leap year.
// Some year types also have an _Israel version.
//
// Numbers are indices into the table above for a given week.  Numbers > 100 indicate
// a double parsha.  E.g. 150 means read both table entries 50 and 51.
//
// These tables were stolen (with some massaging) from the GNU code.

#define XX 255
  static unsigned const char Sat_short[] =
    { XX, 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, 25,
    126, 128, 30, 131, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44,
    45, 46, 47, 48, 49, 50,
  };

  static unsigned const char Sat_long[] =
    { XX, 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, 25,
    126, 128, 30, 131, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44,
    45, 46, 47, 48, 49, 150,
  };

  static unsigned const char Mon_short[] =
    { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, 25, 126,
    128, 30, 131, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44, 45,
    46, 47, 48, 49, 150,
  };

  static unsigned const char Mon_long[] =	/* split */
  { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, 25, 126,
    128, 30, 131, 33, XX, 34, 35, 36, 37, 138, 40, 141, 43, 44, 45,
    46, 47, 48, 49, 150,
  };

#define Mon_long_Israel Mon_short

#define Tue_normal  Mon_long
#define Tue_normal_Israel  Mon_short

  static unsigned const char Thu_normal[] =
    { 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, XX, 25,
    126, 128, 30, 131, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44,
    45, 46, 47, 48, 49, 50,
  };
  static unsigned const char Thu_normal_Israel[] =
    { 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 121, 23, 24, XX, 25, 126,
    128, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44,
    45, 46, 47, 48, 49, 50,
  };

  static unsigned const char Thu_long[] =
    { 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, XX, 25,
    126, 128, 30, 131, 33, 34, 35, 36, 37, 38, 39, 40, 141, 43, 44,
    45, 46, 47, 48, 49, 50,
  };

  static unsigned const char Sat_short_leap[] =
    { XX, 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, XX, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 141, 43, 44, 45, 46, 47, 48, 49, 150,
  };

  static unsigned const char Sat_long_leap[] =
    { XX, 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, XX, 28, 29, 30, 31, 32, 33, XX, 34, 35, 36, 37, 138,
    40, 141, 43, 44, 45, 46, 47, 48, 49, 150,
  };

#define Sat_long_leap_Israel  Sat_short_leap

  static unsigned const char Mon_short_leap[] =
    { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, XX, 28, 29, 30, 31, 32, 33, XX, 34, 35, 36, 37, 138, 40,
    141, 43, 44, 45, 46, 47, 48, 49, 150,
  };
  static unsigned const char Mon_short_leap_Israel[] =
    { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, XX, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    141, 43, 44, 45, 46, 47, 48, 49, 150,
  };

  static unsigned const char Mon_long_leap[] =
    { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, XX, XX, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 141, 43, 44, 45, 46, 47, 48, 49, 50,
  };
  static unsigned const char Mon_long_leap_Israel[] =
    { 51, 52, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, XX, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  };

#define Tue_normal_leap  Mon_long_leap
#define Tue_normal_leap_Israel  Mon_long_leap_Israel

  static unsigned const char Thu_short_leap[] =
    { 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, XX, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  };

  static unsigned const char Thu_long_leap[] =
    { 52, XX, XX, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, XX, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 150,
  };

/* Find the parsha for a given day of the year.  daynumber is the day of the year.
 * kvia and leap_p refer to the year type.
 */

  int week = daynumber / 7;	// week of the year
  unsigned const char *array = NULL;
  int index;

  // get the appropriate array by exhaustive search into the 14 year types.  Since we
  // know it's a Shabbat, we can find out what day Rosh Hashanah was on by looking
  // at daynumber %7.
  if (!leap_p)
      {
	switch (daynumber % 7)
	    {
	    case 1:		/* RH was on a Saturday */
	      if (kvia == 0)
		array = Sat_short;
	      else if (kvia == 2)
		array = Sat_long;
	      break;
	    case 6:		/* RH was on a Monday */
	      if (kvia == 0)
		array = Mon_short;
	      else if (kvia == 2)
		array = israel_p ? Mon_long_Israel : Mon_long;
	      break;
	    case 5:		/* RH was on a Tueday */
	      if (kvia == 1)
		array = israel_p ? Tue_normal_Israel : Tue_normal;
	      break;
	    case 3:		/* RH was on a Thu */
	      if (kvia == 1)
		array = israel_p ? Thu_normal_Israel : Thu_normal;
	      else if (kvia == 2)
		array = Thu_long;
	      break;
	    }
      }
  else				/* leap year */
    switch (daynumber % 7)
	{
	case 1:		/* RH was on a Sat */
	  if (kvia == 0)
	    array = Sat_short_leap;
	  else if (kvia == 2)
	    array = israel_p ? Sat_long_leap_Israel : Sat_long_leap;
	  break;
	case 6:		/* RH was on a Mon */
	  if (kvia == 0)
	    array = israel_p ? Mon_short_leap_Israel : Mon_short_leap;
	  else if (kvia == 2)
	    array = israel_p ? Mon_long_leap_Israel : Mon_long_leap;
	  break;
	case 5:		/* RH was on a Tue */
	  if (kvia == 1)
	    array =
	      israel_p ? Tue_normal_leap_Israel : Tue_normal_leap;
	  break;
	case 3:		/* RH was on a Thu */
	  if (kvia == 0)
	    array = Thu_short_leap;
	  else if (kvia == 2)
	    array = Thu_long_leap;
	  break;

	}

  QString buffer;

  if (array == NULL)
    /* Something is terribly wrong. */
      {
	buffer = "??Parsha??";
	return buffer;
      }
  index = array[week];
  if (index == XX)		// no Parsha this week.
      {
	buffer = "";
	return buffer;
      }
  else if (index < 100)
      {
	buffer = parshiot_names[index];
	return buffer;
      }
  else
      {				// Create a double parsha
	buffer =
	  parshiot_names[index - 100] + "-" + parshiot_names[index -
							     99];
	return buffer;

      }
}
