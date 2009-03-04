/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

#include "monthgraphicsitems.h"
#include "monthitem.h"
#include "monthscene.h"

#include "koprefs.h"
#include "kohelper.h"

#include <QPainter>
#include <QGraphicsScene>

using namespace KOrg;

ScrollIndicator::ScrollIndicator( ScrollIndicator::ArrowDirection dir )
  : mDirection( dir )
{
  setZValue( 200 ); // on top of everything
  hide();
}

QRectF ScrollIndicator::boundingRect() const
{
  return QRectF( - mWidth / 2, - mHeight / 2, mWidth, mHeight );
}

void ScrollIndicator::paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );

  painter->setRenderHint( QPainter::Antialiasing );

  QPolygon arrow( 3 );
  if ( mDirection == ScrollIndicator::UpArrow ) {
    arrow.setPoint( 0, 0, - mHeight / 2 );
    arrow.setPoint( 1, mWidth / 2, mHeight / 2 );
    arrow.setPoint( 2, - mWidth / 2, mHeight / 2 );
  } else if ( mDirection == ScrollIndicator::DownArrow ) { // down
    arrow.setPoint( 1, mWidth / 2, - mHeight / 2 );
    arrow.setPoint( 2, - mWidth / 2, - mHeight / 2 );
    arrow.setPoint( 0, 0, mHeight / 2 );
  }
  QColor color( Qt::black );
  color.setAlpha( 155 );
  painter->setBrush( color );
  painter->setPen( color );
  painter->drawPolygon( arrow );
}

//-------------------------------------------------------------
MonthCell::MonthCell( int id, QDate date, QGraphicsScene *scene )
  : mId( id ), mDate( date ), mScene( scene )
{
  mUpArrow = new ScrollIndicator( ScrollIndicator::UpArrow );
  mDownArrow = new ScrollIndicator( ScrollIndicator::DownArrow );
  mScene->addItem( mUpArrow );
  mScene->addItem( mDownArrow );
}

MonthCell::~MonthCell()
{
  mScene->removeItem( mUpArrow );
  mScene->removeItem( mDownArrow );
  delete mUpArrow; // we've taken ownership, so this is safe
  delete mDownArrow;
}

bool MonthCell::hasEventBelow( int height )
{
  if ( mHeightHash.isEmpty() ) {
    return false;
  }

  for ( int i = 0; i < height; ++i ) {
    if ( mHeightHash.value( i ) != 0 ) {
      return true;
    }
  }

  return false;
}

int MonthCell::topMargin()
{
  return 18;
}

void MonthCell::addMonthItem( MonthItem *manager, int height )
{
  mHeightHash[ height ] = manager;
}

int MonthCell::firstFreeSpace()
{
  MonthItem *manager = 0;
  int i = 0;
  while ( true ) {
    manager = mHeightHash[ i ];
    if ( manager == 0 ) {
      return i;
    }
    i++;
  }
}

//-------------------------------------------------------------
// MONTHGRAPHICSITEM
static const int ft = 2; // frame thickness

MonthGraphicsItem::MonthGraphicsItem( MonthItem *manager )
  : QGraphicsItem( 0, manager->monthScene() ),
    mMonthItem( manager )
{
  QTransform transform;
  transform = transform.translate( 0.5, 0.5 );
  setTransform( transform );

  setToolTip( mMonthItem->toolTipText() );
}

MonthGraphicsItem::~MonthGraphicsItem()
{
}

bool MonthGraphicsItem::isMoving() const
{
  return mMonthItem->isMoving();
}

bool MonthGraphicsItem::isEndItem() const
{
  return startDate().addDays( daySpan() ) == mMonthItem->endDate();
}

bool MonthGraphicsItem::isBeginItem() const
{
  return startDate() == mMonthItem->startDate();
}

QPainterPath MonthGraphicsItem::shape() const
{
  // The returned shape must be a closed path,
  // otherwise MonthScene:itemAt( pos ) can have
  // problems detecting the item
  return widgetPath( false );
}

// TODO: remove this method.
QPainterPath MonthGraphicsItem::widgetPath( bool border ) const
{
  // If border is set we won't draw all the path. Items spanning on multiple
  // rows won't have borders on their boundaries.
  // If this is the mask, we draw it one pixel bigger
  int x0 = 0;
  int y0 = 0;
  int x1 = boundingRect().width();
  int y1 = boundingRect().height();

  int height = y1 - y0;
  int beginRound = height / 3;

  QPainterPath path( QPoint( x0 + beginRound, y0 ) );
  if ( isBeginItem() ) {
    path.arcTo( QRect( x0, y0, beginRound * 2, height ), +90, +180 );
  } else {
    path.lineTo( x0, y0 );
    if ( !border ) {
      path.lineTo( x0, y1 );
    } else {
      path.moveTo( x0, y1 );
    }
    path.lineTo( x0 + beginRound, y1 );
  }

  if ( isEndItem() ) {
    path.lineTo( x1 - beginRound, y1 );
    path.arcTo( QRect( x1 - 2 * beginRound, y0, beginRound * 2, height ), -90, +180 );
  } else {
    path.lineTo( x1, y1 );
    if ( !border ) {
      path.lineTo( x1, y0 );
    } else {
      path.moveTo( x1, y0 );
    }
  }

  // close path
  path.lineTo( x0 + beginRound, y0 );

  return path;
}

QRectF MonthGraphicsItem::boundingRect() const
{
  // width - 2 because of the cell-dividing line with width == 1 at beginning and end
  return QRectF( 0,  0, ( daySpan() + 1 ) * mMonthItem->monthScene()->columnWidth() - 2,
                 mMonthItem->monthScene()->itemHeight() );
}

