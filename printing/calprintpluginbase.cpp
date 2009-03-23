/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2008 Ron Goodheart <rong.dev@gmail.com>

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
#include "koprefs.h"
#include "kohelper.h"

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
#include <QTextDocumentFragment>
#include <QTextDocument>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <qmath.h> // qCeil

#ifndef KORG_NOPRINTER

/******************************************************************
 **              The Todo positioning structure                  **
 ******************************************************************/
class CalPrintPluginBase::TodoParentStart
{
  public:
    TodoParentStart( QRect pt = QRect(), bool hasLine = false, bool page = true )
      : mRect( pt ), mHasLine( hasLine ), mSamePage( page ) {}

    QRect mRect;
    bool mHasLine;
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
    mExcludeConfidential = group.readEntry( "Exclude confidential", true );
    mExcludePrivate = group.readEntry( "Exclude private", true );
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
    group.writeEntry( "Exclude confidential", mExcludeConfidential );
    group.writeEntry( "Exclude private", mExcludePrivate );
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
  QColor tColor( KOHelper::getTextColor( bgColor ) );
  if ( tColor.isValid() ) {
    p.setPen( tColor );
  }
}

QColor CalPrintPluginBase::categoryBgColor( Incidence *incidence )
{
  if ( mCoreHelper && incidence ) {
    QColor backColor = mCoreHelper->categoryColor( incidence->categories() );
    if ( incidence->type() == "Todo" ) {
      if ( static_cast<Todo*>( incidence )->isOverdue() ) {
        backColor = KOPrefs::instance()->todoOverdueColor();
      }
    }
    return backColor;
  } else {
    return QColor();
  }
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
    KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
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
  // no border
  if ( linewidth >= 0 ) {
  pen.setWidth( linewidth );
  p.setPen( pen );
  } else {
    p.setPen( Qt::NoPen );
  }
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
    p.setPen( KOHelper::getTextColor( bgColor ) );
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
                                            const QFont &textFont,
                                            bool richContents )
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
  captionBox = p.boundingRect( captionBox,
                               Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine,
                               caption );
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
  }
  drawBox( p, BOX_BORDER_WIDTH, box );
  p.setFont( captionFont );
  p.drawText( captionBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine,
              caption );

  if ( !contents.isEmpty() ) {
    if ( sameLine ) {
      QString contentText = toPlainText( contents );
      p.setFont( textFont );
      p.drawText( textBox, Qt::AlignLeft | Qt::AlignTop | Qt::SingleLine,
                  contents );
    } else {
      QTextDocument rtb;
      int borderWidth = 2 * BOX_BORDER_WIDTH;
      if ( richContents ) {
        rtb.setHtml( contents );
      } else {
        rtb.setPlainText( contents );
      }
      int boxHeight = allbox.height();
      if ( !sameLine ) {
        boxHeight -= captionBox.height();
      }
      rtb.setPageSize( QSize( textBox.width(), boxHeight ) );
      rtb.setDefaultFont( textFont );
      p.save();
      p.translate( textBox.x() - borderWidth, textBox.y() );
      QRect clipBox( 0, 0, box.width(), boxHeight );
      rtb.drawContents( &p, clipBox );
      p.restore();
      textBox.setBottom( textBox.y() +
                         rtb.documentLayout()->documentSize().height() );
    }
  }
  p.setFont( oldFont );

  if ( expand ) {
    return box.bottom();
  } else {
    return textBox.bottom();
  }
}

