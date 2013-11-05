/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "yearprint.h"

#include <KCalendarSystem>

class YearPrintFactory : public KOrg::PrintPluginFactory
{
  public:
    KOrg::PrintPlugin *createPluginFactory() { return new CalPrintYear; }
};

K_EXPORT_PLUGIN( YearPrintFactory )

/**************************************************************
 *           Print Year
 **************************************************************/

QWidget *CalPrintYear::createConfigWidget( QWidget *w )
{
  return new CalPrintYearConfig( w );
}

void CalPrintYear::readSettingsWidget()
{
  CalPrintYearConfig *cfg = dynamic_cast<CalPrintYearConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mYear = cfg->mYear->value();
    mPages = cfg->mPages->currentText().toInt();
    mSubDaysEvents = ( cfg->mSubDays->currentIndex() == 0 ) ? Text : TimeBoxes;
    mHolidaysEvents = ( cfg->mHolidays->currentIndex() == 0 ) ? Text : TimeBoxes;
  }
}

void CalPrintYear::setSettingsWidget()
{
  CalPrintYearConfig *cfg =
      dynamic_cast<CalPrintYearConfig*>( (QWidget *)mConfigWidget );
  if ( cfg ) {
    const KCalendarSystem *calsys = calendarSystem();
    QDate start;
    calsys->setDate( start, mYear, 1, 1 );
    int months = calsys->monthsInYear( start );
    int prevPages = 0;
    for ( int i=1; i<= months; ++i ) {
      const int pages = ( months - 1 ) / i + 1;
      if ( pages != prevPages ) {
        prevPages = pages;
        cfg->mPages->addItem( QString::number( pages ), 0 );
      }
    }

    cfg->mYear->setValue( mYear );
    cfg->mPages->setItemText( cfg->mPages->currentIndex(), QString::number( mPages ) );

    cfg->mSubDays->setCurrentIndex( ( mSubDaysEvents == Text ) ? 0 : 1 );
    cfg->mHolidays->setCurrentIndex( ( mHolidaysEvents == Text ) ? 0 : 1 );
  }
}

void CalPrintYear::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Yearprint" );
    mYear = config.readEntry( "Year", QDate::currentDate().year() );
    mPages = config.readEntry( "Pages", 1 );
    mSubDaysEvents = config.readEntry( "ShowSubDayEventsAs", (int)TimeBoxes );
    mHolidaysEvents = config.readEntry( "ShowHolidaysAs", (int)Text );
  }
  setSettingsWidget();
}

void CalPrintYear::saveConfig()
{
  kDebug();

  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Yearprint" );
    config.writeEntry( "Year", mYear );
    config.writeEntry( "Pages", mPages );
    config.writeEntry( "Pages", mPages );
    config.writeEntry( "ShowSubDayEventsAs", mSubDaysEvents );
    config.writeEntry( "ShowHolidaysAs", mHolidaysEvents );
  }
}

QPrinter::Orientation CalPrintYear::defaultOrientation()
{
  return ( mPages == 1 ) ? QPrinter::Landscape : QPrinter::Portrait;
}

void CalPrintYear::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintYearConfig *cfg = dynamic_cast<CalPrintYearConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mYear->setValue( from.year() );
  }
}

void CalPrintYear::print( QPainter &p, int width, int height )
{
  const KCalendarSystem *calsys = calendarSystem();
  KLocale *locale = KGlobal::locale();
  if ( !calsys || !locale ) {
    return;
  }

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  QDate start;
  calsys->setDate( start, mYear, 1, 1 );

  // Determine the nr of months and the max nr of days per month (dependent on
  // calendar system!!!!)
  QDate temp( start );
  int months = calsys->monthsInYear( start );
  int maxdays = 1;
  for ( int i = 1; i< months; ++i ) {
    maxdays = qMax( maxdays, temp.daysInMonth() );
    temp = calsys->addMonths( temp, 1 );
  }

  // Now determine the months per page so that the printout fits on
  // exactly mPages pages
  int monthsPerPage = ( months - 1 ) / mPages + 1;
  int pages = ( months - 1 ) / monthsPerPage + 1;
  int thismonth = 0;
  temp = start;
  for ( int page = 0; page < pages; ++page ) {
    if ( page > 0 ) {
      mPrinter->newPage();
    }
    QDate end( calsys->addMonths( start, monthsPerPage ) );
    end = calsys->addDays( end, -1 );
    QString stdate = locale->formatDate( start );
    QString endate = locale->formatDate( end );
    QString title;
    if ( orientation() == QPrinter::Landscape ) {
      title = i18nc( "date from - to", "%1 - %2", stdate, endate );
    } else {
      title = i18nc( "date from -\nto", "%1 -\n%2", stdate, endate );
    }
    drawHeader( p, title, calsys->addMonths( start, -1 ),
                calsys->addMonths( start, monthsPerPage ), headerBox );

    QRect monthesBox( headerBox );
    monthesBox.setTop( monthesBox.bottom() + padding() );
    monthesBox.setBottom( height );

    drawBox( p, BOX_BORDER_WIDTH, monthesBox );
    float monthwidth = float( monthesBox.width() ) / float( monthsPerPage );

    for ( int j=0; j<monthsPerPage; ++j ) {
      if ( ++thismonth > months ) {
        break;
      }
      int xstart = int( j * monthwidth + 0.5 );
      int xend = int( ( j + 1 ) * monthwidth + 0.5 );
      QRect monthBox( xstart, monthesBox.top(), xend - xstart, monthesBox.height() );
      drawMonth( p, temp, monthBox, maxdays, mSubDaysEvents, mHolidaysEvents );

      temp = calsys->addMonths( temp, 1 );
    }

    drawFooter( p, footerBox );
    start = calsys->addMonths( start, monthsPerPage );
  }
}