void MonthGraphicsItem::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( !mMonthItem->monthScene()->initialized() ) {
    return;
  }

  MonthScene *scene = mMonthItem->monthScene();

  p->setRenderHint( QPainter::Antialiasing );

  int textMargin = 10;

  QColor bgColor = mMonthItem->bgColor();
  // keep this (110) in sync with agendaview
  bgColor = mMonthItem->selected() ? bgColor.lighter( 110 ) : bgColor;
  QColor frameColor = mMonthItem->frameColor( bgColor );
  frameColor = mMonthItem->selected() ? frameColor.lighter( 110 ) : frameColor;
  QColor textColor = getTextColor(bgColor);

  // make moving or resizing items translucent
  if ( mMonthItem->isMoving() || mMonthItem->isResizing() ) {
    bgColor.setAlphaF( 0.75f );
  }

  QLinearGradient gradient( 0, 0, 0, boundingRect().height() );
  gradient.setColorAt( 0, bgColor );
  gradient.setColorAt( 0.7, bgColor.dark( 110 ) );
  gradient.setColorAt( 1, bgColor.dark( 150 ) );
  QBrush brush( gradient );
  p->setBrush( brush );
  p->setPen( Qt::NoPen );
  // Rounded rect without border
  p->drawPath( widgetPath( false ) );

  // Draw the border without fill
  QPen pen( frameColor );
  pen.setWidth( ft );
  p->setPen( pen );
  p->setBrush( Qt::NoBrush );
  p->drawPath( widgetPath( true ) );

  p->setPen( textColor );

  int alignFlag = Qt::AlignVCenter;
  if ( isBeginItem() ) {
    alignFlag |= Qt::AlignLeft;
  } else if ( isEndItem() ) {
    alignFlag |= Qt::AlignRight;
  } else {
    alignFlag |= Qt::AlignHCenter;
  }

  // !isBeginItem() is not always isEndItem()
  QString text = mMonthItem->text( !isBeginItem() );
  p->setFont( KOPrefs::instance()->monthViewFont() );

  // Every item should set its own LayoutDirection, or eliding fails miserably
  p->setLayoutDirection( text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight );

  QRect textRect = QRect( textMargin, 0,
                          boundingRect().width() - 2 * textMargin, scene->itemHeight() );

  if ( KOPrefs::instance()->enableMonthItemIcons() ) {
    QList<QPixmap *> icons = mMonthItem->icons();
    int iconWidths = 0;

    foreach ( QPixmap *icon, icons ) {
      iconWidths += icon->width();
    }

    if ( !icons.isEmpty() ) {
      // add some margin between the icons and the text
      iconWidths += textMargin / 2;
    }

    int textWidth = p->fontMetrics().size( 0, text ).width();
    if ( textWidth + iconWidths > textRect.width() ) {
      textWidth = textRect.width() - iconWidths;
      text = p->fontMetrics().elidedText( text, Qt::ElideRight, textWidth );
    }

    int curXPos = textRect.left();
    if ( alignFlag & Qt::AlignRight ) {
      curXPos += textRect.width() - textWidth - iconWidths ;
    } else if ( alignFlag & Qt::AlignHCenter ) {
      curXPos += ( textRect.width() - textWidth - iconWidths ) / 2;
    }
    alignFlag &= ~( Qt::AlignRight | Qt::AlignCenter );
    alignFlag |= Qt::AlignLeft;

    // update the rect, where the text will be displayed
    textRect.setLeft( curXPos + iconWidths );

    // assume that all pixmaps have the same height
    int pixYPos = icons.isEmpty() ? 0 : ( textRect.height() - icons[0]->height() ) / 2;
    foreach ( QPixmap *icon, icons ) {
      p->drawPixmap( curXPos, pixYPos, *icon );
      curXPos += icon->width();
    }

    p->drawText( textRect, alignFlag, text );
  } else {
    text = p->fontMetrics().elidedText( text, Qt::ElideRight, textRect.width() );
    p->drawText( textRect, alignFlag, text );
  }

}

void MonthGraphicsItem::setStartDate( const QDate &date )
{
  mStartDate = date;
}

QDate MonthGraphicsItem::endDate() const
{
  return startDate().addDays( daySpan() );
}

QDate MonthGraphicsItem::startDate() const
{
  return mStartDate;
}

void MonthGraphicsItem::setDaySpan( int span )
{
  mDaySpan = span;
}

int MonthGraphicsItem::daySpan() const
{
  return mDaySpan;
}

void MonthGraphicsItem::updateGeometry()
{
  MonthCell *cell =  mMonthItem->monthScene()->mMonthCellMap.value( startDate() );

  // If the item is moving and this one is moved outside the view,
  // cell will be null
  if ( mMonthItem->isMoving() && !cell ) {
    hide();
    return;
  }

  Q_ASSERT( cell );

  prepareGeometryChange();

  int beginX = 1 + mMonthItem->monthScene()->cellHorizontalPos( cell );
  int beginY = 1 + cell->topMargin() + mMonthItem->monthScene()->cellVerticalPos( cell );

  beginY += mMonthItem->position() *
            mMonthItem->monthScene()->itemHeightIncludingSpacing() -
            mMonthItem->monthScene()->startHeight() *
            mMonthItem->monthScene()->itemHeightIncludingSpacing(); // scrolling

  setPos( beginX, beginY );

  if ( mMonthItem->position() < mMonthItem->monthScene()->startHeight() ||
       mMonthItem->position() - mMonthItem->monthScene()->startHeight() >=
          mMonthItem->monthScene()->maxRowCount() ) {
    hide();
  } else {
    show();
    update();
  }
}
