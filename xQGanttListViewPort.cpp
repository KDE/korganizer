//
//  file    : xQGanttListViewPort.C
//  date    : 26 oct 2000
//  changed : 29 nov 2000
//  author  : jh
//

#include "xQGanttListViewPort.h"

#include <qcolor.h>


int xQGanttListViewPort::_ListViewCounter = 0;


xQGanttListViewPort::xQGanttListViewPort(xQTask* maintask, QWidget* parent,
					 const char * name, WFlags f)
  : QFrame(parent,name,f)
/////////////////////////////////////////////////////////
{
  _maintask = maintask;

  setBackgroundColor(QColor(white));

  _barviewport = NULL;

  _width = 1000;

  brush1 = QBrush(QColor(200,200,230));
  brush2 = QBrush(QColor(240,240,240));

}



xQGanttListViewPort::~xQGanttListViewPort()
/////////////////////////////////////////
{
}



void
xQGanttListViewPort::setBarViewPort(xQGanttBarViewPort* v)
{
  _barviewport = v;

  //  printf("setBarViewPort()\n");

  resize(500, _barviewport->height());

  printf("setBarViewPort()\n");

  connect(_barviewport, SIGNAL(resized()),
	  this, SLOT(barViewResized()));
  

  connect(_barviewport, SIGNAL(recalculated()),
	  this, SLOT(update()));
  
  /*
    connect(_barviewport, SIGNAL(contentsRepainted()),
    this, SLOT(barViewRepainted()));
  */
}



void 
xQGanttListViewPort::barViewResized()
//////////////////////////////////////
{
  printf("xQGanttListViewPort::barViewResized()\n");
  
  static int _h = 0;

  int h = _barviewport->height();

  if(h!=_h) {
    _h = h;
    resize(_width, _h);
  }

}



void 
xQGanttListViewPort::drawContents(QPainter* p, int x1, int y1, int x2, int y2)
//////////////////////////////////////////////////////////////////////////////
{
  /*printf("\nxQGanttListViewPort::drawContents(%d,%d,%d,%d)\n",
	 x1, y1, x2, y2 ); 
  */

  _ListViewCounter = 0;

  if(_barviewport) {
    drawTask(_maintask, p, QRect(x1, y1, x2-x1, y2-y1), 5 );
  }

}



void
xQGanttListViewPort::drawTask(xQTask* task, QPainter* p, const QRect& rect,
			      int offsetX )
/////////////////////////////////////////////////////////////////////////////
{
  static int margin = 2;

  xQTaskPosition* tpos = _barviewport->_gTaskList[task];

  if(!tpos) return;
  
  if( (tpos->_screenY+5 >= rect.y() &&
       tpos->_screenY-5 <= rect.y() + rect.height()) ||
      ((tpos->_screenY + tpos->_screenH)+5 >= rect.y() &&
       (tpos->_screenY + tpos->_screenH)-5 <= rect.y() + rect.height() ) ) {

    p->setPen(QPen(QColor(black)));
    
    int y = tpos->_screenY;
    int h = tpos->_screenH;
    
    if(tpos->_nr % 2 == 0)
      p->fillRect(0 + margin, y + margin ,
		  _width - 2 * margin, h - 2 * margin, brush1);
    else
      p->fillRect(0 + margin, y + margin, 
		  _width - 2* margin, h - 2* margin, brush2);
    
    QString str = task->getText() + "  [" + 
      task->getStart().toString() + " / " +
      task->getEnd().toString() + "]";
    
    p->drawText(offsetX, tpos->_textPosY, str );
    
  }

    
  if(task->isOpen() && task->getSubTasks().count()>0) {
    
    for(xQTask* subtask = task->getSubTasks().first(); 
	subtask != 0; 
	subtask = task->getSubTasks().next() ) {
      
      drawTask(subtask, p, rect, offsetX + 20);
      
    }
    
    p->setPen(QPen(QColor(blue),2));
    p->drawLine(offsetX + 3,  tpos->_textPosY + 3, 
		offsetX + 3,  tpos->_screenY + tpos->_screenHS - 3);

  }  

}


void 
xQGanttListViewPort::update(int x1, int y1, int x2, int y2)
/////////////////////////////////////////////////
{
  QPainter p(this);

  /*
    printf("\nxQGanttListViewPort::update(%d,%d,%d,%d)\n",
    x1, y1, x2, y2 );
  */
  drawContents(&p, x1, y1, x2, y2);
  
}

#include "xQGanttListViewPort.moc"
