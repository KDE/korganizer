#ifndef _XQGANTTBARVIEWPORT_H_
#define _XQGANTTBARVIEWPORT_H_

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

    file    : xQGanttBarViewPort.h
    date    : 26 oct 2000


    changelog :

*/



#include "xQTask.h"

#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qptrdict.h>
#include <qaction.h> 

#define sgn(n) (n < 0 ? -1 : 1)


// This is an internal class.
// helper for drawing tasks

class xQTaskPosition 
////////////////////
{

public :

  xQTaskPosition(int nr, int x, int y, int w, int h, int hiSub, int tP, int d) 
    :  _nr(nr), _screenX(x), _screenY(y), _screenW(w), _screenH(h), 
       _screenHS(hiSub), _textPosY(tP), _depth(d)
    {
      _screenHandleX = _screenHandleY = _screenHandleW = _screenHandleH = 0;
    }

  int _nr;
  int _screenX, _screenY, _screenW;
  int _screenH; // height without subtasks
  int _screenHS; // height including subtasks
  int _textPosY;

  int _screenHandleX, _screenHandleY, _screenHandleW, _screenHandleH;

  int _depth;
 
};



///  GanttBarViewPort Widget.
/*!
 *
 */

/////////////////////////////////////////
class xQGanttBarViewPort : public QFrame
////////////////////////////////////////
{

  Q_OBJECT

  friend class xQGanttBarView;
 
public:

  enum Mode {Select, Zoom, Move};


  ///  Constructor.
  /*!
   *
   */
  xQGanttBarViewPort(xQTask* maintask, QWidget* parent = 0,
		     const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~xQGanttBarViewPort();



  ///  Update widget.
  /*!
   *
   */
  void update(int x1, int y1, int x2, int y2); 
  

 
  QPtrDict<xQTaskPosition> _gTaskList;


  ///  Add holiday.
  /*!
   *   
   */
  void addHoliday(int y, int m, int d);



  ///  Remove holiday.
  /*!
   *
   */
  void removeHoliday(int , int , int ) {
  }



  QPopupMenu* menu() {
    return _menu;
  }



  ///
  /*!
   *
   */
  QToolBar* toolbar(QMainWindow* mw);


signals:

  void modeChanged(int);
  void scroll(int x, int y);
  void resized();
  void recalculated();
  void message(const QString& msg);


public slots:

  void setMode(int mode);

  void setSelect();
  void setZoom();
  void setMove();

  void popup(int index);

  void selectAll();
  void unselectAll();
 

private slots:  

  void mainTaskChanged(xQTask* task, xQTask::Change c);


private:


  ///  Transform world coordinate to screen coordinate.
  /*!
   *
   */
  inline int screenX(int wx);


  ///  Transform world coordinate to screen coordinate.
  /*!
   *
   */
  inline int screenY(int wy);

  
  ///  Transform screen coordinate to world coordinate.
  /*!
   *
   */
  inline int worldX(int sx);


  ///  Transform screen coordinate to world coordinate.
  /*!
   *
   */
  inline int worldY(int sy);


  void zoom(double sfactor, int wx, int wy);
  void zoomAll();

  enum Position { Outside = 0,
		  Handle  = 1,
		  North   = 2,
		  South   = 4,
		  West    = 8,
		  East    = 16,
		  Center  = 32 };

  int _grid, _snapgrid;
  bool _drawGrid, _drawHeader;

  Mode _mode;

  int _marginX, _marginY; // margin in minutes
  double _scaleX, _scaleY;

  int _margin;

  QCursor* _cursor_lupe;

  QLabel* _taskInfo;

  xQTask* _maintask;

  QPopupMenu* _menu;
  QToolBar* _toolbar;

  QPoint* _startPoint, *_endPoint;

  QList<QDate> _holidays;

  QList<QAction> _actions;


  /// 

  void initMenu(); 

  void drawGrid(QPainter*, int x1, int y1, int x2, int y2);
  void drawHeader(QPainter*, int x1, int y1, int x2, int y2);
  void drawTask(xQTask* task, QPainter* p, const QRect& rect );

  void recalc(xQTask* task, int xPos, int yPos, int depth, int nr );
  void recalc();

  void selectTask(xQTask*,bool);

  void adjustSize();

  Position check(xQTask** foundtask, int x, int y);

  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);  
  void keyPressEvent(QKeyEvent* e);
  void paintEvent(QPaintEvent * e);
  
};




//   inline


int xQGanttBarViewPort::screenX(int wx)
///////////////////////////////////////
{   
  return (int) (0.5 + (wx + _marginX)  * _scaleX);
}  
int xQGanttBarViewPort::screenY(int wy)
/////////////////////////////////////
{   
  return (int) (0.5 + (wy + _marginY) * _scaleY);
}  
int xQGanttBarViewPort::worldX(int sx)
/////////////////////////////////////
{   
  return (int) (0.5 + (sx/_scaleX - _marginX));
}  
int xQGanttBarViewPort::worldY(int sy)
//////////////////////////////////////
{   
  return (int) (0.5 + (sy/_scaleY - _marginY));
}  


#endif
