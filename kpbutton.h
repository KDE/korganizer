/* 	$Id$	 */

#ifndef _KPBUTTON_H 
#define _KPBUTTON_H

#include <qpixmap.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qevent.h>
#include <qtoolbutton.h>

/**
 * This is an internal class for use in control
 *  note: this was a clone of KToolBarButton.
 *
 * @short Internal Button class for control
 */
class KPButton : public QToolButton
{
  Q_OBJECT
    
 public:
   KPButton(const QPixmap& pixmap, QWidget *parent,
	    const char *name=0L);
   KPButton(const QPixmap& pixmap, const char *text, QWidget *parent,
	    const char *name=0L);
   void enable(bool enable);
   void makeDisabledPixmap();
   QPixmap disabledPixmap;
   virtual void setPixmap( const QPixmap & );
   bool isRight () {return right;};
   void alignRight (bool flag) {right = flag;};
   void on(bool flag);
   void toggle();

 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);

 protected:
   QPixmap enabledPixmap;
   const char *text;
   bool right;

 protected slots:

 signals:
};

#endif
