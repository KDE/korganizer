/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KORG_NOPRINTER

#include "calprintyearconfig_base.h"
#include "yearprint.h"

#include <libkcal/calendar.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kcalendarsystem.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpainter.h>

class YearPrintFactory : public KOrg::PrintPluginFactory {
  public:
    KOrg::PrintPlugin *create() { return new CalPrintYear; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_yearlyprint, YearPrintFactory )


/**************************************************************
 *           Print Year
 **************************************************************/

QWidget *CalPrintYear::createConfigWidget( QWidget *w )
{
  return new CalPrintYearConfig_Base( w );
}

void CalPrintYear::readSettingsWidget()
{
  CalPrintYearConfig_Base *cfg =
      dynamic_cast<CalPrintYearConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mYear = cfg->mYear->value();
    mPages = cfg->mPages->currentText().toInt();
    mSubDaysEvents = (cfg->mSubDays->currentItem()==0)?Text:TimeBoxes;
    mHolidaysEvents = (cfg->mHolidays->currentItem()==0)?Text:TimeBoxes;
  }
}

void CalPrintYear::setSettingsWidget()
{
  CalPrintYearConfig_Base *cfg =
      dynamic_cast<CalPrintYearConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    const KCalendarSystem *calsys = calendarSystem();
    QDate start;
    calsys->setYMD( start, mYear, 1, 1 );
    int months = calsys->monthsInYear( start );
    int pages=0, prevPages=0;
    for ( int i=1; i<= months; ++i ) {
      pages = (months-1)/i + 1;
      if ( pages != prevPages ) {
        prevPages = pages;
        cfg->mPages->insertItem( QString::number( pages ), 0 );
      }
    }

    cfg->mYear->setValue( mYear );
    cfg->mPages->setCurrentText( QString::number( mPages ) );

    cfg->mSubDays->setCurrentItem( (mSubDaysEvents==Text)?0:1 );
    cfg->mHolidays->setCurrentItem( (mHolidaysEvents==Text)?0:1 );
  }
}

void CalPrintYear::loadConfig()
{
  if ( mConfig ) {
    mYear = mConfig->readNumEntry( "Year", 2007 );
    mPages = mConfig->readNumEntry( "Pages", 1 );
    mSubDaysEvents = mConfig->readNumEntry( "ShowSubDayEventsAs", TimeBoxes );
    mHolidaysEvents = mConfig->readNumEntry( "ShowHolidaysAs", Text );
  }
  setSettingsWidget();
}

void CalPrintYear::saveConfig()
{
  kdDebug(5850) << "CalPrintYear::saveConfig()" << endl;

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Year", mYear );
    mConfig->writeEntry( "Pages", mPages );
    mConfig->writeEntry( "Pages", mPages );
    mConfig->writeEntry( "ShowSubDayEventsAs", mSubDaysEvents );
    mConfig->writeEntry( "ShowHolidaysAs", mHolidaysEvents );
  }
}

KPrinter::Orientation CalPrintYear::defaultOrientation()
{
  return ( mPages == 1 )?(KPrinter::Landscape):(KPrinter::Portrait);
}


void CalPrintYear::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintYearConfig_Base *cfg =
      dynamic_cast<CalPrintYearConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mYear->setValue( from.year() );
  }
}

void CalPrintYear::print( QPainter &p, int width, int height )
{
kdDebug()<<"CalPrintYear::print, width: "<<width<<", height: "<<height<<endl;
  QRect headerBox( 0, 0, width, headerHeight() );
kdDebug()<<"headerBox: "<<headerBox<<endl;
  const KCalendarSystem *calsys = calendarSystem();
  KLocale *locale = KGlobal::locale();
  if ( !calsys || !locale ) return;

  QDate start;
  calsys->setYMD( start, mYear, 1, 1 );

  // Determine the nr of months and the max nr of days per month (dependent on
  // calendar system!!!!)
  QDate temp( start );
  int months = calsys->monthsInYear( start );
  int maxdays = 1;
  for ( int i = 1; i< months; ++i ) {
    maxdays = QMAX( maxdays, temp.daysInMonth() );
    temp = calsys->addMonths( temp, 1 );
  }

  // Now determine the months per page so that the printout fits on
  // exactly mPages pages
  int monthsPerPage = (months-1) / mPages + 1;
  int pages = (months-1) / monthsPerPage + 1;
  int thismonth = 0;
  temp = start;
  for ( int page = 0; page < pages; ++page ) {
    if ( page > 0 ) {
      mPrinter->newPage();
    }
    QDate end( calsys->addMonths( start, monthsPerPage ) );
    end = calsys->addDays( end, -1 );
    QString title;
    if ( orientation() == KPrinter::Landscape ) {
      title = i18n("date from - to", "%1 - %2");
    } else {
      title = i18n("date from -\nto", "%1 -\n%2");
    }
    drawHeader( p, title
        .arg( locale->formatDate( start ) )
        .arg( locale->formatDate( end ) ),
      calsys->addMonths( start, -1), calsys->addMonths( start, monthsPerPage ),
      headerBox );

    QRect monthesBox( headerBox );
    monthesBox.setTop( monthesBox.bottom() + padding() );
    monthesBox.setBottom( height );

    drawBox( p, BOX_BORDER_WIDTH, monthesBox );
    float monthwidth = float(monthesBox.width()) / float( monthsPerPage );

    for ( int j=0; j<monthsPerPage; ++j ) {
      if ( ++thismonth > months ) break;
      int xstart = int(j*monthwidth + 0.5);
      int xend = int((j+1)*monthwidth + 0.5);
      QRect monthBox( xstart, monthesBox.top(), xend-xstart, monthesBox.height() );
      drawMonth( p, temp, monthBox, maxdays, mSubDaysEvents, mHolidaysEvents );

      temp = calsys->addMonths( temp, 1 );
    }
    start = calsys->addMonths( start, monthsPerPage );
  }
}

#endif