int CalPrintPluginBase::drawHeader( QPainter &p, const QString &title,
    const QDate &month1, const QDate &month2, const QRect &allbox,
    bool expand, QColor backColor )
{
  // print previous month for month view, print current for to-do, day and week
  int smallMonthWidth = ( allbox.width() / 4 ) - 10;
  if ( smallMonthWidth > 100 ) {
    smallMonthWidth = 100;
  }

  QRect box( allbox );
  QRect textRect( allbox );

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

  if ( !backColor.isValid() ) {
    backColor = QColor( 232, 232, 232 );
  }

  drawShadedBox( p, BOX_BORDER_WIDTH, backColor, box );

#if 0
  // current month title left justified, prev month, next month right justified
  QRect monthbox2( box.right()-10-smallMonthWidth, box.top(),
                   smallMonthWidth, box.height() );
  if ( month2.isValid() ) {
    drawSmallMonth( p, QDate( month2.year(), month2.month(), 1 ), monthbox2 );
    textRect.setRight( monthbox2.left() );
  }
  QRect monthbox1( monthbox2.left()-10-smallMonthWidth, box.top(),
                   smallMonthWidth, box.height() );
  if ( month1.isValid() ) {
    drawSmallMonth( p, QDate( month1.year(), month1.month(), 1 ), monthbox1 );
    textRect.setRight( monthbox1.left() );
  }

  // Set the margins
  p.setFont( newFont );
  textRect.adjust( 5, 0, 0, 0 );
  p.drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak, title );
  p.setFont( oldFont );
#endif
  // prev month left, current month centered, next month right
  QRect monthbox2( box.right()-10-smallMonthWidth, box.top(),
                   smallMonthWidth, box.height() );
  if ( month2.isValid() ) {
    drawSmallMonth( p, QDate( month2.year(), month2.month(), 1 ), monthbox2 );
    textRect.setRight( monthbox2.left() );
  }
  QRect monthbox1( box.left()+10, box.top(), smallMonthWidth, box.height() );
  if ( month1.isValid() ) {
    drawSmallMonth( p, QDate( month1.year(), month1.month(), 1 ), monthbox1 );
    textRect.setLeft( monthbox1.right() );
  }

  // Set the margins
  p.setFont( newFont );
  p.drawText( textRect, Qt::AlignCenter | Qt::AlignVCenter | Qt::WordBreak, title );
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
  p.setFont( QFont( "sans-serif", int(cellHeight-2), QFont::Normal ) );

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
                                       const QRect &box,
                                       bool excludeConfidential,
                                       bool excludePrivate )
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
    if ( ( excludeConfidential && currEvent->secrecy() == Incidence::SecrecyConfidential ) ||
         ( excludePrivate      && currEvent->secrecy() == Incidence::SecrecyPrivate ) ) {
      continue;
    }
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
                                           const QRect &oldbox,
                                           bool includeDescription,
                                           bool excludeTime,
                                           bool excludeConfidential,
                                           bool excludePrivate )
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
    for ( it = events.constBegin(); it != events.constEnd(); ++it ) {
      event = *it;
      if ( ( excludeConfidential && event->secrecy() == Incidence::SecrecyConfidential ) ||
           ( excludePrivate      && event->secrecy() == Incidence::SecrecyPrivate ) ) {
        continue;
      }
      // skip items without times so that we do not adjust for all day items
      if ( event->allDay() ) {
        continue;
      }
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
  for ( itEvents = events.constBegin(); itEvents != events.constEnd(); ++itEvents ) {
    QList<KDateTime> times = (*itEvents)->startDateTimesForDate( qd );
    for ( QList<KDateTime>::ConstIterator it = times.constBegin();
          it != times.constEnd(); ++it ) {
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
    drawAgendaItem( placeItem, p, startPrintDate, endPrintDate, minlen, box,
                    includeDescription, excludeTime );
  }
}

void CalPrintPluginBase::drawAgendaItem( PrintCellItem *item, QPainter &p,
                                   const KDateTime &startPrintDate,
                                   const KDateTime &endPrintDate,
                                   float minlen, const QRect &box,
                                   bool includeDescription,
                                   bool excludeTime )
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
    QString str;
    if ( excludeTime ) {
      if ( event->location().isEmpty() ) {
        str = event->summary();
      } else {
        str = i18nc( "summary, location", "%1, %2",
                    event->summary(), event->location() );
      }
    } else {
      if ( event->location().isEmpty() ) {
        str = i18nc( "starttime - endtime summary",
                    "%1-%2 %3",
                    KGlobal::locale()->formatTime( startTime.toLocalZone().time() ),
                    KGlobal::locale()->formatTime( endTime.toLocalZone().time() ),
                    event->summary() );
      } else {
        str = i18nc( "starttime - endtime summary, location",
                    "%1-%2 %3, %4",
                    KGlobal::locale()->formatTime( startTime.toLocalZone().time() ),
                    KGlobal::locale()->formatTime( endTime.toLocalZone().time() ),
                    event->summary(),
                    event->location() );
      }
    }
    if ( includeDescription ) {
      str += '\n';
      if ( event->descriptionIsRich() ) {
        str += toPlainText( event->description() );
      } else {
        str += event->description();
      }
    }
    showEventBox( p, eventBox, event, str );
  }
}

