/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

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

#include "timelabels.h"
#include "timelabelszone.h"
#include "timescaleconfigdialog.h"
#include "koglobals.h"
#include "koprefs.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"
#include "koalternatelabel.h"
#include "views/agendaview/koagendaitem.h"
#include "views/agendaview/koagenda.h"

#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kholidays.h>
#include <KVBox>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kcombobox.h>

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QFont>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QCursor>
#include <QPaintEvent>
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVBoxLayout>

using namespace KOrg;

TimeLabels::TimeLabels( const KDateTime::Spec &spec, int rows,
                        TimeLabelsZone *parent, Qt::WFlags f )
  : Q3ScrollView( parent, /*name*/0, f )
{
  mTimeLabelsZone = parent;
  mSpec = spec;

  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = KOPrefs::instance()->mHourSize * 4;

  enableClipper( true );

  setHScrollBarMode( AlwaysOff );
  setVScrollBarMode( AlwaysOff );
  setFrameStyle( Plain );

  resizeContents( 50, int( mRows * mCellHeight ) );

  viewport()->setBackgroundRole( QPalette::Background );
  setBackgroundRole( QPalette::Background );

  mMousePos = new QFrame( this );
  mMousePos->setLineWidth( 1 );
  mMousePos->setFrameStyle( QFrame::HLine | QFrame::Plain );
//  mMousePos->setMargin(0);
  QPalette pal;
  pal.setColor( QPalette::Window, // for Oxygen
                KOPrefs::instance()->agendaMarcusBainsLineLineColor() );
  pal.setColor( QPalette::WindowText, // for Plastique
                KOPrefs::instance()->agendaMarcusBainsLineLineColor() );
  mMousePos->setPalette( pal );
  mMousePos->setFixedSize( width(), 1 );
  addChild( mMousePos, 0, 0 );

  if ( mSpec.isValid() ) {
    setToolTip( i18n( "Timezone:" ) + mSpec.timeZone().name() );
  }
}

void TimeLabels::mousePosChanged( const QPoint &pos )
{
  moveChild( mMousePos, 0, pos.y() );

  // The repaint somehow prevents that the red line leaves a black artifact when
  // moved down. It's not a full solution, though.
  repaint();
}

void TimeLabels::showMousePos()
{
  mMousePos->show();
}

void TimeLabels::hideMousePos()
{
  mMousePos->hide();
}

