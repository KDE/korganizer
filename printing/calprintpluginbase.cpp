/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "calprintpluginbase.h"
#include "cellitem.h"

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kcalendarsystem.h>
#include <kwordwrap.h>
#include <kconfiggroup.h>

#include <QPainter>
#include <QLayout>
#include <QFrame>
#include <QLabel>
#include <QtAlgorithms>

#ifndef KORG_NOPRINTER

inline int round( const double x )
{
  return int( x > 0.0 ? x + 0.5 : x - 0.5 );
}

/******************************************************************
 **              The Todo positioning structure                  **
 ******************************************************************/
class CalPrintPluginBase::TodoParentStart
{
  public:
    TodoParentStart( QRect pt = QRect(), bool page = true )
      : mRect( pt ), mSamePage( page ) {}

    QRect mRect;
    bool mSamePage;
};

/******************************************************************
 **                     The Print item                           **
 ******************************************************************/

class PrintCellItem : public KOrg::CellItem
{
  public:
    PrintCellItem( Event *event, const KDateTime &start, const KDateTime &end )
      : mEvent( event ), mStart( start ), mEnd( end )
    {
    }

    Event *event() const { return mEvent; }

    QString label() const { return mEvent->summary(); }

    KDateTime start() const { return mStart; }
    KDateTime end() const { return mEnd; }

    /** Calculate the start and end date/time of the recurrence that
        happens on the given day */
    bool overlaps( KOrg::CellItem *o ) const
    {
      PrintCellItem *other = static_cast<PrintCellItem *>( o );
      return !( other->start() >= end() || other->end() <= start() );
    }

  private:
    Event *mEvent;
    KDateTime mStart, mEnd;
};

/******************************************************************
 **                    The Print plugin                          **
 ******************************************************************/

CalPrintPluginBase::CalPrintPluginBase()
  : PrintPlugin(), mUseColors( true ),
    mHeaderHeight(-1), mSubHeaderHeight( SUBHEADER_HEIGHT ),
    mMargin( MARGIN_SIZE ), mPadding( PADDING_SIZE ), mCalSys( 0 )
{
}

CalPrintPluginBase::~CalPrintPluginBase()
{
}

QWidget *CalPrintPluginBase::createConfigWidget( QWidget *w )
{
  QFrame *wdg = new QFrame( w );
  QVBoxLayout *layout = new QVBoxLayout( wdg );

  QLabel *title = new QLabel( description(), wdg );
  QFont titleFont( title->font() );
  titleFont.setPointSize( 20 );
  titleFont.setBold( true );
  title->setFont( titleFont );

  layout->addWidget( title );
  layout->addWidget( new QLabel( info(), wdg ) );
  layout->addSpacing( 20 );
  layout->addWidget(
    new QLabel( i18n( "This printing style does not have any configuration options." ), wdg ) );
  layout->addStretch();
  return wdg;
}

void CalPrintPluginBase::doPrint( QPrinter *printer )
{
  if ( !printer ) {
    return;
  }
  mPrinter = printer;
  QPainter p;

  mPrinter->setColorMode( mUseColors ? QPrinter::Color : QPrinter::GrayScale );

  p.begin( mPrinter );
  // TODO: Fix the margins!!!
  // the painter initially begins at 72 dpi per the Qt docs.
  // we want half-inch margins.
  int margins = margin();
  p.setViewport( margins, margins,
                 p.viewport().width() - 2 * margins,
                 p.viewport().height() - 2 * margins );
//   QRect vp( p.viewport() );
// vp.setRight( vp.right()*2 );
// vp.setBottom( vp.bottom()*2 );
//   p.setWindow( vp );
  int pageWidth = p.window().width();
  int pageHeight = p.window().height();
//   int pageWidth = p.viewport().width();
//   int pageHeight = p.viewport().height();

  print( p, pageWidth, pageHeight );

  p.end();
  mPrinter = 0;
}

void CalPrintPluginBase::doLoadConfig()
{
  if ( mConfig ) {
    KConfigGroup group( mConfig, description() );
    mConfig->sync();
    QDateTime dt = QDateTime::currentDateTime();
    mFromDate = group.readEntry( "FromDate", dt ).date();
    mToDate = group.readEntry( "ToDate", dt ).date();
    mUseColors = group.readEntry( "UseColors", true );
    setUseColors( mUseColors );
    loadConfig();
  } else {
    kDebug() << "No config available in loadConfig!!!!";
  }
}

void CalPrintPluginBase::doSaveConfig()
{
  if ( mConfig ) {
    KConfigGroup group( mConfig, description() );
    saveConfig();
    QDateTime dt = QDateTime::currentDateTime(); // any valid QDateTime will do
    dt.setDate( mFromDate );
    group.writeEntry( "FromDate", dt );
    dt.setDate( mToDate );
    group.writeEntry( "ToDate", dt );
    group.writeEntry( "UseColors", mUseColors );
    mConfig->sync();
  } else {
    kDebug() << "No config available in saveConfig!!!!";
  }
}

void CalPrintPluginBase::setKOrgCoreHelper( KOrg::CoreHelper *helper )
{
  PrintPlugin::setKOrgCoreHelper( helper );
  if ( helper ) {
    setCalendarSystem( helper->calendarSystem() );
  }
}

bool CalPrintPluginBase::useColors() const
{
  return mUseColors;
}

void CalPrintPluginBase::setUseColors( bool useColors )
{
  mUseColors = useColors;
}

QPrinter::Orientation CalPrintPluginBase::orientation() const
{
  return mPrinter ? mPrinter->orientation() : QPrinter::Portrait;
}

QTime CalPrintPluginBase::dayStart()
{
  QTime start( 8, 0, 0 );
  if ( mCoreHelper ) {
    start = mCoreHelper->dayStart();
  }
  return start;
}

void CalPrintPluginBase::setCategoryColors( QPainter &p, Incidence *incidence )
{
  QColor bgColor = categoryBgColor( incidence );
  if ( bgColor.isValid() ) {
    p.setBrush( bgColor );
  }
  QColor tColor( textColor( bgColor ) );
  if ( tColor.isValid() ) {
    p.setPen( tColor );
  }
}

QColor CalPrintPluginBase::categoryBgColor( Incidence *incidence )
{
  if ( mCoreHelper && incidence ) {
    return mCoreHelper->categoryColor( incidence->categories() );
  } else {
    return QColor();
  }
}

QColor CalPrintPluginBase::textColor( const QColor &color )
{
  return mCoreHelper ? mCoreHelper->textColor( color ) : QColor();
}

bool CalPrintPluginBase::isWorkingDay( const QDate &dt )
{
  return mCoreHelper ? mCoreHelper->isWorkingDay( dt ) : true;
}

QString CalPrintPluginBase::holidayString( const QDate &dt )
{
  return mCoreHelper ? mCoreHelper->holidayString(dt) : QString();
}

Event *CalPrintPluginBase::holiday( const QDate &dt )
{
  QString hstring( holidayString( dt ) );
  if ( !hstring.isEmpty() ) {
    //FIXME: KOPrefs::instance()->timeSpec()?
    KDateTime::Spec timeSpec = KPIM::KPimPrefs::timeSpec();
    KDateTime kdt( dt, QTime(), timeSpec );
    Event *holiday = new Event();
    holiday->setSummary( hstring );
    holiday->setDtStart( kdt );
    holiday->setDtEnd( kdt );
    holiday->setAllDay( true );
    holiday->setCategories( i18n( "Holiday" ) );
    return holiday;
  }
  return 0;
}

const KCalendarSystem *CalPrintPluginBase::calendarSystem() const
{
  return mCalSys;
}

void CalPrintPluginBase::setCalendarSystem( const KCalendarSystem *calsys )
{
  mCalSys = calsys;
}

