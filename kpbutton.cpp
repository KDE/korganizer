/*   $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.troll.no and http://www.kde.org respectively
*/

#include <qdrawutl.h>
#include <qbitmap.h>

#include <kapp.h>
#include <klocale.h>

#include "kpbutton.h"
#include "kpbutton.moc"	

KPButton::KPButton( const QPixmap& pixmap,
		    QWidget *parent, const char *name) 
  : QToolButton( parent, name )
{
  resize(24,24);
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else {
    warning(i18n("KPButton: pixmap is empty, perhaps some missing file"));
    enabledPixmap.resize( 22, 22);
  };
  makeDisabledPixmap();
  QToolButton::setPixmap( enabledPixmap );
  right = false;
}

KPButton::KPButton( const QPixmap& pixmap, const char *text,
		    QWidget *parent, const char *name) 
  : QToolButton( parent, name )
{
  KPButton::text = text;
  resize(24,24);
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(i18n("KPButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( 22, 22);
    };
  makeDisabledPixmap();
  QToolButton::setPixmap( enabledPixmap );
  connect( this, SIGNAL( clicked() ),
           this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
  right = false;
}

void KPButton::leaveEvent(QEvent *e)
{
  if (isToggleButton() == FALSE)
    QToolButton::leaveEvent(e);
}

void KPButton::enterEvent(QEvent *e)
{
  if (isToggleButton() == FALSE)
    QToolButton::enterEvent(e);
}

void KPButton::on(bool flag)
{
  setOn(flag);
  repaint();
}

void KPButton::drawButton( QPainter *_painter )
{
  QColorGroup g = QWidget::colorGroup();
  if (isOn())
    qDrawShadePanel(_painter, 0, 0, width(), height(), g , TRUE, 2);
  QToolButton::drawButton(_painter);
}

void KPButton::toggle()
{
  setOn(!isOn());
  repaint();
}

void KPButton::setPixmap( const QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(i18n("KPButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( 22, 22);
    }
  // makeDisabledPixmap();
  QToolButton::setPixmap( enabledPixmap );
}

void KPButton::makeDisabledPixmap()
{
  QPalette pal = palette();
  QColorGroup g = pal.disabled();

  // Find the outline of the colored portion of the normal pixmap

  QBitmap *pmm = (QBitmap*) enabledPixmap.mask();
  QPixmap pm;
  if (pmm != 0L)
    {
      pmm->setMask( *pmm );
      pm = *pmm;
    }
  else
    {
      pm.resize(22 , 22);
      enabledPixmap.fill(this, 0, 0);
      // warning("KPButton::makeDisabledPixmap: mask is null.");
    };

  // Prepare the disabledPixmap for drawing

  disabledPixmap.resize(22,22);
  disabledPixmap.fill( g.background() );

  // Draw the outline found above in highlight and then overlay a grey version
  // for a cheesy 3D effect ! BTW. this is the way that Qt 1.2+ does it for
  // Windows style

  QPainter p;
  p.begin( &disabledPixmap );
  p.setPen( g.light() );
  p.drawPixmap(1, 1, pm);
  p.setPen( g.text() );
  p.drawPixmap(0, 0, pm);
  p.end();
}

void KPButton::paletteChange(const QPalette &)
{
  makeDisabledPixmap();
  if ( !isEnabled() )
    QToolButton::setPixmap( disabledPixmap );
  repaint( TRUE );
}

void KPButton::enable( bool enabled )
{
  QToolButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  setEnabled( enabled );
}