void TimeLabels::setCellHeight( double height )
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view is redrawn.
  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  int beginning;

  if ( !mSpec.isValid() ) {
    beginning = 0;
  } else {
    beginning = ( mSpec.timeZone().currentOffset() -
                  KOPrefs::instance()->timeSpec().timeZone().currentOffset() ) / ( 60 * 60 );
  }

  p->setBrush( palette().window() ); // TODO: theming, see if we want sth here...
  p->fillRect( cx, cy, cw, ch, p->brush() );

  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  cx = contentsX() + frameWidth() * 2;
  cw = contentsWidth();

  // end of workaround
  int cell = ( (int)( cy / mCellHeight ) ) + beginning;  // the hour we start drawing with
  double y = ( cell - beginning ) * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  int timeHeight = fm.ascent();
  QFont hourFont = KOPrefs::instance()->agendaTimeLabelsFont();
  p->setFont( font() );

  QString suffix;
  if ( ! KGlobal::locale()->use12Clock() ) {
      suffix = "00";
  } else {
    suffix = "am";
    if ( cell > 11 ) {
      suffix = "pm";
    }
  }

  // We adjust the size of the hour font to keep it reasonable
  if ( timeHeight >  mCellHeight ) {
    timeHeight = int( mCellHeight - 1 );
    int pointS = hourFont.pointSize();
    while ( pointS > 4 ) { // TODO: use smallestReadableFont() when added to kdelibs
      hourFont.setPointSize( pointS );
      fm = QFontMetrics( hourFont );
      if ( fm.ascent() < mCellHeight ) {
        break;
      }
      --pointS;
    }
    fm = QFontMetrics( hourFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont suffixFont = hourFont;
  suffixFont.setPointSize( suffixFont.pointSize() / 2 );
  QFontMetrics fmS( suffixFont );
  int startW = mMiniWidth - frameWidth() - 2 ;
  int tw2 = fmS.width( suffix );
  int divTimeHeight = ( timeHeight - 1 ) / 2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while ( y < cy + ch + mCellHeight ) {
    QPen pen;
    if ( cell < 0 || cell >= 24 ) {
      pen.setColor( QColor( 150, 150, 150 ) );
    } else {
      pen.setColor( palette().color( QPalette::WindowText ) );
    }
    p->setPen( pen );

    // hour, full line
    p->drawLine( cx, int( y ), cw + 2, int( y ) );

    hour.setNum( cell % 24 );
    // handle different timezones
    if ( cell < 0 ) {
      hour.setNum( cell + 24 );
    }
    // handle 24h and am/pm time formats
    if ( KGlobal::locale()->use12Clock() ) {
      if ( cell == 12 ) {
        suffix = "pm";
      }
      if ( cell == 0 ) {
        hour.setNum( 12 );
      }
      if ( cell > 12 ) {
        hour.setNum( cell - 12 );
      }
    }

    // center and draw the time label
    int timeWidth = fm.width( hour );
    int offset = startW - timeWidth - tw2 -1 ;
    p->setFont( hourFont );
    p->drawText( offset, int( y + timeHeight ), hour );
    p->setFont( suffixFont );
    offset = startW - tw2;
    p->drawText( offset, int( y + timeHeight - divTimeHeight ), suffix );

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
  setFont( KOPrefs::instance()->agendaTimeLabelsFont() );

  QString test = "20";
  if ( KGlobal::locale()->use12Clock() ) {
      test = "12";
  }
  mMiniWidth = fontMetrics().width( test );
  if ( KGlobal::locale()->use12Clock() ) {
    test = "pm";
  } else {
    test = "00";
  }
  QFont sFont = font();
  sFont.setPointSize( sFont.pointSize() / 2 );
  QFontMetrics fmS( sFont );
  mMiniWidth += fmS.width( test ) + frameWidth() * 2 + 4 ;
  // update geometry restrictions based on new settings
  setFixedWidth( mMiniWidth );

  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize * 4;
  // If the agenda is zoomed out so that more than 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4 * mAgenda->gridSpacingY() ) {
       mCellHeight = 4 * mAgenda->gridSpacingY();
  }
  resizeContents( mMiniWidth, int( mRows * mCellHeight + 1 ) );
}

/** update time label positions */
void TimeLabels::positionChanged()
{
  int adjustment = mAgenda->contentsY();
  if ( adjustment != contentsY() ) {
    setContentsPos( 0, adjustment );
  }
}

void TimeLabels::positionChanged( int pos )
{
  if ( pos != contentsY() ) {
    setContentsPos( 0, pos );
  }
}

/**  */
void TimeLabels::setAgenda( KOAgenda *agenda )
{
  mAgenda = agenda;

  connect( mAgenda, SIGNAL(mousePosSignal(const QPoint &)),
           this, SLOT(mousePosChanged(const QPoint &)) );
  connect( mAgenda, SIGNAL(enterAgenda()), this, SLOT(showMousePos()) );
  connect( mAgenda, SIGNAL(leaveAgenda()), this, SLOT(hideMousePos()) );
  connect( mAgenda, SIGNAL(gridSpacingYChanged(double)),
           this, SLOT(setCellHeight(double)) );
}

/** This is called in response to repaint() */
void TimeLabels::paintEvent( QPaintEvent * )
{
//  kDebug();
  QPainter painter( this );
  drawContents( &painter, contentsX(), contentsY(), visibleWidth(), visibleHeight() );
}

void TimeLabels::contextMenuEvent( QContextMenuEvent *event )
{
  Q_UNUSED( event );

  QMenu popup( this );
  QAction *editTimeZones =
    popup.addAction( KIcon( "document-properties" ), i18n( "&Edit Timezones..." ) );
  QAction *removeTimeZone =
    popup.addAction( KIcon( "edit-delete" ),
                     i18n( "&Remove %1 Timezone", mSpec.timeZone().name() ) );
  if ( !mSpec.isValid() ||
       !KOPrefs::instance()->timeScaleTimezones().count() ||
       mSpec == KOPrefs::instance()->timeSpec() ) {
    removeTimeZone->setEnabled( false );
  }

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == editTimeZones ) {
    TimeScaleConfigDialog dialog( this );
    if ( dialog.exec() == QDialog::Accepted ) {
      mTimeLabelsZone->reset();
    }
  } else if ( activatedAction == removeTimeZone ) {
    QStringList list = KOPrefs::instance()->timeScaleTimezones();
    list.removeAll( mSpec.timeZone().name() );
    KOPrefs::instance()->setTimeScaleTimezones( list );
    mTimeLabelsZone->reset();
    hide();
    deleteLater();
  }
}

KDateTime::Spec TimeLabels::timeSpec()
{
  return mSpec;
}

QString TimeLabels::header() const
{
  KTimeZone tz = mSpec.timeZone();

  QString header = tz.countryCode();
  if ( header.isEmpty() ) {
    header = tz.name();
  }

  return header;
}

QString TimeLabels::headerToolTip() const
{
  KTimeZone tz = mSpec.timeZone();

  QString toolTip;
  toolTip += "<qt>";
  toolTip += i18n( "Timezone: %1", tz.name() );
  toolTip += "<br/>";
  toolTip += i18n( "Country Code: %1", tz.countryCode() );
  if ( !tz.abbreviations().isEmpty() ) {
    toolTip += "<br/>";
    toolTip += i18n( "Abbreviations:" );
    foreach ( const QByteArray &a, tz.abbreviations() ) {
      toolTip += "<br/>";
      toolTip += "&nbsp;" + QString::fromLocal8Bit( a );
    }
  }
  if ( !tz.comment().isEmpty() ) {
    toolTip += "<br/>";
    toolTip += i18n( "Comment:<br/>%1", tz.comment() );
  }
  toolTip += "</qt>";

  return toolTip;
}

#include "timelabels.moc"