int CalPrintPluginBase::headerHeight() const
{
  if ( mHeaderHeight >= 0 ) {
    return mHeaderHeight;
  } else if ( orientation() == QPrinter::Portrait ) {
    return PORTRAIT_HEADER_HEIGHT;
  } else {
    return LANDSCAPE_HEADER_HEIGHT;
  }
}

void CalPrintPluginBase::setHeaderHeight( const int height )
{
  mHeaderHeight = height;
}

int CalPrintPluginBase::subHeaderHeight() const
{
  return mSubHeaderHeight;
}

void CalPrintPluginBase::setSubHeaderHeight( const int height )
{
  mSubHeaderHeight = height;
}

int CalPrintPluginBase::margin() const
{
  return mMargin;
}

void CalPrintPluginBase::setMargin( const int margin )
{
  mMargin = margin;
}

int CalPrintPluginBase::padding() const
{
  return mPadding;
}

void CalPrintPluginBase::setPadding( const int padding )
{
  mPadding = padding;
}

int CalPrintPluginBase::borderWidth() const
{
  return mBorder;
}

void CalPrintPluginBase::setBorderWidth( const int borderwidth )
{
  mBorder = borderwidth;
}

void CalPrintPluginBase::drawBox( QPainter &p, int linewidth, const QRect &rect )
{
  QPen pen( p.pen() );
  QPen oldpen( pen );
  pen.setWidth( linewidth );
  p.setPen( pen );
  p.drawRect( rect );
  p.setPen( oldpen );
}

void CalPrintPluginBase::drawShadedBox( QPainter &p, int linewidth,
                                        const QBrush &brush, const QRect &rect )
{
  QBrush oldbrush( p.brush() );
  p.setBrush( brush );
  drawBox( p, linewidth, rect );
  p.setBrush( oldbrush );
}

void CalPrintPluginBase::printEventString( QPainter &p, const QRect &box,
                                           const QString &str, int flags )
{
  QRect newbox( box );
  newbox.adjust( 3, 1, -1, -1 );
  p.drawText( newbox, ( flags == -1 ) ?
              ( Qt::AlignTop | Qt::AlignJustify | Qt::BreakAnywhere ) : flags, str );
}

void CalPrintPluginBase::showEventBox( QPainter &p, const QRect &box,
                                       Incidence *incidence, const QString &str,
                                       int flags )
{
  QPen oldpen( p.pen() );
  QBrush oldbrush( p.brush() );
  QColor bgColor( categoryBgColor( incidence ) );
  if ( mUseColors & bgColor.isValid() ) {
    p.setBrush( bgColor );
  } else {
    p.setBrush( QColor( 232, 232, 232 ) );
  }
  drawBox( p, EVENT_BORDER_WIDTH, box );

  if ( mUseColors && bgColor.isValid() ) {
    p.setPen( textColor( bgColor ) );
  }
  printEventString( p, box, str, flags );
  p.setPen( oldpen );
  p.setBrush( oldbrush );
}

void CalPrintPluginBase::drawSubHeaderBox( QPainter &p, const QString &str, const QRect &box )
{
  drawShadedBox( p, BOX_BORDER_WIDTH, QColor( 232, 232, 232 ), box );
  QFont oldfont( p.font() );
  p.setFont( QFont( "sans-serif", 10, QFont::Bold ) );
  p.drawText( box, Qt::AlignCenter | Qt::AlignVCenter, str );
  p.setFont( oldfont );
}

void CalPrintPluginBase::drawVerticalBox( QPainter &p, const QRect &box, const QString &str )
{
  p.save();
  p.rotate( -90 );
  QRect rotatedBox( -box.top()-box.height(), box.left(), box.height(), box.width() );
  showEventBox( p, rotatedBox, 0, str, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine );

  p.restore();
}

/*
 * Return value: If expand, bottom of the printed box, otherwise vertical end
 * of the printed contents inside the box.
 */
int CalPrintPluginBase::drawBoxWithCaption( QPainter &p, const QRect &allbox,
                                            const QString &caption,
                                            const QString &contents,
                                            bool sameLine, bool expand,
                                            const QFont &captionFont,
                                            const QFont &textFont )
{
  QFont oldFont( p.font() );
//   QFont captionFont( "sans-serif", 11, QFont::Bold );
//   QFont textFont( "sans-serif", 11, QFont::Normal );
//   QFont captionFont( "Tahoma", 11, QFont::Bold );
//   QFont textFont( "Tahoma", 11, QFont::Normal );

  QRect box( allbox );

  // Bounding rectangle for caption, single-line, clip on the right
  QRect captionBox( box.left() + padding(), box.top() + padding(), 0, 0 );
  p.setFont( captionFont );
  captionBox = p.boundingRect( captionBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine, caption );
  p.setFont( oldFont );
  if ( captionBox.right() > box.right() ) {
    captionBox.setRight( box.right() );
  }
  if ( expand && captionBox.bottom() + padding() > box.bottom() ) {
    box.setBottom( captionBox.bottom() + padding() );
  }

  // Bounding rectangle for the contents (if any), word break, clip on the bottom
  QRect textBox( captionBox );
  if ( !contents.isEmpty() ) {
    if ( sameLine ) {
      textBox.setLeft( captionBox.right() + padding() );
    } else {
      textBox.setTop( captionBox.bottom() + padding() );
    }
    textBox.setRight( box.right() );
    textBox.setHeight( 0 );
    p.setFont( textFont );
    textBox = p.boundingRect( textBox, Qt::WordBreak | Qt::AlignTop | Qt::AlignLeft, contents );
    p.setFont( oldFont );
    if ( textBox.bottom() + padding() > box.bottom() ) {
      if ( expand ) {
        box.setBottom( textBox.bottom() + padding() );
      } else {
        textBox.setBottom( box.bottom() );
      }
    }
  }

  drawBox( p, BOX_BORDER_WIDTH, box );
  p.setFont( captionFont );
  p.drawText( captionBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine, caption );
  if ( !contents.isEmpty() ) {
    p.setFont( textFont );
    p.drawText( textBox, Qt::WordBreak | Qt::AlignTop | Qt::AlignLeft, contents );
  }
  p.setFont( oldFont );

  if ( expand ) {
    return box.bottom();
  } else {
    return textBox.bottom();
  }
}

int CalPrintPluginBase::drawHeader( QPainter &p, const QString &title,
    const QDate &month1, const QDate &month2, const QRect &allbox, bool expand )
{
  // print previous month for month view, print current for to-do, day and week
  int smallMonthWidth = ( allbox.width() / 4 ) - 10;
  if ( smallMonthWidth > 100 ) {
    smallMonthWidth = 100;
  }

  int right = allbox.right();
  if ( month1.isValid() ) {
    right -= ( 20 + smallMonthWidth );
  }
  if ( month2.isValid() ) {
    right -= ( 20 + smallMonthWidth );
  }
  QRect box( allbox );
  QRect textRect( allbox );
  textRect.adjust( 5, 0, 0, 0 );
  textRect.setRight( right );

  QFont oldFont( p.font() );
  QFont newFont( "sans-serif", ( textRect.height() < 60 ) ? 16 : 18, QFont::Bold );
  if ( expand ) {
    p.setFont( newFont );
    QRect boundingR =
      p.boundingRect( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak, title );
    p.setFont( oldFont );
    int h = boundingR.height();
    if ( h > allbox.height() ) {
      box.setHeight( h );
      textRect.setHeight( h );
    }
  }

  drawShadedBox( p, BOX_BORDER_WIDTH, QColor( 232, 232, 232 ), box );

  QRect monthbox( box.right()-10-smallMonthWidth, box.top(), smallMonthWidth, box.height() );
  if ( month2.isValid() ) {
    drawSmallMonth( p, QDate( month2.year(), month2.month(), 1 ), monthbox );
    monthbox.moveLeft( 20 + smallMonthWidth );
  }
  if ( month1.isValid() ) {
    drawSmallMonth( p, QDate( month1.year(), month1.month(), 1 ), monthbox );
    monthbox.moveLeft( 20 + smallMonthWidth );
  }

  // Set the margins
  p.setFont( newFont );
  p.drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak, title );
  p.setFont( oldFont );

  return textRect.bottom();
}

