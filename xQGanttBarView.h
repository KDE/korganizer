#ifndef _XQGANTTBARVIEW_H_
#define _XQGANTTBARVIEW_H_
 
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

    file    : xQGanttBarView.h
    date    : 26 oct 2000


    changelog :

*/


#define sgn(n) (n < 0 ? -1 : 1)
#define TOPMARGIN 45


#include <qscrollview.h>

#include "xQGanttBarViewPort.h"




///  Gantt view.
/*!
 *   Widget for drawing gantt diagrams.
 */

//////////////////////////////////////////
class xQGanttBarView : public QScrollView
//////////////////////////////////////////
{

  Q_OBJECT


public:


  ///  Constructor.
  /*!
   *
   */
  xQGanttBarView(xQTask* maintask, QWidget* parent = 0,  
		 const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~xQGanttBarView();



  ///
  /*!
   *
   */
  xQGanttBarViewPort* viewport() {
    return _viewport;
  }



public slots:

  void horizontalScrollBarChanged(int);
 

protected slots:
  
   void drawHeader();


protected:

  xQGanttBarViewPort* _viewport;

  //  ptr to maintask
  xQTask* _task;

  QBrush _headerBackBrush;

  void paintEvent(QPaintEvent * e);
  
};



#endif
