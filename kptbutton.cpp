#include <qpainter.h>
#include <qdrawutl.h>

#include "kptbutton.h"
#include "kptbutton.moc"

KPTButton::KPTButton( QWidget *_parent, const char *name )
    : QButton( _parent , name)
{
  raised = FALSE;
  setFocusPolicy( NoFocus );
  setText(0);
  setAutoResize( TRUE );
}

void KPTButton::setText(const QString &t)
{
  textLabel = t;
  repaint();
}

QString KPTButton::text() const
{
  return textLabel;
}

QSize KPTButton::sizeHint() const
{
  int w=0, h=0;

  if (pixmap()) {
    w = 5 + pixmap()->width();
    h = 5 + pixmap()->height() ;
  }

  if (text()) {
    w += fontMetrics().width(text()) + 10;
    if (fontMetrics().height() > h)
      h = fontMetrics().height();
  }
  if (w > 0 && h > 0)
    return QSize(w, h);
  else
    return QSize(1,1);
}

void KPTButton::enterEvent( QEvent* )
{
  if ( isEnabled() )
    {
      raised = TRUE;
      repaint(FALSE);
    }
}

void KPTButton::leaveEvent( QEvent * )
{
  if( raised != FALSE )
    {
      raised = FALSE;
      repaint();
    }
}
    
void KPTButton::drawButton( QPainter *_painter )
{
  paint( _painter );
}

void KPTButton::drawButtonLabel( QPainter *_painter )
{
  paint( _painter );
}

void KPTButton::paint( QPainter *painter )
{
  int dx=0, dy=0;
  QFontMetrics fm(fontMetrics());

  if ( isDown() || isOn() )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), 
			height(), colorGroup(), TRUE );
      else
	qDrawShadePanel( painter, 0, 0, width(), 
			 height(), colorGroup(), TRUE, 2, 0L );
    }
  else if ( raised )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), height(), 
			colorGroup(), FALSE );
      else
	qDrawShadePanel( painter, 0, 0, width(), height(), 
			 colorGroup(), FALSE, 2, 0L );
    }
  
  if ( pixmap() )
    {
      //dx = ( width() - pixmap()->width() ) / 2;
      dx = 5;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle ) {
	dx++;
	dy++;
      }
      painter->drawPixmap( dx, dy, *pixmap() );
    }
  if (text()) {
    dx = dx + (pixmap() ? pixmap()->width() : 0) + 5;
    dy = dy + (pixmap() ? (pixmap()->height() / 2) : height()/2-2) + 
      (fm.boundingRect(text()).height() / 2);
    painter->drawText(dx, dy, text());
  }
}


KPTButton::~KPTButton()
{
}

