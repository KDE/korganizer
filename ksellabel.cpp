// 	$Id$	

#include <qapplication.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>

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
    if ( style().guiStyle() == WindowsStyle )
      fc = QApplication::winStyleHighlightColor();
    else
      fc = colorGroup().text();
    p.fillRect(cr, fc );
    p.setPen( style().guiStyle() == WindowsStyle ? white : colorGroup().base() );
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

