#ifndef _XQGANTTLISTVIEWPORT_H_
#define _XQGANTTLISTVIEWPORT_H_

/*

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    author  : jh, jochen@ifb.bv.tu-berlin.de

    file    : xQGanttListViewPort.h
    date    : 26 oct 2000


    changelog :

*/



#include "xQGanttBarViewPort.h"
#include "xQTask.h"


#include <qcursor.h>



///  GanttListViewPort Widget.
/*!
 *
 */

////////////////////////////////////////////
class xQGanttListViewPort : public QFrame
////////////////////////////////////////////
{

  Q_OBJECT

  friend class xQGanttListView;
 

public:


  ///  Constructor.
  /*!
   *
   */
  xQGanttListViewPort(xQTask* maintask, QWidget* parent = 0,
		     const char * name=0, WFlags f=0 );



  ///  Destructor.
  /*!
   *
   */
  ~xQGanttListViewPort();
 


public slots:

  void barViewResized();


protected:

  ///  Update widget.
  /*!
   *
   */
  void update(int x1, int y1, int x2, int y2);


  ///
  /*!
   *
   */
  void setBarViewPort(xQGanttBarViewPort* v);


  void drawContents(QPainter*, int x1, int y1, int x2, int y2);
  void drawTask(xQTask*, QPainter* p, const QRect&, int);

  xQGanttBarViewPort* _barviewport;

  int _width;

  xQTask* _maintask;

  void paintEvent(QPaintEvent * e) {    
//    printf("xQGanttListViewPort::paintEvent()\n");
    update(e->rect().left(), e->rect().top(),
	   e->rect().right(), e->rect().bottom() );
  }

  QPopupMenu* _menu;

  void mousePressEvent(QMouseEvent* e) {

    if(e->button() == RightButton && e->state() == ControlButton ) {
      _menu->popup(e->globalPos());
      return;
    }
    
  }


  QBrush brush1, brush2;

  static int _ListViewCounter;

};


#endif