void CalPrintPluginBase::drawSmallMonth( QPainter &p, const QDate &qd, const QRect &box )
{
  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  int month = qd.month();
  QDate monthDate( QDate( qd.year(), qd.month(), 1 ) );
  // correct begin of week
  QDate monthDate2( monthDate.addDays( -weekdayCol ) );

  double cellWidth = double( box.width() ) / double( 7 );
  int rownr = 3 + ( qd.daysInMonth() + weekdayCol - 1 ) / 7;
  // 3 Pixel after month name, 2 after day names, 1 after the calendar
  double cellHeight = ( box.height() - 5 ) / rownr;
  QFont oldFont( p.font() );
  p.setFont( QFont( "sans-serif", int(cellHeight-1), QFont::Normal ) );

  // draw the title
  if ( mCalSys ) {
    QRect titleBox( box );
    titleBox.setHeight( int( cellHeight + 1 ) );
    p.drawText( titleBox, Qt::AlignTop | Qt::AlignHCenter, mCalSys->monthName( qd ) );
  }

  // draw days of week
  QRect wdayBox( box );
  wdayBox.setTop( int( box.top() + 3 + cellHeight ) );
  wdayBox.setHeight( int( 2 * cellHeight ) - int( cellHeight ) );

  if ( mCalSys ) {
    for ( int col = 0; col < 7; ++col ) {
      QString tmpStr = mCalSys->weekDayName( monthDate2 )[0].toUpper();
      wdayBox.setLeft( int( box.left() + col * cellWidth ) );
      wdayBox.setRight( int( box.left() + ( col + 1 ) * cellWidth ) );
      p.drawText( wdayBox, Qt::AlignCenter, tmpStr );
      monthDate2 = monthDate2.addDays( 1 );
    }
  }

  // draw separator line
  int calStartY = wdayBox.bottom() + 2;
  p.drawLine( box.left(), calStartY, box.right(), calStartY );
  monthDate = monthDate.addDays( -weekdayCol );

  for ( int row = 0; row < (rownr-2); row++ ) {
    for ( int col = 0; col < 7; col++ ) {
      if ( monthDate.month() == month ) {
        QRect dayRect( int( box.left() + col * cellWidth ),
                       int( calStartY + row * cellHeight ), 0, 0 );
        dayRect.setRight( int( box.left() + ( col + 1 ) * cellWidth ) );
        dayRect.setBottom( int( calStartY + ( row + 1 ) * cellHeight ) );
        p.drawText( dayRect, Qt::AlignCenter, QString::number( monthDate.day() ) );
      }
      monthDate = monthDate.addDays(1);
    }
  }
  p.setFont( oldFont );
}

/*
 * This routine draws a header box over the main part of the calendar
 * containing the days of the week.
 */
void CalPrintPluginBase::drawDaysOfWeek( QPainter &p,
                                         const QDate &fromDate,
                                         const QDate &toDate, const QRect &box )
{
  double cellWidth = double( box.width() ) / double( fromDate.daysTo( toDate ) + 1 );
  QDate cellDate( fromDate );
  QRect dateBox( box );
  int i = 0;

  while ( cellDate <= toDate ) {
    dateBox.setLeft( box.left() + int( i * cellWidth ) );
    dateBox.setRight( box.left() + int( ( i + 1 ) * cellWidth ) );
    drawDaysOfWeekBox( p, cellDate, dateBox );
    cellDate = cellDate.addDays( 1 );
    i++;
  }
}

void CalPrintPluginBase::drawDaysOfWeekBox( QPainter &p, const QDate &qd, const QRect &box )
{
  drawSubHeaderBox( p, ( mCalSys ) ? ( mCalSys->weekDayName( qd ) ) : QString(), box );
}

void CalPrintPluginBase::drawTimeLine( QPainter &p, const QTime &fromTime,
                                       const QTime &toTime, const QRect &box )
{
  drawBox( p, BOX_BORDER_WIDTH, box );

  int totalsecs = fromTime.secsTo( toTime );
  float minlen = (float)box.height() * 60. / (float)totalsecs;
  float cellHeight = ( 60. * (float)minlen );
  float currY = box.top();
  // TODO: Don't use half of the width, but less, for the minutes!
  int xcenter = box.left() + box.width() / 2;

  QTime curTime( fromTime );
  QTime endTime( toTime );
  if ( fromTime.minute() > 30 ) {
    curTime = QTime( fromTime.hour()+1, 0, 0 );
  } else if ( fromTime.minute() > 0 ) {
    curTime = QTime( fromTime.hour(), 30, 0 );
    float yy = currY + minlen * (float)fromTime.secsTo( curTime ) / 60.;
    p.drawLine( xcenter, (int)yy, box.right(), (int)yy );
    curTime = QTime( fromTime.hour() + 1, 0, 0 );
  }
  currY += ( float( fromTime.secsTo( curTime ) * minlen ) / 60. );

  while ( curTime < endTime ) {
    p.drawLine( box.left(), (int)currY, box.right(), (int)currY );
    int newY = (int)( currY + cellHeight / 2. );
    QString numStr;
    if ( newY < box.bottom() ) {
      QFont oldFont( p.font() );
      // draw the time:
      if ( !KGlobal::locale()->use12Clock() ) {
        p.drawLine( xcenter, (int)newY, box.right(), (int)newY );
        numStr.setNum( curTime.hour() );
        if ( cellHeight > 30 ) {
          p.setFont( QFont( "sans-serif", 14, QFont::Bold ) );
        } else {
          p.setFont( QFont( "sans-serif", 12, QFont::Bold ) );
        }
        p.drawText( box.left() + 4, (int)currY + 2, box.width() / 2 - 2, (int)cellHeight,
                  Qt::AlignTop | Qt::AlignRight, numStr );
        p.setFont( QFont( "helvetica", 10, QFont::Normal ) );
        p.drawText( xcenter + 4, (int)currY+2, box.width() / 2 + 2, (int)( cellHeight / 2 ) - 3,
                  Qt::AlignTop | Qt::AlignLeft, "00" );
      } else {
        p.drawLine( box.left(), (int)newY, box.right(), (int)newY );
        QTime time( curTime.hour(), 0 );
        numStr = KGlobal::locale()->formatTime( time );
        if ( box.width() < 60 ) {
          p.setFont( QFont( "sans-serif", 7, QFont::Bold ) ); // for weekprint
        } else {
          p.setFont( QFont( "sans-serif", 12, QFont::Bold ) ); // for dayprint
        }
        p.drawText( box.left() + 2, (int)currY + 2, box.width() - 4, (int)cellHeight / 2 - 3,
                    Qt::AlignTop|Qt::AlignLeft, numStr );
      }
      currY += cellHeight;
      p.setFont( oldFont );
    } // enough space for half-hour line and time
    if ( curTime.secsTo( endTime ) > 3600 ) {
      curTime = curTime.addSecs( 3600 );
    } else {
      curTime = endTime;
    }
  }
}

