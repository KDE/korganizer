#ifndef _XQGANTTLISTVIEW_H_
#define _XQGANTTLISTVIEW_H_
 
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

    file    : xQGanttListView.h
    date    : 23 nov 2000


    changelog :

*/




#include "xQGanttBarView.h"
#include "xQGanttListViewPort.h"

#include <qscrollview.h>



///  GanttListView Widget.
/*!
 *
 */

//////////////////////////////////////////////
class xQGanttListView : public QScrollView
//////////////////////////////////////////////
{

  Q_OBJECT


public:


  ///  Constructor.
  /*!
   *
   */
  xQGanttListView(xQTask* maintask, QWidget* parent = 0,  
		  const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~xQGanttListView();



  ///  Connect barview to this listview.
  /*!
   *
   */
  void setBarView(xQGanttBarView* v) {
    ((xQGanttListViewPort*) _viewport)->setBarViewPort(v->viewport());
  }



public slots:

  void contentsMoved(int,int);


protected:

  //  ptr to maintask
  xQTask* _task;

  QBrush _headerBackBrush;
 
  xQGanttBarView* _barview;
  xQGanttListViewPort* _viewport;

  void drawHeader();
  void paintEvent(QPaintEvent * e);

};





#endif