void CalPrintPluginBase::drawDayBox( QPainter &p, const QDate &qd,
                                     const QRect &box, bool fullDate,
                                     bool printRecurDaily, bool printRecurWeekly,
                                     bool singleLineLimit, bool showNoteLines,
                                     bool includeDescription,
                                     bool excludeConfidential,
                                     bool excludePrivate )
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
  const QFont oldFont( p.font() );

  QRect headerTextBox( subHeaderBox );
  headerTextBox.setLeft( subHeaderBox.left()+5 );
  headerTextBox.setRight( subHeaderBox.right()-5 );
  if ( !hstring.isEmpty() ) {
    p.setFont( QFont( "sans-serif", 8, QFont::Bold, true ) );

    p.drawText( headerTextBox, Qt::AlignLeft | Qt::AlignVCenter, hstring );
  }
  p.setFont( QFont( "sans-serif", 10, QFont::Bold ) );
  p.drawText( headerTextBox, Qt::AlignRight | Qt::AlignVCenter, dayNumStr );

  Event::List eventList = mCalendar->events( qd, KOPrefs::instance()->timeSpec(),
                                             EventSortStartDate,
                                             SortDirectionAscending );

  QString timeText;
  p.setFont( QFont( "sans-serif", 8 ) );

  int textY = mSubHeaderHeight; // gives the relative y-coord of the next printed entry
  Event::List::ConstIterator it;

  for ( it=eventList.constBegin(); it != eventList.constEnd() && textY < box.height(); ++it ) {
    Event *currEvent = *it;
    if ( ( !printRecurDaily  && currEvent->recurrenceType() == Recurrence::rDaily ) ||
         ( !printRecurWeekly && currEvent->recurrenceType() == Recurrence::rWeekly ) ) {
      continue;
    }
    if ( ( excludeConfidential && currEvent->secrecy() == Incidence::SecrecyConfidential ) ||
         ( excludePrivate      && currEvent->secrecy() == Incidence::SecrecyPrivate ) ) {
      continue;
    }
    if ( currEvent->allDay() || currEvent->isMultiDay() ) {
      timeText.clear();
    } else {
      timeText = local->formatTime( currEvent->dtStart().toLocalZone().time() ) + ' ';
    }
    p.save();
    setCategoryColors( p, currEvent );
    drawIncidence( p, box, timeText,
                   currEvent->summary(), currEvent->description(),
                   textY, singleLineLimit, includeDescription,
                   currEvent->descriptionIsRich() );
    p.restore();
  }

  if ( textY < box.height() ) {
    Todo::List todos = mCalendar->todos( qd );
    Todo::List::ConstIterator it2;
    for ( it2=todos.constBegin(); it2 != todos.constEnd() && textY < box.height(); ++it2 ) {
      Todo *todo = *it2;
      if ( ( !printRecurDaily  && todo->recurrenceType() == Recurrence::rDaily ) ||
           ( !printRecurWeekly && todo->recurrenceType() == Recurrence::rWeekly ) ) {
        continue;
      }
      if ( ( excludeConfidential && todo->secrecy() == Incidence::SecrecyConfidential ) ||
           ( excludePrivate      && todo->secrecy() == Incidence::SecrecyPrivate ) ) {
        continue;
      }
      if ( todo->hasDueDate() && !todo->allDay() ) {
        timeText += KGlobal::locale()->formatTime( todo->dtDue().toLocalZone().time() ) + ' ';
      } else {
        timeText.clear();
      }
      p.save();
      setCategoryColors( p, todo );
      drawIncidence( p, box, timeText,
                     i18n( "To-do: %1", todo->summary() ), todo->description(),
                     textY, singleLineLimit, includeDescription,
                     todo->descriptionIsRich() );
      p.restore();
    }
  }
  if ( showNoteLines ) {
    drawNoteLines( p, box, box.y() + textY );
  }

  p.setFont( oldFont );
}