/**
  prints the all-day box for the agenda print view. if expandable is set,
  height is the cell height of a single cell, and the returned height will
  be the total height used for the all-day events. If !expandable, only one
  cell will be used, and multiple events are concatenated using ", ".
*/
int CalPrintPluginBase::drawAllDayBox( QPainter &p, Event::List &eventList,
                                       const QDate &qd, bool expandable,
                                       const QRect &box )
{
  Event::List::Iterator it, itold;
  int offset = box.top();
  QString multiDayStr;

  Event *hd = holiday( qd );
  if ( hd ) {
    eventList.prepend( hd );
  }

  it = eventList.begin();
  Event *currEvent = 0;
  // First, print the all-day events
  while ( it != eventList.end() ) {
    currEvent = *it;
    itold = it;
    ++it;
    if ( currEvent && currEvent->allDay() ) {
      // set the colors according to the categories
      if ( expandable ) {
        QRect eventBox( box );
        eventBox.setTop( offset );
        showEventBox( p, eventBox, currEvent, currEvent->summary() );
        offset += box.height();
      } else {
        if ( !multiDayStr.isEmpty() ) {
          multiDayStr += ", ";
        }
        multiDayStr += currEvent->summary();
      }
      eventList.erase( itold );
    }
  }
  if ( hd ) {
    delete hd;
  }

  int ret = box.height();
  QRect eventBox( box );
  if ( !expandable ) {
    if ( !multiDayStr.isEmpty() ) {
      drawShadedBox( p, BOX_BORDER_WIDTH, QColor( 128, 128, 128 ), eventBox );
      printEventString( p, eventBox, multiDayStr );
    } else {
      drawBox( p, BOX_BORDER_WIDTH, eventBox );
    }
  } else {
    ret = offset - box.top();
    eventBox.setBottom( ret );
    drawBox( p, BOX_BORDER_WIDTH, eventBox );
  }
  return ret;
}

void CalPrintPluginBase::drawAgendaDayBox( QPainter &p, Event::List &events,
                                           const QDate &qd, bool expandable,
                                           QTime &fromTime, QTime &toTime,
                                           const QRect &oldbox )
{
  if ( !isWorkingDay( qd ) ) {
    drawShadedBox( p, BOX_BORDER_WIDTH, QColor( 232, 232, 232 ), oldbox );
  } else {
    drawBox( p, BOX_BORDER_WIDTH, oldbox );
  }
  QRect box( oldbox );
  // Account for the border with and cut away that margin from the interior
//   box.setRight( box.right()-BOX_BORDER_WIDTH );

  Event *event;

  if ( expandable ) {
    // Adapt start/end times to include complete events
    Event::List::ConstIterator it;
    for ( it = events.begin(); it != events.end(); ++it ) {
      event = *it;
      if ( event->dtStart().time() < fromTime ) {
        fromTime = event->dtStart().time();
      }
      if ( event->dtEnd().time() > toTime ) {
        toTime = event->dtEnd().time();
      }
    }
  }

  // calculate the height of a cell and of a minute
  int totalsecs = fromTime.secsTo( toTime );
  float minlen = box.height() * 60. / totalsecs;
  float cellHeight = 60. * minlen;
  float currY = box.top();

  // print grid:
  QTime curTime( QTime( fromTime.hour(), 0, 0 ) );
  currY += fromTime.secsTo( curTime ) * minlen / 60;

  while ( curTime < toTime && curTime.isValid() ) {
    if ( currY > box.top() ) {
      p.drawLine( box.left(), int( currY ), box.right(), int( currY ) );
    }
    currY += cellHeight / 2;
    if ( ( currY > box.top() ) && ( currY < box.bottom() ) ) {
      // enough space for half-hour line
      QPen oldPen( p.pen() );
      p.setPen( QColor( 192, 192, 192 ) );
      p.drawLine( box.left(), int( currY ), box.right(), int( currY ) );
      p.setPen( oldPen );
    }
    if ( curTime.secsTo( toTime ) > 3600 ) {
      curTime = curTime.addSecs( 3600 );
    } else {
      curTime = toTime;
    }
    currY += cellHeight / 2;
  }

  KDateTime startPrintDate = KDateTime( qd, fromTime );
  KDateTime endPrintDate = KDateTime( qd, toTime );

  // Calculate horizontal positions and widths of events taking into account
  // overlapping events

  QList<KOrg::CellItem *> cells;

  Event::List::ConstIterator itEvents;
  for ( itEvents = events.begin(); itEvents != events.end(); ++itEvents ) {
    QList<KDateTime> times = (*itEvents)->startDateTimesForDate( qd );
    for ( QList<KDateTime>::ConstIterator it = times.begin();
          it != times.end(); ++it ) {
      cells.append( new PrintCellItem( *itEvents, (*it), (*itEvents)->endDateForStart( *it ) ) );
    }
  }

  QListIterator<KOrg::CellItem *> it1( cells );
  while ( it1.hasNext() ) {
    KOrg::CellItem *placeItem = it1.next();
    KOrg::CellItem::placeItem( cells, placeItem );
  }

  QListIterator<KOrg::CellItem *> it2( cells );
  while ( it2.hasNext() ) {
    PrintCellItem *placeItem = static_cast<PrintCellItem *>( it2.next() );
    drawAgendaItem( placeItem, p, startPrintDate, endPrintDate, minlen, box );
  }
}

void CalPrintPluginBase::drawAgendaItem( PrintCellItem *item, QPainter &p,
                                   const KDateTime &startPrintDate,
                                   const KDateTime &endPrintDate,
                                   float minlen, const QRect &box )
{
  Event *event = item->event();

  // start/end of print area for event
  KDateTime startTime = item->start();
  KDateTime endTime = item->end();
  if ( ( startTime < endPrintDate && endTime > startPrintDate ) ||
       ( endTime > startPrintDate && startTime < endPrintDate ) ) {
    if ( startTime < startPrintDate ) {
      startTime = startPrintDate;
    }
    if ( endTime > endPrintDate ) {
      endTime = endPrintDate;
    }
    int currentWidth = box.width() / item->subCells();
    int currentX = box.left() + item->subCell() * currentWidth;
    int currentYPos =
      int( box.top() + startPrintDate.secsTo( startTime ) * minlen / 60. );
    int currentHeight =
      int( box.top() + startPrintDate.secsTo( endTime ) * minlen / 60. ) - currentYPos;

    QRect eventBox( currentX, currentYPos, currentWidth, currentHeight );
    QString str = i18nc( "starttime - endtime summary, location",
                         "%1-%2 %3, %4",
                         KGlobal::locale()->formatTime( startTime.time() ),
                         KGlobal::locale()->formatTime( endTime.time() ),
                         event->summary(),
                         event->location() );
    showEventBox( p, eventBox, event, str );
  }
}

