/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>
	Calendar routines from Hdate by Amos Shapir 1978 (rev. 1985, 1992)

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
*/


#include "hebrew.h"

class HebrewFactory : public CalendarDecorationFactory {
  public:
    CalendarDecoration *create() { return new Hebrew; }
};

extern "C" {
  void *init_libkorg_hebrew()
  {
    return (new HebrewFactory);
  }
}


QString Hebrew::shortText(const QDate &date)
{

	hmname << i18n("These are months of the Jewish calendar, and don't have "
	"translations. They may have different spellings in your language; otherwise "
	"just translate the sound to your characters.", "Tishrei") << i18n("Heshvan") 
	<< 	i18n("Kislev")	<< i18n("Tevet") << i18n("Shvat") << i18n("Adar") 	<< 
	i18n("Nissan") << i18n("Iyar") << i18n("Sivan")	<< i18n("Tammuz") << i18n("Av") 
	<< 	i18n("Elul")	<< i18n("The I is a Roman 1", "Adar I") <<  i18n("The II is a" 
	" Roman 2", "Adar II");
	int day = date.day();
	int month = date.month();
	int year = date.year();
	
	struct hdate *h;
	h = hebrewDate( day, month, year); 
	QString result = QString::number(h->hd_day+1) + " " + hmname[h->hd_mon];
	return result;
}

QString Hebrew::info()
{
	return i18n("This plugin provides the date in the Jewish calendar.");
}

/* constants, in 1/18th of minute */
#define HOUR 1080
#define DAY  (24*HOUR)
#define WEEK (7*DAY)
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))

/* no. of days in y years */
int Hebrew::dysiz(int y)
//	register y;
{
	register m, nm, dw, s, l;

	l = y*7+1;	/* no. of leap months */
	m = y*12+l/19;	/* total no. of months */
	l %= 19;
	nm = m*MONTH+M(1+6,779); /* molad new year 3744 (16BC) + 6 hours */
	s = m*28+nm/DAY-2;

	nm %= WEEK;
	dw = nm/DAY;
	nm %= DAY;

	/* special cases of Molad Zaken */
	if(l < 12 && dw==3 && nm>=M(9+6,204) ||
	l < 7 && dw==2 && nm>=M(15+6,589))
		s++,dw++;
	/* ADU */
	if(dw == 1 || dw == 4 || dw == 6)
		s++;
	return s;
}


/*
 | compute date structure from no. of days since 1 Tishrei 3744
 */
/*struct hdate *   
hdate(d, m, y)
	register m, y, d; */
struct hdate*  Hebrew::hebrewDate(int d, int m, int y) /* register m, y, d;*/

{
	static struct hdate h;
	register s;

	if((m -= 2) <= 0) {
		m += 12;
		y--;
	}
	/* no. of days, Julian calendar */
	d += 365*y+y/4+367*m/12+5968;
	d -= y/100-y/400-2;
	h.hd_dw = (d+1)%7;

	/* compute the year */
	y += 16;
	s = dysiz(y);
	m = dysiz(y+1);
	while(d >= m) {	/* computed year was underestimated */
		s = m;
		y++;
		m = dysiz(y+1);
	}
	d -= s;
	s = m-s;	/* size of current year */
	y += 3744;

	h.hd_flg = s%10-4;

	/* compute day and month */
	if(d >= s-236) {	/* last 8 months are regular */
		d -= s-236;
		m = d*2/59;
		d -= (m*59+1)/2;
		m += 4;
		if(s>365 && m<=5)	/* Adar of Meuberet */
			m += 8;
	} else {
		/* first 4 months have 117-119 days */
		s = 114+s%10;
		m = d*4/s;
		d -= (m*s+3)/4;
	}

	h.hd_day = d;
	h.hd_mon = m;
	h.hd_year = y;
	return(&h);
}
