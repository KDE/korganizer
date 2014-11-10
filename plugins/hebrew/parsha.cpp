/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  Calendar routines from Hebrew Calendar by Frank Yellin.
  Based on some GNU Emacs code (lisp/calendar/cal-hebrew.el),
  copyright (C) 1995, 1997 Free Software Foundation, Inc.,
  authors: Nachum Dershowitz <nachum@cs.uiuc.edu>
           Edward M. Reingold <reingold@cs.uiuc.edu>

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
*/

#include "parsha.h"

#include <QDebug>
#include <KLocalizedString>

QString Parsha::findParshaName(int dayNumber, int kvia, bool isLeapYear,
                               bool useIsraelSettings)
{
    // The names of the Parshiot.
    static QStringList parshiotNames = QStringList();
    parshiotNames
            << i18nc("These are weekly readings and do not have translations. "
                     "They may have different spellings in your language; "
                     "otherwise, just translate the sound to your characters", "Bereshit")
            <<  i18n("Noach")
            <<  i18n("Lech L'cha")
            <<  i18n("Vayera")
            <<  i18n("Chaye Sarah")
            <<  i18n("Toldot")
            <<  i18n("Vayetze")
            <<  i18n("Vayishlach")
            <<  i18n("Vayeshev")
            <<  i18n("Miketz")
            <<  i18n("Vayigash")
            <<  i18n("Vayechi")
            <<  i18n("Shemot")
            <<  i18n("Vaera")
            <<  i18n("Bo")
            <<  i18n("Beshalach")
            <<  i18n("Yitro")
            <<  i18n("Mishpatim")
            <<  i18n("Terumah")
            <<  i18n("Tetzaveh")
            <<  i18n("Ki Tisa")
            <<  i18n("Vayakhel")
            <<  i18n("Pekudei")
            <<  i18n("Vayikra")
            <<  i18n("Tzav")
            <<  i18n("Shemini")
            <<  i18n("Tazria")
            <<  i18n("Metzora")
            <<  i18n("Acharei Mot")
            <<  i18n("Kedoshim")
            <<  i18n("Emor")
            <<  i18n("Behar")
            <<  i18n("Bechukotai")
            <<  i18n("Bemidbar")
            <<  i18n("Naso")
            <<  i18n("Behaalotcha")
            <<  i18n("Shelach")
            <<  i18n("Korach")
            <<  i18n("Chukat")
            <<  i18n("Balak")
            <<  i18n("Pinchas")
            <<  i18n("Matot")
            <<  i18n("Masei")
            <<  i18n("Devarim")
            <<  i18n("Vaetchanan")
            <<  i18n("Ekev")
            <<  i18n("Reeh")
            <<  i18n("Shoftim")
            <<  i18n("Ki Tetze")
            <<  i18n("Ki Tavo")
            <<  i18n("Nitzavim")
            <<  i18n("Vayelech")
            <<  i18n("Haazinu");

    /*
      Tables for each of the year types. XX indicates that it is a Holiday, and a
      special parsha is read that week. For some year types among the 14, Israel
      is different from the diaspora.

      The names indicate the day of the week on which Rosh Hashanah fell, whether
      it is a short/normal/long year (kvia=0,1,2), and whether it is a leap year.
      As said earlier, some year types also have an _Israel version.

      The numbers are indices in the list above for a given week.
      Numbers > 100 indicate a double parsha, e.g. 150 means read both table
      entries 150 - 100 = 50 and 150 - 99 = 51.

      These tables were stolen (with some massaging) from the Gnu code.
    */

    static const quint8 XX = 255;

    /* Non-leap years */
    static const quint8 SatShort[] = {
        XX,  52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
        11,  12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  25,
        126, 128,  30, 131,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,
        45,  46,  47,  48,  49,  50,
    };
    static const quint8 SatLong[] = {
        XX,  52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
        11,  12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  25,
        126, 128,  30, 131,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,
        45,  46,  47,  48,  49, 150,
    };

    static const quint8 MonShort[] = {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  25, 126,
        128,  30, 131,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,  45,
        46,  47,  48,  49, 150,
    };
    static const quint8 MonLong[] =        /* split */
    {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  25, 126,
        128,  30, 131,  33,  XX,  34,  35,  36,  37, 138,  40, 141,  43,  44,  45,
        46,  47,  48,  49, 150,
    };
#define MonLong_Israel MonShort

#define TueNormal MonLong
#define TueNormal_Israel MonShort

    static const quint8 ThuNormal[] = {
        52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  XX,  25,
        126, 128,  30, 131,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,
        45,  46,  47,  48,  49,  50,
    };
    static const quint8 ThuNormal_Israel[] = {
        52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20, 121,  23,  24,  XX,  25, 126,
        128,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,
        45,  46,  47,  48,  49,  50,
    };
    static const quint8 ThuLong[] = {
        52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  XX,  25,
        126, 128,  30, 131,  33,  34,  35,  36,  37,  38,  39,  40, 141,  43,  44,
        45,  46,  47,  48,  49,  50,
    };

    /* Leap years */
    static const quint8 SatShortLeap[] = {
        XX,  52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
        11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
        26,  27,  XX,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
        40, 141,  43,  44,  45,  46,  47,  48,  49, 150,
    };
    static const quint8 SatLongLeap[] = {
        XX,  52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
        11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
        26,  27,  XX,  28,  29,  30,  31,  32,  33,  XX,  34,  35,  36,  37, 138,
        40, 141,  43,  44,  45,  46,  47,  48,  49, 150,
    };
#define SatLongLeap_Israel SatShortLeap

    static const quint8 MonShortLeap[] = {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  XX,  28,  29,  30,  31,  32,  33,  XX,  34,  35,  36,  37, 138,  40,
        141,  43,  44,  45,  46,  47,  48,  49, 150,
    };
    static const quint8 MonShortLeap_Israel[] = {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  XX,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
        141,  43,  44,  45,  46,  47,  48,  49, 150,
    };
    static const quint8 MonLongLeap[] = {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  XX,  XX,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
        40, 141,  43,  44,  45,  46,  47,  48,  49,  50,
    };
    static const quint8 MonLongLeap_Israel[] = {
        51,  52,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  XX,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
        41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
    };

#define TueNormalLeap  MonLongLeap
#define TueNormalLeap_Israel  MonLongLeap_Israel

    static const quint8 ThuShortLeap[] = {
        52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  28,  XX,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
        41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
    };
    static const quint8 ThuLongLeap[] = {
        52,  XX,  XX,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  28,  XX,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
        41,  42,  43,  44,  45,  46,  47,  48,  49, 150,
    };

    /* Make the calculations */
    int week = dayNumber / 7; // week of the year
    const quint8 *array = 0;
    int index;

    /* Get the appropriate array by exhaustive search into the 14 year types.
       Since we know it's a Shabbat, we can find out what day Rosh Hashanah was on
       by looking at dayNumber % 7. */
    if (!isLeapYear) {
        switch (dayNumber % 7) {
        case 1: /* Rosh Hashanah was on a Saturday */
            if (kvia == 0) {
                array = SatShort;
            } else if (kvia == 2) {
                array = SatLong;
            }
            break;
        case 6: /* Rosh Hashanah was on a Monday */
            if (kvia == 0) {
                array = MonShort;
            } else if (kvia == 2) {
                array = useIsraelSettings ? MonLong_Israel : MonLong;
            }
            break;
        case 5: /* Rosh Hashanah was on a Tuesday */
            if (kvia == 1) {
                array = useIsraelSettings ? TueNormal_Israel : TueNormal;
            }
            break;
        case 3: /* Rosh Hashanah was on a Thursday */
            if (kvia == 1) {
                array = useIsraelSettings ? ThuNormal_Israel : ThuNormal;
            } else if (kvia == 2) {
                array = ThuLong;
            }
            break;
        }
    } else { /* leap year */
        switch (dayNumber % 7) {
        case 1: /* Rosh Hashanah was on a Sat */
            if (kvia == 0) {
                array = SatShortLeap;
            } else if (kvia == 2) {
                array = useIsraelSettings ? SatLongLeap_Israel : SatLongLeap;
            }
            break;
        case 6: /* Rosh Hashanah was on a Monday */
            if (kvia == 0) {
                array = useIsraelSettings ? MonShortLeap_Israel : MonShortLeap;
            } else if (kvia == 2) {
                array = useIsraelSettings ? MonLongLeap_Israel : MonLongLeap;
            }
            break;
        case 5: /* Rosh Hashanah was on a Tuesday */
            if (kvia == 1) {
                array = useIsraelSettings ? TueNormalLeap_Israel : TueNormalLeap;
            }
            break;
        case 3: /* Rosh Hashanah was on a Thursday */
            if (kvia == 0) {
                array = ThuShortLeap;
            } else if (kvia == 2) {
                array = ThuLongLeap;
            }
            break;
        }
    }
    QString buffer;

    if (!array) {   /* Something is terribly wrong! */
        buffer = QLatin1String("??Parsha??");
        qWarning() << "Hebrew Plugin: Was not able to determine the Parsha."
                   << "Please report this as a bug.";
        return buffer;
    }

    index = array[week];
    if (index == XX) {    // no Parsha this week.
        buffer.clear();
        return buffer;
    } else if (index < 100) {
        buffer = parshiotNames[index];
        return buffer;
    } else {  // Create a double parsha
        buffer = parshiotNames[index - 100] + QLatin1Char('-') + parshiotNames[index - 99];
        return buffer;
    }
}