void CalPrintPluginBase::drawDayBox( QPainter &p, const QDate &qd,
    const QRect &box,
    bool fullDate, bool printRecurDaily, bool printRecurWeekly )
{
  QString dayNumStr;
  QString ampm;
  const KLocale *local = KGlobal::locale();

  if ( fullDate && mCalSys ) {
    dayNumStr = i18nc( "weekday, shortmonthname daynumber",
                       "%1, %2 <numid>%3</numid>",
                       mCalSys->weekDayName( qd ),
                       mCalSys->monthName( qd, KCalendarSystem::ShortName ),
                       qd.day() );
  } else {
    dayNumStr = QString::number( qd.day() );
  }

  QRect subHeaderBox( box );
  subHeaderBox.setHeight( mSubHeaderHeight );
  drawShadedBox( p, BOX_BORDER_WIDTH, p.background(), box );
  drawShadedBox( p, 0, QColor( 232, 232, 232 ), subHeaderBox );
  drawBox( p, BOX_BORDER_WIDTH, box );
  QString hstring( holidayString( qd ) );
  QFont oldFont( p.font() );

  QRect headerTextBox( subHeaderBox );
  headerTextBox.setLeft( subHeaderBox.left()+5 );
  headerTextBox.setRight( subHeaderBox.right()-5 );
  if ( !hstring.isEmpty() ) {
    p.setFont( QFont( "sans-serif", 8, QFont::Bold, true ) );

    p.drawText( headerTextBox, Qt::AlignLeft | Qt::AlignVCenter, hstring );
  }
  p.setFont( QFont( "sans-serif", 10, QFont::Bold ) );
  p.drawText( headerTextBox, Qt::AlignRight | Qt::AlignVCenter, dayNumStr );

  Event::List eventList = mCalendar->events( qd, KPIM::KPimPrefs::timeSpec(),
                                             EventSortStartDate,
                                             SortDirectionAscending );
  QString text;
  p.setFont( QFont( "sans-serif", 8 ) );

  int textY = mSubHeaderHeight + 3; // gives the relative y-coord of the next printed entry
  Event::List::ConstIterator it;

  for ( it = eventList.begin(); it != eventList.end() && textY<box.height(); ++it ) {
    Event *currEvent = *it;
    if ( ( !printRecurDaily  && currEvent->recurrenceType() == Recurrence::rDaily ) ||
         ( !printRecurWeekly && currEvent->recurrenceType() == Recurrence::rWeekly ) ) {
      continue;
    }
    if ( currEvent->allDay() || currEvent->isMultiDay() ) {
      text = "";
    } else {
      text = local->formatTime( currEvent->dtStart().time() );
    }
    QString str;
    if ( !currEvent->location().isEmpty() ) {
      str = i18nc( "summary, location", "%1, %2",
                   currEvent->summary(), currEvent->location() );
    } else {
      str = currEvent->summary();
    }
    drawIncidence( p, box, text, str, textY );
  }

  if ( textY < box.height() ) {
    Todo::List todos = mCalendar->todos( qd );
    Todo::List::ConstIterator it2;
    for ( it2 = todos.begin(); it2 != todos.end() && textY<box.height(); ++it2 ) {
      Todo *todo = *it2;
      if ( ( !printRecurDaily  && todo->recurrenceType() == Recurrence::rDaily ) ||
           ( !printRecurWeekly && todo->recurrenceType() == Recurrence::rWeekly ) ) {
        continue;
      }
      if ( todo->hasDueDate() && !todo->allDay() ) {
        text += KGlobal::locale()->formatTime( todo->dtDue().time() ) + ' ';
      } else {
        text = "";
      }
      QString str;
      if ( !todo->location().isEmpty() ) {
        str = i18nc( "summary, location", "%1, %2",
                     todo->summary(), todo->location() );
      } else {
        str = todo->summary();
      }
      drawIncidence( p, box, text, i18n( "To-do: %1", str ), textY );
    }
  }

  p.setFont( oldFont );
}

void CalPrintPluginBase::drawIncidence( QPainter &p, const QRect &dayBox,
                                        const QString &time,
                                        const QString &summary, int &textY )
{
  kDebug() << "summary =" << summary;

  int flags = Qt::AlignLeft;
  QFontMetrics fm = p.fontMetrics();
  QRect timeBound = p.boundingRect( dayBox.x() + 5, dayBox.y() + textY,
                                    dayBox.width() - 10, fm.lineSpacing(),
                                    flags, time );
  p.drawText( timeBound, flags, time );

  int summaryWidth = time.isEmpty() ? 0 : timeBound.width() + 4;
  QRect summaryBound = QRect( dayBox.x() + 5 + summaryWidth, dayBox.y() + textY,
                              dayBox.width() - summaryWidth -5, dayBox.height() );

  KWordWrap *ww = KWordWrap::formatText( fm, summaryBound, flags, summary );
  ww->drawText( &p, dayBox.x() + 5 + summaryWidth, dayBox.y() + textY, flags );

  textY += ww->boundingRect().height();

  delete ww;
}

void CalPrintPluginBase::drawWeek( QPainter &p, const QDate &qd, const QRect &box )
{
  QDate weekDate = qd;
  bool portrait = ( box.height() > box.width() );
  int cellWidth, cellHeight;
  int vcells;
  if ( portrait ) {
    cellWidth = box.width() / 2;
    vcells=3;
  } else {
    cellWidth = box.width() / 6;
    vcells=1;
  }
  cellHeight = box.height() / vcells;

  // correct begin of week
  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  weekDate = qd.addDays( -weekdayCol );

  for ( int i = 0; i < 7; i++, weekDate = weekDate.addDays(1) ) {
    // Saturday and sunday share a cell, so we have to special-case sunday
    int hpos = ( ( i < 6 ) ? i : ( i - 1 ) ) / vcells;
    int vpos = ( ( i < 6 ) ? i : ( i - 1 ) ) % vcells;
    QRect dayBox(
      box.left() + cellWidth * hpos,
      box.top() + cellHeight * vpos + ( ( i == 6 ) ? ( cellHeight / 2 ) : 0 ),
      cellWidth, ( i < 5 ) ? ( cellHeight ) : ( cellHeight / 2 ) );
    drawDayBox( p, weekDate, dayBox, true );
  } // for i through all weekdays
}

void CalPrintPluginBase::drawTimeTable( QPainter &p,
                                        const QDate &fromDate,
                                        const QDate &toDate,
                                        QTime &fromTime, QTime &toTime,
                                        const QRect &box )
{
  // timeline is 1 hour:
  int alldayHeight = (int)( 3600. * box.height() / ( fromTime.secsTo( toTime ) + 3600. ) );
  int timelineWidth = TIMELINE_WIDTH;

  QRect dowBox( box );
  dowBox.setLeft( box.left() + timelineWidth );
  dowBox.setHeight( mSubHeaderHeight );
  drawDaysOfWeek( p, fromDate, toDate, dowBox );

  QRect tlBox( box );
  tlBox.setWidth( timelineWidth );
  tlBox.setTop( dowBox.bottom() + BOX_BORDER_WIDTH + alldayHeight );
  drawTimeLine( p, fromTime, toTime, tlBox );

  // draw each day
  QDate curDate(fromDate);
  KDateTime::Spec timeSpec = KPIM::KPimPrefs::timeSpec();
  int i=0;
  double cellWidth = double( dowBox.width() ) / double( fromDate.daysTo( toDate ) + 1 );
  while ( curDate <= toDate ) {
    QRect allDayBox( dowBox.left()+int( i * cellWidth ), dowBox.bottom() + BOX_BORDER_WIDTH,
                     int( ( i + 1 ) * cellWidth ) - int( i * cellWidth ), alldayHeight );
    QRect dayBox( allDayBox );
    dayBox.setTop( tlBox.top() );
    dayBox.setBottom( box.bottom() );
    Event::List eventList = mCalendar->events( curDate, timeSpec,
                                               EventSortStartDate,
                                               SortDirectionAscending );
    alldayHeight = drawAllDayBox( p, eventList, curDate, false, allDayBox );
    drawAgendaDayBox( p, eventList, curDate, false, fromTime, toTime, dayBox );
    i++;
    curDate=curDate.addDays(1);
  }
}

class MonthEventStruct
{
  public:
    MonthEventStruct() : event(0) {}
    MonthEventStruct( const KDateTime &s, const KDateTime &e, Event *ev )
    {
      event = ev;
      start = s;
      end = e;
      if ( event->allDay() ) {
        start = KDateTime( start.date(), QTime( 0, 0, 0 ) );
        end = KDateTime( end.date().addDays(1), QTime( 0, 0, 0 ) ).addSecs(-1);
      }
    }
    bool operator < ( const MonthEventStruct &mes ) { return start < mes.start; }
    KDateTime start;
    KDateTime end;
    Event *event;
};