void CalPrintPluginBase::drawIncidence( QPainter &p, const QRect &dayBox,
                                        const QString &time,
                                        const QString &summary,
                                        const QString &description,
                                        int &textY,  bool singleLineLimit,
                                        bool includeDescription,
                                        bool richDescription )
{
  kDebug() << "summary =" << summary << ", singleLineLimit=" << singleLineLimit;

  int flags = Qt::AlignLeft | Qt::OpaqueMode;
  QFontMetrics fm = p.fontMetrics();
  const int borderWidth = p.pen().width() + 1;
  QRect timeBound = p.boundingRect( dayBox.x() + borderWidth,
                                    dayBox.y() + textY,
                                    dayBox.width(), fm.lineSpacing(),
                                    flags, time );

  int summaryWidth = time.isEmpty() ? 0 : timeBound.width() + 3;
  QRect summaryBound = QRect( dayBox.x() + borderWidth + summaryWidth,
                              dayBox.y() + textY + 1,
                              dayBox.width() - summaryWidth - ( borderWidth * 2 ),
                              dayBox.height() - textY );

  QString summaryText = summary;
  bool boxOverflow = false;

  if ( singleLineLimit ) {
    QString lineText;
    lineText = summaryText;
    if ( includeDescription ) {
      lineText += ", " + toPlainText( description );
    }
    int totalHeight = fm.lineSpacing() + borderWidth;
    int textBoxHeight = ( totalHeight > ( dayBox.height() - textY ) ) ?
                        dayBox.height() - textY : totalHeight;
    summaryBound.setHeight(textBoxHeight);
    QRect lineRect( dayBox.x() + borderWidth, dayBox.y() + textY,
                    dayBox.width() - ( borderWidth * 2 ), textBoxHeight );
    drawBox( p, -1, lineRect );
    if ( !time.isEmpty() ) {
      p.drawText( timeBound, flags, time );
    }
    p.drawText( summaryBound, flags, lineText );
  } else {
    QTextDocument textDoc;
    QTextCursor textCursor( &textDoc );
    if ( richDescription ) {
      QTextCursor textCursor( &textDoc );
      textCursor.insertText( summaryText );
      if ( includeDescription ) {
        textCursor.insertText( "\n" );
        textCursor.insertHtml( description );
      }
    } else {
      textCursor.insertText( summaryText );
      if ( includeDescription ) {
        textCursor.insertText( "\n" );
        textCursor.insertText( description );
      }
    }
    textDoc.setPageSize( QSize( summaryBound.width(), summaryBound.height() ) );
    p.save();
    QRect clipBox( 0, 0, summaryBound.width(), summaryBound.height() );
    p.setFont( p.font() );
    p.translate( summaryBound.x(), summaryBound.y() - 6 );
    textDoc.drawContents( &p, clipBox );
    summaryBound.setHeight( textDoc.documentLayout()->documentSize().height()- 9 );
    if ( summaryBound.bottom() > dayBox.bottom() ) {
      summaryBound.setBottom( dayBox.bottom() );
    }
    clipBox.setHeight( summaryBound.height() );
    p.restore();

    p.save();
    QRect backBox( timeBound.x(), timeBound.y(),
                   dayBox.width() - ( borderWidth * 2 ), clipBox.height() );
    drawBox( p, -1, backBox );

    if ( !time.isEmpty() ) {
      if ( timeBound.bottom() > dayBox.bottom() ) {
        timeBound.setBottom( dayBox.bottom() );
      }
      p.drawText( timeBound, flags, time );
    }
    p.translate( summaryBound.x(), summaryBound.y() - 6 );
    textDoc.drawContents( &p, clipBox );
    p.restore();
    boxOverflow = textDoc.pageCount() > 1;
  }
  if ( summaryBound.bottom() < dayBox.bottom() ) {
    QPen oldPen( p.pen() );
    p.setPen( QPen() );
    p.drawLine( dayBox.x(), summaryBound.bottom(),
                dayBox.x() + dayBox.width(), summaryBound.bottom() );
    p.setPen( oldPen );
  }
  textY += summaryBound.height();

  // show that we have overflowed the box
  if ( boxOverflow ) {
    QPolygon poly(3);
    int x = dayBox.x() + dayBox.width();
    int y = dayBox.y() + dayBox.height();
    poly.setPoint( 0, x - 10, y );
    poly.setPoint( 1, x, y - 10 );
    poly.setPoint( 2, x, y );
    QBrush oldBrush( p.brush() );
    p.setBrush( QBrush( Qt::black ) );
    p.drawPolygon(poly);
    p.setBrush( oldBrush );
    textY = dayBox.height();
  }
}

