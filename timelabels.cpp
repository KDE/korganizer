/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "timelabels.h"

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include <kglobal.h>

#include "koglobals.h"
#include "kocore.h"
#include "koprefs.h"
#include "koagenda.h"

TimeLabels::TimeLabels(int rows,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = KOPrefs::instance()->mHourSize*4;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50, int(mRows * mCellHeight) );

  viewport()->setBackgroundMode( PaletteBackground );

  mMousePos = new QFrame(this);
  mMousePos->setLineWidth(0);
  mMousePos->setMargin(0);
  mMousePos->setBackgroundColor(Qt::red);
  mMousePos->setFixedSize(width(), 1);
  addChild(mMousePos, 0, 0);
}

void TimeLabels::mousePosChanged(const QPoint &pos)
{
  moveChild(mMousePos, 0, pos.y());
}

void TimeLabels::showMousePos()
{
  mMousePos->show();
}

void TimeLabels::hideMousePos()
{
  mMousePos->hide();
}

void TimeLabels::setCellHeight(double height)
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view
  is redrawn.  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents(QPainter *p,int cx, int cy, int cw, int ch)
{
  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  cx = contentsX() + frameWidth()*2;
  cw = contentsWidth() ;
  // end of workaround

  int cell = ((int)(cy/mCellHeight));
  double y = cell * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  QString suffix = "am";
  int timeHeight =  fm.ascent();
  QFont nFont = font();
  p->setFont( font() );

  if (!KGlobal::locale()->use12Clock()) {
      suffix = "00";
  } else
      if (cell > 11) suffix = "pm";

  if ( timeHeight >  mCellHeight ) {
    timeHeight = int(mCellHeight-1);
    int pointS = nFont.pointSize();
    while ( pointS > 4 ) {
      nFont.setPointSize( pointS );
      fm = QFontMetrics( nFont );
      if ( fm.ascent() < mCellHeight )
        break;
      -- pointS;
    }
    fm = QFontMetrics( nFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont sFont = nFont;
  sFont.setPointSize( sFont.pointSize()/2 );
  QFontMetrics fmS(  sFont );
  int startW = mMiniWidth - frameWidth()-2 ;
  int tw2 = fmS.width(suffix);
  int divTimeHeight = (timeHeight-1) /2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while (y < cy + ch+mCellHeight) {
    // hour, full line
    p->drawLine( cx, int(y), cw+2, int(y) );
    hour.setNum(cell);
    // handle 24h and am/pm time formats
    if (KGlobal::locale()->use12Clock()) {
      if (cell == 12) suffix = "pm";
      if (cell == 0) hour.setNum(12);
      if (cell > 12) hour.setNum(cell - 12);
    }

    // center and draw the time label
    int timeWidth = fm.width(hour);
    int offset = startW - timeWidth - tw2 -1 ;
    p->setFont( nFont );
    p->drawText( offset, int(y+timeHeight), hour);
    p->setFont( sFont );
    offset = startW - tw2;
    p->drawText( offset, int(y+timeHeight-divTimeHeight), suffix);

    // increment indices
    y += mCellHeight;
    cell++;
  }

}

/**
   Calculates the minimum width.
*/
int TimeLabels::minimumWidth() const
{
  return mMiniWidth;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  setFont(KOPrefs::instance()->mTimeBarFont);

  QString test = "20";
  if ( KGlobal::locale()->use12Clock() )
      test = "12";
  mMiniWidth = fontMetrics().width( test );
  if ( KGlobal::locale()->use12Clock() )
      test = "pm";
  else {
      test = "00";
  }
  QFont sFont = font();
  sFont.setPointSize(  sFont.pointSize()/2 );
  QFontMetrics fmS(   sFont );
  mMiniWidth += fmS.width(  test ) + frameWidth()*2+4 ;
  // update geometry restrictions based on new settings
  setFixedWidth(  mMiniWidth );

  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize*4;
  // If the agenda is zoomed out so that more then 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4*mAgenda->gridSpacingY() )
       mCellHeight = 4*mAgenda->gridSpacingY();
  resizeContents( mMiniWidth, int(mRows * mCellHeight+1) );
}

/** update time label positions */
void TimeLabels::positionChanged()
{
  int adjustment = mAgenda->contentsY();
  setContentsPos(0, adjustment);
}

void TimeLabels::positionChanged( int pos )
{
  setContentsPos( 0, pos );
}

/**  */
void TimeLabels::setAgenda(KOAgenda* agenda)
{
  mAgenda = agenda;

  connect(mAgenda, SIGNAL(mousePosSignal(const QPoint &)), this, SLOT(mousePosChanged(const QPoint &)));
  connect(mAgenda, SIGNAL(enterAgenda()), this, SLOT(showMousePos()));
  connect(mAgenda, SIGNAL(leaveAgenda()), this, SLOT(hideMousePos()));
  connect(mAgenda, SIGNAL(gridSpacingYChanged( double ) ), this, SLOT( setCellHeight( double ) ) );
}


/** This is called in response to repaint() */
void TimeLabels::paintEvent(QPaintEvent*)
{
//  kdDebug(5850) << "paintevent..." << endl;
  // this is another hack!
//  QPainter painter(this);
  //QString c
  repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

#include "timelabels.moc"