void CalPrintPluginBase::drawMonth( QPainter &p, const QDate &dt,
                                    const QRect &box, int maxdays,
                                    int subDailyFlags, int holidaysFlags )
{
  const KCalendarSystem *calsys = calendarSystem();
  QRect subheaderBox( box );
  subheaderBox.setHeight( subHeaderHeight() );
  QRect borderBox( box );
  borderBox.setTop( subheaderBox.bottom() + 1 );
  drawSubHeaderBox( p, calsys->monthName( dt ), subheaderBox );
  // correct for half the border width
  int correction = ( BOX_BORDER_WIDTH/*-1*/ ) / 2;
  QRect daysBox( borderBox );
  daysBox.adjust( correction, correction, -correction, -correction );

  int daysinmonth = calsys->daysInMonth( dt );
  if ( maxdays <= 0 ) {
    maxdays = daysinmonth;
  }

  int d;
  float dayheight = float( daysBox.height() ) / float( maxdays );

  QColor holidayColor( 240, 240, 240 );
  QColor workdayColor( 255, 255, 255 );
  int dayNrWidth = p.fontMetrics().width( "99" );

  // Fill the remaining space (if a month has less days than others) with a crossed-out pattern
  if ( daysinmonth<maxdays ) {
    QRect dayBox( box.left(), daysBox.top() + round( dayheight * daysinmonth ), box.width(), 0 );
    dayBox.setBottom( daysBox.bottom() );
    p.fillRect( dayBox, Qt::DiagCrossPattern );
  }
  // Backgrounded boxes for each day, plus day numbers
  QBrush oldbrush( p.brush() );
  for ( d = 0; d < daysinmonth; ++d ) {
    QDate day;
    calsys->setYMD( day, dt.year(), dt.month(), d+1 );
    QRect dayBox(
      daysBox.left()/*+rand()%50*/,
      daysBox.top() + round( dayheight * d ), daysBox.width()/*-rand()%50*/, 0 );
    // FIXME: When using a border width of 0 for event boxes,
    // don't let the rectangles overlap, i.e. subtract 1 from the top or bottom!
    dayBox.setBottom( daysBox.top() + round( dayheight * ( d + 1 ) ) - 1 );

    p.setBrush( isWorkingDay( day ) ? workdayColor : holidayColor );
    p.drawRect( dayBox );
    QRect dateBox( dayBox );
    dateBox.setWidth( dayNrWidth + 3 );
    p.drawText( dateBox,
                Qt::AlignRight | Qt::AlignVCenter | Qt::SingleLine,
                QString::number( d + 1 ) );
  }
  p.setBrush( oldbrush );
  int xstartcont = box.left() + dayNrWidth + 5;

  QDate start, end;
  calsys->setYMD( start, dt.year(), dt.month(), 1 );
  end = calsys->addMonths( start, 1 );
  end = calsys->addDays( end, -1 );

  Event::List events = mCalendar->events( start, end );
  QMap<int, QStringList> textEvents;
  QList<KOrg::CellItem *> timeboxItems;

  // 1) For multi-day events, show boxes spanning several cells, use CellItem
  //    print the summary vertically
  // 2) For sub-day events, print the concated summaries into the remaining
  //    space of the box (optional, depending on the given flags)
  // 3) Draw some kind of timeline showing free and busy times

  // Holidays
  Event::List holidays;
  for ( QDate d( start ); d <= end; d = d.addDays(1) ) {
    Event *e = holiday( d );
    if ( e ) {
      holidays.append( e );
      if ( holidaysFlags & TimeBoxes ) {
        timeboxItems.append( new PrintCellItem( e, KDateTime( d, QTime( 0, 0, 0 ) ),
                                                KDateTime( d.addDays(1), QTime( 0, 0, 0 ) ) ) );
      }
      if ( holidaysFlags & Text ) {
        textEvents[ d.day() ] << e->summary();
      }
    }
  }

  QList<MonthEventStruct> monthentries;

  //FIXME: KOPrefs::instance()->timeSpec()?
  KDateTime::Spec timeSpec = KPIM::KPimPrefs::timeSpec();
  for ( Event::List::ConstIterator evit = events.begin(); evit != events.end(); ++evit ) {
    Event *e = (*evit);
    if ( !e ) {
      continue;
    }
    if ( e->recurs() ) {
      if ( e->recursOn( start, timeSpec ) ) {
        // This occurrence has possibly started before the beginning of the
        // month, so obtain the start date before the beginning of the month
        QList<KDateTime> starttimes = e->startDateTimesForDate( start );
        QList<KDateTime>::ConstIterator it = starttimes.begin();
        for ( ; it != starttimes.end(); ++it ) {
          monthentries.append( MonthEventStruct( *it, e->endDateForStart( *it ), e ) );
        }
      }
      // Loop through all remaining days of the month and check if the event
      // begins on that day (don't use Event::recursOn, as that will
      // also return events that have started earlier. These start dates
      // however, have already been treated!
      Recurrence *recur = e->recurrence();
      QDate d1( start.addDays(1) );
      while ( d1 <= end ) {
        if ( recur->recursOn( d1, timeSpec ) ) {
          TimeList times( recur->recurTimesOn( d1, timeSpec ) );
          for ( TimeList::ConstIterator it = times.begin();
                it != times.end(); ++it ) {
            KDateTime d1start( d1, *it, timeSpec );
            monthentries.append( MonthEventStruct( d1start, e->endDateForStart( d1start ), e ) );
          }
        }
        d1 = d1.addDays(1);
      }
    } else {
      monthentries.append( MonthEventStruct( e->dtStart(), e->dtEnd(), e ) );
    }
  }
#ifdef __GNUC__
#warning todo: to port the month entries sorting
#endif
//  qSort( monthentries.begin(), monthentries.end() );

  QList<MonthEventStruct>::ConstIterator mit = monthentries.begin();
  KDateTime endofmonth( end, QTime( 0, 0, 0 ) );
  endofmonth = endofmonth.addDays(1);
  for ( ; mit != monthentries.end(); ++mit ) {
    if ( (*mit).start.date() == (*mit).end.date() ) {
      // Show also single-day events as time line boxes
      if ( subDailyFlags & TimeBoxes ) {
        timeboxItems.append( new PrintCellItem( (*mit).event, (*mit).start, (*mit).end ) );
      }
      // Show as text in the box
      if ( subDailyFlags & Text ) {
        textEvents[ (*mit).start.date().day() ] << (*mit).event->summary();
      }
    } else {
      // Multi-day events are always shown as time line boxes
      KDateTime thisstart( (*mit).start );
      KDateTime thisend( (*mit).end );
      if ( thisstart.date() < start ) {
        thisstart.setDate( start );
      }
      if ( thisend > endofmonth ) {
        thisend = endofmonth;
      }
      timeboxItems.append( new PrintCellItem( (*mit).event, thisstart, thisend ) );
    }
  }

  // For Multi-day events, line them up nicely so that the boxes don't overlap
  QListIterator<KOrg::CellItem *> it1( timeboxItems );
  while ( it1.hasNext() ) {
    KOrg::CellItem *placeItem = it1.next();
    KOrg::CellItem::placeItem( timeboxItems, placeItem );
  }
  KDateTime starttime( start, QTime( 0, 0, 0 ) );
  int newxstartcont = xstartcont;

  QFont oldfont( p.font() );
  p.setFont( QFont( "sans-serif", 7 ) );
  while ( it1.hasNext() ) {
    PrintCellItem *placeItem = static_cast<PrintCellItem *>( it1.next() );
    int minsToStart = starttime.secsTo( placeItem->start() ) / 60;
    int minsToEnd = starttime.secsTo( placeItem->end() ) / 60;

    QRect eventBox(
      xstartcont + placeItem->subCell() * 17,
      daysBox.top() + round(
        double( minsToStart * daysBox.height() ) / double( maxdays * 24 * 60 ) ), 14, 0 );
    eventBox.setBottom(
      daysBox.top() + round(
        double( minsToEnd * daysBox.height() ) / double( maxdays * 24 * 60 ) ) );
    drawVerticalBox( p, eventBox, placeItem->event()->summary() );
    newxstartcont = qMax( newxstartcont, eventBox.right() );
  }
  xstartcont = newxstartcont;

  // For Single-day events, simply print their summaries into the remaining
  // space of the day's cell
  for ( int d=0; d<daysinmonth; ++d ) {
    QStringList dayEvents( textEvents[d+1] );
    QString txt = dayEvents.join( ", " );
    QRect dayBox( xstartcont, daysBox.top() + round( dayheight * d ), 0, 0 );
    dayBox.setRight( box.right() );
    dayBox.setBottom( daysBox.top() + round( dayheight * ( d + 1 ) ) );
    printEventString( p, dayBox, txt, Qt::AlignTop | Qt::AlignLeft | Qt::BreakAnywhere );
  }
  p.setFont( oldfont );
  drawBox( p, BOX_BORDER_WIDTH, borderBox );
  p.restore();
}