void CalPrintPluginBase::drawWeek( QPainter &p, const QDate &qd,
                                   const QRect &box,
                                   bool singleLineLimit, bool showNoteLines,
                                   bool includeDescription,
                                   bool excludeConfidential,
                                   bool excludePrivate )
{
  QDate weekDate = qd;
  const bool portrait = ( box.height() > box.width() );
  int cellWidth;
  int vcells;
  if ( portrait ) {
    cellWidth = box.width() / 2;
    vcells=3;
  } else {
    cellWidth = box.width() / 6;
    vcells=1;
  }
  const int cellHeight = box.height() / vcells;

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
    drawDayBox( p, weekDate, dayBox, true, true, true,
                singleLineLimit, showNoteLines, includeDescription,
                excludeConfidential, excludePrivate );
  } // for i through all weekdays
}

void CalPrintPluginBase::drawDays( QPainter &p, const QDate &start,
                                   const QDate &end, const QRect &box,
                                   bool singleLineLimit, bool showNoteLines,
                                   bool includeDescription,
                                   bool excludeConfidential, bool excludePrivate )
{
  const int numberOfDays = start.daysTo( end ) + 1;
  int vcells;
  const bool portrait = ( box.height() > box.width() );
  int cellWidth;
  if ( portrait ) {
    // 2 columns
    vcells = qCeil( static_cast<double>( numberOfDays ) / 2.0 );
    if ( numberOfDays > 1 ) {
      cellWidth = box.width() / 2;
    } else {
      cellWidth = box.width();
    }
  } else {
    // landscape: N columns
    vcells = 1;
    cellWidth = box.width() / numberOfDays;
  }
  const int cellHeight = box.height() / vcells;
  QDate weekDate = start;
  for ( int i = 0; i < numberOfDays; ++i, weekDate = weekDate.addDays(1) ) {
    const int hpos = i / vcells;
    const int vpos = i % vcells;
    const QRect dayBox(
      box.left() + cellWidth * hpos,
      box.top() + cellHeight * vpos,
      cellWidth, cellHeight );
    drawDayBox( p, weekDate, dayBox, true, true, true, singleLineLimit,
                showNoteLines, includeDescription, excludeConfidential,
                excludePrivate );
  } // for i through all selected days
}

