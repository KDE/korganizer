/*
    This file is part of KOrganizer.

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qapplication.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qstyle.h>

#include "ksellabel.h"
#include "ksellabel.moc"

#define MARGIN 3

KSelLabel::KSelLabel(QWidget    *parent, 
		     const QString & text,
		     int         idx,
		     const char *name) 
  : QFrame(parent, name)
{
  setFocusPolicy(QWidget::ClickFocus);
  labeltext = text;
  index = idx;
  alignment = AlignRight | AlignVCenter;
  act = FALSE;
  updateLabel();
  setBackgroundMode(PaletteBase);
}


KSelLabel::~KSelLabel()
{
}

QString KSelLabel::text()
{
  return(labeltext);
}

void KSelLabel::setText(const QString & s)
{
  labeltext = s;
  updateLabel();
}

void KSelLabel::setActivated(bool activated)
{
  act = activated;
  updateLabel();
}

void KSelLabel::setAlignment(int align)
{
  alignment = align;
  updateLabel();
}

void KSelLabel::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  QRect cr = contentsRect();

  p.eraseRect(cr);
  if(act) {
    QColor   fc;                            // fill color
    if ( style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      fc = QApplication::winStyleHighlightColor();
    else
      fc = colorGroup().text();
    p.fillRect(cr, fc );
    p.setPen( style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle ? white : colorGroup().base() );
    p.setBackgroundColor( fc );
  }

  cr.setLeft(cr.left()+MARGIN);
  cr.setRight(cr.right()-MARGIN);
  cr.setTop(cr.top()+MARGIN);
  cr.setBottom(cr.bottom()-MARGIN);
  p.drawText( cr.x(), cr.y(), cr.width(), cr.height(), 
	       alignment, labeltext, -1 );
}

void KSelLabel::focusInEvent(QFocusEvent *)
{
  emit labelActivated(index);
}

void KSelLabel::mouseDoubleClickEvent(QMouseEvent *m)
{
  if (m->button() == LeftButton) {
    emit newEventSignal(index);
  }
}
    
void KSelLabel::updateLabel()
{
  //  paintEvent(&QPaintEvent(rect()));
  repaint();
}

QSize KSelLabel::sizeHint() const
{
    QPainter p(this);
    QRect br;
    br = p.boundingRect( 0,0, 1000,1000, alignment, labeltext );
    return QSize(br.width() + 2*MARGIN + 2*frameWidth(),
		 br.height() + 2*MARGIN + 2*frameWidth());
}