void CalPrintPluginBase::drawMonthTable( QPainter &p, const QDate &qd,
                                         bool weeknumbers, bool recurDaily,
                                         bool recurWeekly, const QRect &box )
{
  int yoffset = mSubHeaderHeight;
  int xoffset = 0;
  QDate monthDate( QDate( qd.year(), qd.month(), 1 ) );
  QDate monthFirst( monthDate );
  QDate monthLast( monthDate.addMonths(1).addDays(-1) );

  int weekdayCol = weekdayColumn( monthDate.dayOfWeek() );
  monthDate = monthDate.addDays(-weekdayCol);

  if (weeknumbers) {
    xoffset += 14;
  }

  int rows = ( weekdayCol + qd.daysInMonth() - 1 ) / 7 + 1;
  double cellHeight = ( box.height() - yoffset ) / ( 1. * rows );
  double cellWidth = ( box.width() - xoffset ) / 7.;

  // Precalculate the grid...
  // rows is at most 6, so using 8 entries in the array is fine, too!
  int coledges[8], rowedges[8];
  for ( int i = 0; i <= 7; i++ ) {
    rowedges[i] = int( box.top() + yoffset + i * cellHeight );
    coledges[i] = int( box.left() + xoffset + i * cellWidth );
  }

  if ( weeknumbers ) {
    QFont oldFont( p.font() );
    QFont newFont( p.font() );
    newFont.setPointSize( 6 );
    p.setFont( newFont );
    QDate weekDate( monthDate );
    for ( int row = 0; row<rows; ++row ) {
      int calWeek = weekDate.weekNumber();
      QRect rc( box.left(), rowedges[row],
                coledges[0] - 3 - box.left(), rowedges[row + 1] - rowedges[row] );
      p.drawText( rc, Qt::AlignRight | Qt::AlignVCenter, QString::number( calWeek ) );
      weekDate = weekDate.addDays( 7 );
    }
    p.setFont( oldFont );
  }

  QRect daysOfWeekBox( box );
  daysOfWeekBox.setHeight( mSubHeaderHeight );
  daysOfWeekBox.setLeft( box.left() + xoffset );
  drawDaysOfWeek( p, monthDate, monthDate.addDays( 6 ), daysOfWeekBox );

  QColor back = p.background().color();
  bool darkbg = false;
  for ( int row = 0; row < rows; ++row ) {
    for ( int col = 0; col < 7; ++col ) {
      // show days from previous/next month with a grayed background
      if ( ( monthDate < monthFirst ) || ( monthDate > monthLast ) ) {
        p.setBackground( back.darker( 120 ) );
        darkbg = true;
      }
      QRect dayBox( coledges[col], rowedges[row],
                    coledges[col + 1] - coledges[col], rowedges[row + 1] - rowedges[row] );
      drawDayBox( p, monthDate, dayBox, false, recurDaily, recurWeekly );
      if ( darkbg ) {
        p.setBackground( back );
        darkbg = false;
      }
      monthDate = monthDate.addDays(1);
    }
  }
}

void CalPrintPluginBase::drawTodo( int &count, Todo *todo, QPainter &p,
                               TodoSortField sortField, SortDirection sortDir,
                               bool connectSubTodos, bool strikeoutCompleted,
                               bool desc, int posPriority, int posSummary,
                               int posDueDt, int posPercentComplete,
                               int level, int x, int &y, int width,
                               int pageHeight, const Todo::List &todoList,
                               TodoParentStart *r )
{
  QString outStr;
  const KLocale *local = KGlobal::locale();
  QRect rect;
  TodoParentStart startpt;
  // This list keeps all starting points of the parent to-dos so the connection
  // lines of the tree can easily be drawn (needed if a new page is started)
  static QList<TodoParentStart *> startPoints;
  if ( level < 1 ) {
    startPoints.clear();
  }

  // Compute the right hand side of the to-do box
  int rhs = posPercentComplete;
  if ( rhs < 0 ) {
    rhs = posDueDt; //not printing percent completed
  }
  if ( rhs < 0 ) {
    rhs = x + width;  //not printing due dates either
  }

  // size of to-do
  outStr=todo->summary();
  int left = posSummary + ( level * 10 );
  rect = p.boundingRect( left, y, ( rhs-left-5 ), -1, Qt::WordBreak, outStr );
  if ( !todo->description().isEmpty() && desc ) {
    outStr = todo->description();
    rect = p.boundingRect( left + 20, rect.bottom() + 5,
                           width - ( left + 10 - x ), -1, Qt::WordBreak, outStr );
  }
  // if too big make new page
  if ( rect.bottom() > pageHeight ) {
    // first draw the connection lines from parent to-dos:
    if ( level > 0 && connectSubTodos ) {
      TodoParentStart *rct;
      for ( int i = 0; i < startPoints.size(); ++i ) {
        rct = startPoints.at( i );
        int start;
        int center = rct->mRect.left() + ( rct->mRect.width() / 2 );
        int to = p.viewport().bottom();

        // draw either from start point of parent or from top of the page
        if ( rct->mSamePage ) {
          start = rct->mRect.bottom() + 1;
        } else {
          start = p.viewport().top();
        }
        p.drawLine( center, start, center, to );
        rct->mSamePage = false;
      }
    }
    y = 0;
    mPrinter->newPage();
  }

  // If this is a sub-to-do, r will not be 0, and we want the LH side
  // of the priority line up to the RH side of the parent to-do's priority
  bool showPriority = posPriority >= 0;
  int lhs = posPriority;
  if ( r ) {
    lhs = r->mRect.right() + 1;
  }

  outStr.setNum( todo->priority() );
  rect = p.boundingRect( lhs, y + 10, 5, -1, Qt::AlignCenter, outStr );
  // Make it a more reasonable size
  rect.setWidth( 18 );
  rect.setHeight( 18 );

  // Draw a checkbox
  p.setBrush( QBrush( Qt::NoBrush ) );
  p.drawRect( rect );
  if ( todo->isCompleted() ) {
    // cross out the rectangle for completed to-dos
    p.drawLine( rect.topLeft(), rect.bottomRight() );
    p.drawLine( rect.topRight(), rect.bottomLeft() );
  }
  lhs = rect.right() + 3;

  // Priority
  if ( todo->priority() > 0 && showPriority ) {
    p.drawText( rect, Qt::AlignCenter, outStr );
  }
  startpt.mRect = rect; //save for later

  // Connect the dots
  if ( r && level > 0 && connectSubTodos ) {
    int bottom;
    int center( r->mRect.left() + ( r->mRect.width() / 2 ) );
    if ( r->mSamePage ) {
      bottom = r->mRect.bottom() + 1;
    } else {
      bottom = 0;
    }
    int to( rect.top() + ( rect.height() / 2 ) );
    int endx( rect.left() );
    p.drawLine( center, bottom, center, to );
    p.drawLine( center, to, endx, to );
  }

  // summary
  outStr = todo->summary();
  rect = p.boundingRect( lhs, rect.top(), ( rhs - ( left + rect.width() + 5 ) ),
                         -1, Qt::WordBreak, outStr );

  QRect newrect;
  //FIXME: the following code prints underline rather than strikeout text
#if 0
  QFont f( p.font() );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    f.setStrikeOut( true );
    p.setFont( f );
  }
  p.drawText( rect, Qt::WordBreak, outStr, &newrect );
  f.setStrikeOut( false );
  p.setFont( f );