void CalPrintPluginBase::drawTimeTable( QPainter &p,
                                        const QDate &fromDate,
                                        const QDate &toDate,
                                        QTime &fromTime, QTime &toTime,
                                        const QRect &box,
                                        bool includeDescription,
                                        bool excludeTime,
                                        bool excludeConfidential,
                                        bool excludePrivate )
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
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
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
    alldayHeight = drawAllDayBox( p, eventList, curDate, false, allDayBox,
                                  excludeConfidential, excludePrivate );
    drawAgendaDayBox( p, eventList, curDate, false, fromTime, toTime,
                      dayBox, includeDescription, excludeTime,
                      excludeConfidential, excludePrivate );
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
    QRect dayBox( box.left(), daysBox.top() + qRound( dayheight * daysinmonth ), box.width(), 0 );
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
      daysBox.top() + qRound( dayheight * d ), daysBox.width()/*-rand()%50*/, 0 );
    // FIXME: When using a border width of 0 for event boxes,
    // don't let the rectangles overlap, i.e. subtract 1 from the top or bottom!
    dayBox.setBottom( daysBox.top() + qRound( dayheight * ( d + 1 ) ) - 1 );

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

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( Event::List::ConstIterator evit = events.constBegin(); evit != events.constEnd(); ++evit ) {
    Event *e = (*evit);
    if ( !e ) {
      continue;
    }
    if ( e->recurs() ) {
      if ( e->recursOn( start, timeSpec ) ) {
        // This occurrence has possibly started before the beginning of the
        // month, so obtain the start date before the beginning of the month
        QList<KDateTime> starttimes = e->startDateTimesForDate( start );
        QList<KDateTime>::ConstIterator it = starttimes.constBegin();
        for ( ; it != starttimes.constEnd(); ++it ) {
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
          for ( TimeList::ConstIterator it = times.constBegin();
                it != times.constEnd(); ++it ) {
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

  QList<MonthEventStruct>::ConstIterator mit = monthentries.constBegin();
  KDateTime endofmonth( end, QTime( 0, 0, 0 ) );
  endofmonth = endofmonth.addDays(1);
  for ( ; mit != monthentries.constEnd(); ++mit ) {
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
      daysBox.top() + qRound(
        double( minsToStart * daysBox.height() ) / double( maxdays * 24 * 60 ) ), 14, 0 );
    eventBox.setBottom(
      daysBox.top() + qRound(
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
    QRect dayBox( xstartcont, daysBox.top() + qRound( dayheight * d ), 0, 0 );
    dayBox.setRight( box.right() );
    dayBox.setBottom( daysBox.top() + qRound( dayheight * ( d + 1 ) ) );
    printEventString( p, dayBox, txt, Qt::AlignTop | Qt::AlignLeft | Qt::BreakAnywhere );
  }
  p.setFont( oldfont );
  drawBox( p, BOX_BORDER_WIDTH, borderBox );
  p.restore();
}

void CalPrintPluginBase::drawMonthTable( QPainter &p, const QDate &qd,
                                         bool weeknumbers, bool recurDaily,
                                         bool recurWeekly, bool singleLineLimit,
                                         bool showNoteLines,
                                         bool includeDescription,
                                         bool excludeConfidential,
                                         bool excludePrivate,
                                         const QRect &box )
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
      drawDayBox( p, monthDate, dayBox, false,
                  recurDaily, recurWeekly, singleLineLimit, showNoteLines,
                  includeDescription, excludeConfidential, excludePrivate );
      if ( darkbg ) {
        p.setBackground( back );
        darkbg = false;
      }
      monthDate = monthDate.addDays(1);
    }
  }
}


void CalPrintPluginBase::drawTodoLines( QPainter &p,
                                        const QString &entry,
                                        int x, int &y, int width,
                                        int pageHeight, bool richTextEntry,
                                        QList<TodoParentStart *> &startPoints,
                                        bool connectSubTodos )
{
  QString plainEntry = ( richTextEntry ) ? toPlainText( entry ) : entry;

  QRect textrect(0,0,width,-1);
  int flags = Qt::AlignLeft;
  QFontMetrics fm = p.fontMetrics();

  QStringList lines = plainEntry.split( '\n' );
  for ( int currentLine = 0; currentLine < lines.count(); currentLine++ ) {
    // split paragraphs into lines
    KWordWrap *ww = KWordWrap::formatText( fm, textrect, flags, lines[currentLine] );
    QStringList textLine = ww->wrappedString().split( '\n' );
    delete ww;

    // print each individual line
    for ( int lineCount = 0; lineCount < textLine.count(); lineCount++ ) {
      if ( y >= pageHeight ) {
        if ( connectSubTodos ) {
          for ( int i = 0; i < startPoints.size(); ++i ) {
            TodoParentStart *rct;
            rct = startPoints.at( i );
            int start = rct->mRect.bottom() + 1;
            int center = rct->mRect.left() + ( rct->mRect.width() / 2 );
            int to = y;
            if ( !rct->mSamePage ) {
              start = 0;
            }
            if ( rct->mHasLine ) {
              p.drawLine( center, start, center, to );
            }
            rct->mSamePage = false;
          }
        }
        y = 0;
        mPrinter->newPage();
      }
      y += fm.height();
      p.drawText( x, y, textLine[ lineCount ] );
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
                               TodoParentStart *r, bool excludeConfidential,
                               bool excludePrivate )
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

  y += 10;

  // Compute the right hand side of the to-do box
  int rhs = posPercentComplete;
  if ( rhs < 0 ) {
    rhs = posDueDt; //not printing percent completed
  }
  if ( rhs < 0 ) {
    rhs = x + width;  //not printing due dates either
  }

  int left = posSummary + ( level * 10 );

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
    int to( rect.top() + ( rect.height() / 2 ) );
    int endx( rect.left() );
    p.drawLine( center, to, endx, to );  // side connector
    if ( r->mSamePage ) { 
      bottom = r->mRect.bottom() + 1;
    } else {
      bottom = 0;
    }
    p.drawLine( center, bottom, center, to );
  }

  // summary
  outStr = todo->summary();
  rect = p.boundingRect( lhs, rect.top(), ( rhs - ( left + rect.width() + 5 ) ),
                         -1, Qt::WordBreak, outStr );

  QRect newrect;
  QFont newFont( p.font() );
  QFont oldFont( p.font() );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    newFont.setStrikeOut( true );
    p.setFont( newFont );
  }
  p.drawText( rect, Qt::WordBreak, outStr, &newrect );
  p.setFont( oldFont );
  // due date
  if ( todo->hasDueDate() && posDueDt >= 0 ) {
    outStr = local->formatDate( todo->dtDue().toLocalZone().date(), KLocale::ShortDate );
    rect = p.boundingRect( posDueDt, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // percentage completed
  bool showPercentComplete = posPercentComplete >= 0;
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

  y += 10;

  // Make a list of all the sub-to-dos related to this to-do.
  Todo::List t;
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  for ( it = l.constBegin(); it != l.constEnd(); ++it ) {
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if ( subtodo && todoList.contains( subtodo ) ) {
      if ( ( excludeConfidential &&
          subtodo->secrecy() == Incidence::SecrecyConfidential ) ||
          ( excludePrivate      &&
          subtodo->secrecy() == Incidence::SecrecyPrivate ) ) {
        continue;
      }
      t.append( subtodo );
    }
  }

  // has sub-todos?
  startpt.mHasLine = ( l.size() > 0 );
  startPoints.append( &startpt );

  // description
  if ( !todo->description().isEmpty() && desc ) {
    y = newrect.bottom() + 5;
    drawTodoLines( p, todo->description(), left, y,
                   width - ( left + 10 - x ), pageHeight,
                   todo->descriptionIsRich(),
                   startPoints, connectSubTodos );
  } else {
    y += 10;
  }

  // Sort the sub-to-dos and print them
  Todo::List sl = mCalendar->sortTodos( &t, sortField, sortDir );
  Todo::List::ConstIterator isl;
  int subcount = 0;
  for ( isl = sl.constBegin(); isl != sl.constEnd(); ++isl ) {
    count++;
    if ( ++subcount == sl.size() ) {
      startpt.mHasLine = false;
    }
    drawTodo( count, ( *isl ), p, sortField, sortDir,
              connectSubTodos, strikeoutCompleted,
              desc, posPriority, posSummary, posDueDt, posPercentComplete,
              level+1, x, y, width, pageHeight, todoList, &startpt,
              excludeConfidential, excludePrivate );
  }
  startPoints.removeAll( &startpt );
}

int CalPrintPluginBase::weekdayColumn( int weekday )
{
  int w = weekday + 7 - KGlobal::locale()->weekStartDay();
  return w % 7;
}

void CalPrintPluginBase::drawTextLines( QPainter &p, const QString &entry,
                                           int x, int &y, int width,
                                           int pageHeight, bool richTextEntry )
{
  QString plainEntry = ( richTextEntry ) ? toPlainText( entry ) : entry;

  QRect textrect(0,0,width,-1);
  int flags = Qt::AlignLeft;
  QFontMetrics fm = p.fontMetrics();

  QStringList lines = plainEntry.split( '\n' );
  for ( int currentLine = 0; currentLine < lines.count(); currentLine++ ) {
    // split paragraphs into lines
    KWordWrap *ww = KWordWrap::formatText( fm, textrect, flags, lines[currentLine] );
    QStringList textLine = ww->wrappedString().split( '\n' );
    delete ww;
    // print each individual line
    for ( int lineCount = 0; lineCount < textLine.count(); lineCount++ ) {
      if ( y >= pageHeight ) {
        y = 0;
        mPrinter->newPage();
      }
      y += fm.height();
      p.drawText( x, y, textLine[ lineCount ] );
    }
  }
}

void CalPrintPluginBase::drawJournal( Journal * journal, QPainter &p, int x, int &y,
                                      int width, int pageHeight )
{
  QFont oldFont( p.font() );
  p.setFont( QFont( "sans-serif", 15 ) );
  QString headerText;
  QString dateText( KGlobal::locale()->
        formatDate( journal->dtStart().toLocalZone().date(), KLocale::LongDate ) );

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
    drawTextLines( p, i18n( "Person: %1", journal->organizer().fullName() ),
                      x, y, width, pageHeight, false );
    y += 7;
  }
  if ( !( journal->description().isEmpty() ) ) {
    drawTextLines( p, journal->description(), x, y, width, pageHeight,
                      journal->descriptionIsRich() );
    y += 7;
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

void CalPrintPluginBase::drawNoteLines( QPainter &p, const QRect &box, int startY )
{
  int lineHeight = int( p.fontMetrics().lineSpacing() * 1.5 );
  int linePos = box.y();
  int startPos = startY;
  // adjust line to start at multiple from top of box for alignment
  while ( linePos < startPos ) {
    linePos += lineHeight;
  }
  QPen oldPen( p.pen() );
  p.setPen( Qt::DotLine );
  while ( linePos < box.bottom() ) {
    p.drawLine( box.left() + padding(), linePos,
                box.right() - padding(), linePos );
    linePos += lineHeight;
  }
  p.setPen( oldPen );
}

QString CalPrintPluginBase::toPlainText( const QString &htmlText )
{
  // this converts possible rich text to plain text
  return QTextDocumentFragment::fromHtml( htmlText ).toPlainText();
}

void CalPrintPluginBase::drawFooter( QPainter &p, const QRect &box )
{
  QFont oldfont( p.font() );
  p.setFont( QFont( "sans-serif", 6 ) );
  QFontMetrics fm( p.font() );
  QRect footerBox( box.left(), box.bottom() + padding(), box.width(), fm.height() );
  QString dateStr = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() );
  p.drawText( footerBox, Qt::AlignCenter | Qt::AlignVCenter | Qt::SingleLine,
              i18nc( "print date: formatted-datetime", "printed: %1", dateStr ) );
  p.setFont( oldfont );
}

#endif