#endif
  //TODO: Remove this section when the code above is fixed
  p.drawText( rect, Qt::WordBreak, outStr, &newrect );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    // strike out the summary text if to-do is complete
    // Note: we tried to use a strike-out font and for unknown reasons the
    // result was underline instead of strike-out, so draw the lines ourselves.
    int delta = p.fontMetrics().lineSpacing();
    int lines = ( rect.height() / delta ) + 1;
    for ( int i=0; i<lines; i++ ) {
      p.drawLine( rect.left(),  rect.top() + ( delta / 2 ) + ( i * delta ),
                  rect.right(), rect.top() + ( delta / 2 ) + ( i * delta ) );
    }
  }

  // due date
  if ( todo->hasDueDate() && posDueDt>=0 ) {
    outStr = local->formatDate( todo->dtDue().date(), KLocale::ShortDate );
    rect = p.boundingRect( posDueDt, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // percentage completed
  bool showPercentComplete = posPercentComplete>=0;
  if ( showPercentComplete ) {
    int lwidth = 24;
    int lheight = 12;
    //first, draw the progress bar
    int progress = (int)( ( lwidth * todo->percentComplete() ) / 100.0 + 0.5 );

    p.setBrush( QBrush( Qt::NoBrush ) );
    p.drawRect( posPercentComplete, y+3, lwidth, lheight );
    if ( progress > 0 ) {
      p.setBrush( QColor( 128, 128, 128 ) );
      p.drawRect( posPercentComplete, y+3, progress, lheight );
    }

    //now, write the percentage
    outStr = i18n( "%1%", todo->percentComplete() );
    rect = p.boundingRect( posPercentComplete+lwidth+3, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // description
  if ( !todo->description().isEmpty() && desc ) {
    y = newrect.bottom() + 5;
    outStr = todo->description();
    rect = p.boundingRect( left + 20, y, x + width - ( left + 10 ), -1, Qt::WordBreak, outStr );
    p.drawText( rect, Qt::WordBreak, outStr, &newrect );
  }

  // Set the new line position
  y = newrect.bottom() + 10; //set the line position

  // If the to-do has sub-to-dos, we need to call ourselves recursively
#if 0
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  startPoints.append( &startpt );
  for ( it = l.begin(); it != l.end(); ++it ) {
    count++;
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if ( subtodo && todoList.contains( subtodo ) ) {
      drawTodo( count, subtodo, p, connectSubTodos, strikeoutCompleted,
                desc, posPriority, posSummary, posDueDt, posPercentComplete,
                level+1, x, y, width, pageHeight, todoList, &startpt );
    }
  }
#endif
  // Make a list of all the sub-to-dos related to this to-do.
  Todo::List t;
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  for ( it = l.begin(); it != l.end(); ++it ) {
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if ( subtodo && todoList.contains( subtodo ) ) {
      t.append( subtodo );
    }
  }

  // Sort the sub-to-dos and then print them
  Todo::List sl = mCalendar->sortTodos( &t, sortField, sortDir );
  Todo::List::ConstIterator isl;
  startPoints.append( &startpt );
  for ( isl = sl.begin(); isl != sl.end(); ++isl ) {
    count++;
    drawTodo( count, ( *isl ), p, sortField, sortDir,
              connectSubTodos, strikeoutCompleted,
              desc, posPriority, posSummary, posDueDt, posPercentComplete,
              level+1, x, y, width, pageHeight, todoList, &startpt );
  }
  startPoints.removeAll( &startpt );
}

int CalPrintPluginBase::weekdayColumn( int weekday )
{
  int w = weekday + 7 - KGlobal::locale()->weekStartDay();
  return w % 7;
}

void CalPrintPluginBase::drawJournalField( QPainter &p, const QString &entry,
                                       int x, int &y, int width, int pageHeight )
{
  QRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, entry ) );
  if ( rect.bottom() > pageHeight ) {
    // Start new page...
    // FIXME: If it's a multi-line text, draw a few lines on this page, and the
    // remaining lines on the next page.
    y = 0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, entry );
  }
  QRect newrect;
  p.drawText( rect, Qt::WordBreak, entry, &newrect );
  y = newrect.bottom() + 7;
}

void CalPrintPluginBase::drawJournal( Journal * journal, QPainter &p, int x, int &y,
                                  int width, int pageHeight )
{
  QFont oldFont( p.font() );
  p.setFont( QFont( "sans-serif", 15 ) );
  QString headerText;
  QString dateText( KGlobal::locale()->
        formatDate( journal->dtStart().date(), KLocale::LongDate ) );

  if ( journal->summary().isEmpty() ) {
    headerText = dateText;
  } else {
    headerText = i18nc( "Description - date", "%1 - %2", journal->summary(), dateText );
  }

  QRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText ) );
  if ( rect.bottom() > pageHeight ) {
    // Start new page...
    y = 0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText );
  }
  QRect newrect;
  p.drawText( rect, Qt::WordBreak, headerText, &newrect );
  p.setFont( oldFont );

  y = newrect.bottom() + 4;

  p.drawLine( x + 3, y, x + width - 6, y );
  y += 5;

  if ( !( journal->organizer().fullName().isEmpty() ) ) {
    drawJournalField( p, i18n( "Person: %1", journal->organizer().fullName() ),
                      x, y, width, pageHeight );
  }
  if ( !( journal->description().isEmpty() ) ) {
    drawJournalField( p, journal->description(), x, y, width, pageHeight );
  }
  y += 10;
}

void CalPrintPluginBase::drawSplitHeaderRight( QPainter &p, const QDate &fd,
                                               const QDate &td, const QDate &,
                                               int width, int height )
{
  QFont oldFont( p.font() );

  QPen oldPen( p.pen() );
  QPen pen( Qt::black, 4 );

  QString title;
  if ( mCalSys ) {
    if ( fd.month() == td.month() ) {
      title = i18nc( "Date range: Month dayStart - dayEnd", "%1 %2 - %3",
                     mCalSys->monthName( fd.month(), KCalendarSystem::LongName ),
                     mCalSys->dayString( fd, KCalendarSystem::LongFormat ),
                     mCalSys->dayString( td, KCalendarSystem::LongFormat ) );
    } else {
      title = i18nc( "Date range: monthStart dayStart - monthEnd dayEnd", "%1 %2 - %3 %4",
                     mCalSys->monthName( fd.month(), KCalendarSystem::LongName ),
                     mCalSys->dayString( fd, KCalendarSystem::LongFormat ),
                     mCalSys->monthName( td.month(), KCalendarSystem::LongName ),
                     mCalSys->dayString( td, KCalendarSystem::LongFormat ) );
    }
  }

  if ( height < 60 ) {
    p.setFont( QFont( "Times", 22 ) );
  } else {
    p.setFont( QFont( "Times", 28 ) );
  }

  int lineSpacing = p.fontMetrics().lineSpacing();
  p.drawText( 0, 0, width, lineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  title.truncate(0);

  p.setPen( pen );
  p.drawLine( 300, lineSpacing, width, lineSpacing );
  p.setPen( oldPen );

  if ( height < 60 ) {
    p.setFont( QFont( "Times", 14, QFont::Bold, true ) );
  } else {
    p.setFont( QFont( "Times", 18, QFont::Bold, true ) );
  }

  title += QString::number( fd.year() );
  p.drawText( 0, lineSpacing, width, lineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  p.setFont( oldFont );
}

#endif
