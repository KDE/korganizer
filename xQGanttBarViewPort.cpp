//
//  file    : xQGanttBarViewPort.C
//  date    : 26 oct 2000
//  changed : 27 dec 2000
//  author  : jh
//

#include "xQGanttBarViewPort.h"

#include <qcolor.h>
#include <qtoolbutton.h>

#include "lupe.xpm"


xQGanttBarViewPort::xQGanttBarViewPort(xQTask* maintask, QWidget* parent = 0,
				       const char * name=0, WFlags f=0 )
  : QFrame(parent,name,f)
/////////////////////////////////////////////////////////////////////////////
{
  _gTaskList = QPtrDict<xQTaskPosition>(449);
  _gTaskList.setAutoDelete(true);

  _maintask = maintask;

  _taskInfo = new QLabel(this);
  _taskInfo->setBackgroundColor(QColor(235,235,255));
  _taskInfo->setFrameStyle( Panel | Sunken ); 
  _taskInfo->setMargin( 5 );
  _taskInfo->setLineWidth(1);
  _taskInfo->hide();
 
  initMenu();

  setBackgroundColor(QColor(white));        
  
  _grid = 1440;
  _snapgrid = 360;

  _drawGrid = true;
  _drawHeader = false;
  
  _marginX = 10 * 1440;
  _marginY = 50;

  _scaleX = 0.1;
  _scaleY = 1;

  _margin = 1; // margin task in pixel

  _startPoint = new QPoint();  _endPoint = new QPoint();

  _cursor_lupe = new QCursor( QPixmap(lupe) );
 
  connect(_maintask, SIGNAL(changed(xQTask*, xQTask::Change)),
	  this, SLOT(mainTaskChanged(xQTask*, xQTask::Change)) );

  recalc(); adjustSize();

  setFocusPolicy(QWidget::StrongFocus);
  _mode = -1;

}



xQGanttBarViewPort::~xQGanttBarViewPort()
/////////////////////////////////////////
{
}



QToolBar*
xQGanttBarViewPort::toolbar(QMainWindow* mw)
/////////////////////////////////////////////
{
  _toolbar = new QToolBar(mw);
  mw->addToolBar(_toolbar);
  
  QAction* action;

  for(action = _actions.first(); 
      action != 0; 
      action = _actions.next() ) {

    action->addTo(_toolbar);
    if(action->text() == "Move")
      _toolbar->addSeparator();

  }

  return _toolbar;

}



void
xQGanttBarViewPort::initMenu()
/////////////////////////////////
{
  QAction* action;

  _menu = new QPopupMenu(this);

  //  select
  action = new QAction;
  _actions.append(action);
  action->setText("Select");
  action->addTo(_menu);
 
  QPixmap selectPixL("icons/22x22/select.png");
  if(selectPixL.isNull()) printf("select.png not found !\n");
  QPixmap selectPixS("icons/16x16/select.png");
  if(selectPixS.isNull()) printf("select.png not found !\n");
  QIconSet iconset(selectPixS, selectPixL );
  action->setIconSet(iconset);

  connect(action,SIGNAL(activated()),
	  this, SLOT(setSelect()));


  //  zoom
  action = new QAction;
  _actions.append(action);
  action->setText("Zoom");
  action->addTo(_menu);

  QPixmap zoomPixL("icons/22x22/viewmag.png");
  if(zoomPixL.isNull()) printf("viewmag.png not found !\n");
  QPixmap zoomPixS("icons/16x16/viewmag.png");
  if(zoomPixS.isNull()) printf("viewmag.png not found !\n");
  QIconSet iconsetZoom(zoomPixS, zoomPixL );
  action->setIconSet(iconsetZoom);

  connect(action,SIGNAL(activated()),
	  this, SLOT(setZoom()));

  //  move
  action = new QAction;
  _actions.append(action);
  action->setText("Move");
  action->addTo(_menu);

  QPixmap movePixL("icons/22x22/move.png");
  if(movePixL.isNull()) printf("move.png not found !\n");
  QPixmap movePixS("icons/16x16/move.png");
  if(movePixS.isNull()) printf("move.png not found !\n");
  QIconSet iconsetMove(movePixS, movePixL );
  action->setIconSet(iconsetMove);

  connect(action,SIGNAL(activated()),
	  this, SLOT(setMove()));


  _menu->insertSeparator();

  //  select all  
  action = new QAction;
  _actions.append(action);
  action->setText("Select all");
  action->addTo(_menu);

  QPixmap selectTaskPixL("icons/22x22/selectTask.png");
  if(selectTaskPixL.isNull()) printf("selectTask.png not found !\n");
  QPixmap selectTaskPixS("icons/16x16/selectTask.png");
  if(selectTaskPixS.isNull()) printf("selectTask.png not found !\n");
  QIconSet iconsetSelectTask(selectTaskPixS, selectTaskPixL );
  action->setIconSet(iconsetSelectTask);

  connect(action,SIGNAL(activated()),
	  this, SLOT(selectAll()));

 //  unselect all  
  action = new QAction;
  _actions.append(action);
  action->setText("Unselect all");
  action->addTo(_menu);

  QPixmap unselectTaskPixL("icons/22x22/unselectTask.png");
  if(unselectTaskPixL.isNull()) printf("unselectTask.png not found !\n");
  QPixmap unselectTaskPixS("icons/16x16/unselectTask.png");
  if(unselectTaskPixS.isNull()) printf("unselectTask.png not found !\n");
  QIconSet iconsetUnselectTask(unselectTaskPixS, unselectTaskPixL );
  action->setIconSet(iconsetUnselectTask);

  connect(action,SIGNAL(activated()),
	  this, SLOT(unselectAll()));

  _menu->insertSeparator();
  _menu->insertItem("Configure", 10);

}



void
xQGanttBarViewPort::mainTaskChanged(xQTask* task, xQTask::Change c)
///////////////////////////////////////////////////////////////////
{
  // printf("xQGanttBarViewPort::mainTaskChanged(), change = %d \n", c );
  recalc();
  adjustSize();
}



void 
xQGanttBarViewPort::adjustSize()
//////////////////////////////////
{
  // printf("xQGanttBarViewPort::adjustSize()\n");

  static int sw = 0;
  static int sh = 0;

  int w = screenX(_maintask->getWidth() + _marginX);
  int h = screenY(_maintask->getTotalHeight() + _marginY);

  if(sw != w || sh !=h) {

    sw = w;
    sh = h;

    resize(w,h);

    emit resized();

  }

}



void 
xQGanttBarViewPort::update(int x1, int y1, int x2, int y2)
//////////////////////////////////////////////////////////
{
  QPainter p(this);

  printf("xQGanttBarViewPort::update(%d,%d,%d,%d)\n",
	 x1, y1, x2, y2 );

  // QTime time1 = QTime::currentTime();
  
  if(_drawGrid)
    drawGrid(&p, x1, y1, x2, y2);

  // QTime time2 = QTime::currentTime();  
  // printf("%d msec for drawing grid.\n", time1.msecsTo( time2 ) );

  // drawContents(&p, x1, y1, x2, y2);
  drawTask(_maintask, &p, QRect(x1, y1, x2-x1, y2-y1) );

  // time1 = QTime::currentTime();  
  // printf("%d msec for drawing contents.\n", time2.msecsTo( time1 ) );
  
  if(_drawHeader)
    drawHeader(&p, x1, y1, x2, y2);
  
  // time2 = QTime::currentTime();<  
  // printf("%d msec for drawing header.\n", time1.msecsTo( time2 ) );
  
}




void
xQGanttBarViewPort::drawGrid(QPainter* p, int x1, int y1, int x2, int y2)
////////////////////////////////////////////////////////////////
{
  y2 += 5; // avoid white lines at bottom of redrawn region

  static int a, w, end, tmp;
  static QBrush _sat( QColor(200,200,200));
  static QBrush _sun( QColor(255,110,110));
  static QBrush _hol( QColor(200,200,250));
  static QPen penDay( QColor(235,235,235), 0, DotLine);
  static QPen penMonth( QColor(0,150,0), 3, DashDotLine);
  static QPen penHour( QColor(0,0,150), 0, DashDotLine);

  QDate start( _maintask->getStart().addSecs(worldX(x1)*60).date() );

  end = (int) ((x2-x1)/(1440.*_scaleX))+1;
  w = (int) (1440. * _scaleX + 0.5);

  //  draw holydays

  QDate* ptrDate;
  QDate cmp(start.addDays(-1));

  for(ptrDate = _holidays.first(); 
      ptrDate != 0; 
      ptrDate = _holidays.next() ) {
    
    if(*ptrDate > cmp) {
      tmp = _maintask->getStart().secsTo(*ptrDate)/60;
      a = screenX( tmp );
      p->fillRect( a, y1, w, y2, _hol );
    }
    
  }

  //  draw grid
 
  for(int i=0; i<=end; i++, start = start.addDays(1) ) {

    int dayOfWeek = start.dayOfWeek();
    tmp = _maintask->getStart().secsTo(start)/60;
    a = screenX( tmp );
   
    //  draw saturday
    if(dayOfWeek == 6) {

      p->fillRect( a, y1, w, y2, _sat );
      
      if(start.day() == 1) {
	p->setPen( penMonth );
	p->drawLine( a, y1, a, y2);
      }

      // continue;
    }

    //  sunday
    if(dayOfWeek == 7) {

      p->fillRect( a, y1, w, y2, _sun );

      if(start.day() == 1) {
	p->setPen( penMonth );
	p->drawLine( a, y1, a, y2);
      }

      // continue;
    }

    if(start.day() == 1) 
      p->setPen( penMonth );
    else {
      if(dayOfWeek == 1 || dayOfWeek == 6 || dayOfWeek == 7) 
	continue;
      p->setPen( penDay );
    }

    p->drawLine( a, y1, a, y2);

  }
}



void
xQGanttBarViewPort::recalc()
////////////////////////////
{
  // printf("xQGanttBarViewPort::recalc()\n");  
  _gTaskList.clear();
  recalc(_maintask, screenX(0), screenY(0), 0, 0 );
  emit recalculated();
}



void 
xQGanttBarViewPort::recalc(xQTask* task, int xPos, int yPos, 
			   int depth, int nr)
///////////////////////////////////////////////////////////////////////
{
  int tmpTotalHeight = task->getTotalHeight();
  int tmpHeight      = task->getHeight();

  int _screenW = (int) ((double) task->getWidth() * _scaleX);
  int _screenHS = (int) ((double) tmpTotalHeight * _scaleY);
  int _screenH  = (int) (tmpHeight * _scaleY);
  int _textPosY = yPos + (int) (0.65 * (double) tmpHeight * _scaleY);

  xQTaskPosition* tpos =  
    new xQTaskPosition(nr, xPos, yPos, _screenW, _screenH, _screenHS,
		       _textPosY, depth);
  
  _gTaskList.replace(task, tpos );

  int dd = (int) (0.25 * (double) tmpHeight * _scaleY);

  tpos->_screenHandleX = xPos + dd;
  tpos->_screenHandleW = 2 * dd;
  tpos->_screenHandleY = yPos + dd;
  tpos->_screenHandleH = 2 * dd;
  

  //  recalc subtasks
  
  if(task->isOpen()) {

    int h = tmpHeight;

    for(xQTask* subtask = task->getSubTasks().first(); 
	subtask != 0; 
	subtask = task->getSubTasks().next() ) {
      
      recalc(subtask, 
	     xPos + (task->getStart().secsTo(subtask->getStart())/60 * _scaleX), 
	     yPos + h * _scaleY, depth + 1, ++nr );

      h += subtask->getTotalHeight();
      
    }
  }         
 
}



void
xQGanttBarViewPort::drawTask(xQTask* task, QPainter* p, 
			     const QRect& rect )
/////////////////////////////////////////////////////////////////
{
  xQTaskPosition* tpos = _gTaskList[task];

  if(!tpos) return;

  if(tpos->_screenX > (rect.x() + rect.width())) return;
  if((tpos->_screenX + tpos->_screenW) < rect.x()) return;
  if(tpos->_screenY > (rect.y() + rect.height()) ) return; 
  if((tpos->_screenY + tpos->_screenHS) < rect.y()) return;

  p->setPen(task->getPen());

  int style = task->getStyle();

  if(style & xQTask::DrawFilled ) {
    
    p->fillRect(tpos->_screenX, tpos->_screenY + _margin,
		tpos->_screenW, tpos->_screenHS - 2 * _margin,
		task->getBrush() );
  
  }

  if(style & xQTask::DrawBorder ) {
    
    p->drawRect(tpos->_screenX, tpos->_screenY + _margin,
		tpos->_screenW, tpos->_screenHS - 2 * _margin );
    
  }

  if(task->isOpen()) {

    for(xQTask* subtask = task->getSubTasks().first(); 
	subtask != 0; 
	subtask = task->getSubTasks().next() ) {
      
      drawTask(subtask, p, rect );
      
    }
  }      

  p->setPen(task->getPen());
  if(style & xQTask::DrawHandle || 
     ((style & xQTask::DrawHandleWSubtasks) && task->getSubTasks().count()>0) ) {
       
    p->fillRect(tpos->_screenHandleX, tpos->_screenHandleY, 
		tpos->_screenHandleW, tpos->_screenHandleH,
		QBrush("steelblue"));
    
    p->drawRect(tpos->_screenHandleX, tpos->_screenHandleY, 
		tpos->_screenHandleW, tpos->_screenHandleH);

  }
 
  
  if(style & xQTask::DrawText ) {
    p->setPen(task->getTextPen());
    p->drawText(tpos->_screenX + tpos->_screenHandleW + 10 ,
		tpos->_textPosY, task->getText() );
  }

  if(task->isSelected()) {

    p->setPen( QPen(QColor(red),1));

    p->drawRect(tpos->_screenX, tpos->_screenY,
		tpos->_screenW, tpos->_screenHS );

    p->fillRect(tpos->_screenX, tpos->_screenY, 6, 6,
		task->getSelectBrush() );

    p->fillRect(tpos->_screenX + tpos->_screenW - 6, 
		tpos->_screenY, 6, 6,
		task->getSelectBrush() );

    p->fillRect(tpos->_screenX + tpos->_screenW - 6, 
		tpos->_screenY + tpos->_screenHS - 6, 6, 6,
		task->getSelectBrush() );

    p->fillRect(tpos->_screenX, 
		tpos->_screenY + tpos->_screenHS - 6, 6, 6,
		task->getSelectBrush() );
  }

}



void 
xQGanttBarViewPort::drawHeader(QPainter* p, int x1, int y1, int x2, int y2)
//////////////////////////////////////////////////////////////////////////
{
  bool drawDays = false;
  int a,e,tmp;

  QDate start( _maintask->getStart().addSecs(-_marginX * 60 ).date() );
  
  // subtract 1 month to draw first month
  QDate t(start.year(), start.month()-1, start.day() ); 

  QDateTime taskstart = _maintask->getStart();

  int end = (int) (width()/(1440*_scaleX));

  if(end < 12) drawDays = true;

  end += 30; // add 30 days to draw last month

  p->setPen( QPen(QColor(black)) );

  for(int i=0; i<=end; i++, t = t.addDays(1) ) {

    tmp = taskstart.secsTo(t)/60;
    a = screenX( tmp );

    if(t.dayOfWeek() == 1) {
	
      p->fillRect(a, 0, 1440*5*_scaleX, 20, QBrush(QColor(240,240,240)));
      p->drawRect(a, 0, 1440*5*_scaleX, 20 );

      if(!drawDays)
	p->drawText(a+5, 15, QString::number(t.day()) );
    }

    if(drawDays) {
      p->drawText(a+5, 15, t.dayName(t.dayOfWeek()) + " " + QString::number(t.day()) );
    }

    if(t.day()==1) {

      e = t.daysInMonth();

      p->fillRect(a, 21, 1440*e*_scaleX, 20, QBrush(QColor(240,240,240))); 
      p->drawRect(a, 21, 1440*e*_scaleX, 20 );

      if(a<0) a = 0;
      p->drawText(a+5, 36, t.monthName(t.month()) );        

    }

  } 
}



void
xQGanttBarViewPort::setMode(int mode) 
/////////////////////////////
{
  printf("setMode( %d )\n", mode );

  if(_mode == (Mode) mode) {
    return;
  }

  _mode = (Mode) mode;

  switch(_mode) {

  case Select:

    setSelect();
    break;


  case Zoom:
    
    setZoom();
    break;


  case Move:

    setMove();
    break;


  default:

    setCursor(arrowCursor);
    setMouseTracking(false);
    break;

  }

  emit modeChanged(_mode);

}



void
xQGanttBarViewPort::setSelect()
////////////////////////////////
{
  _mode = Select;
  setCursor(arrowCursor);
  setMouseTracking(true);
}



void
xQGanttBarViewPort::setZoom()
{
  _mode = Zoom;
  setCursor( *_cursor_lupe );
  setMouseTracking(false); 
}


void
xQGanttBarViewPort::setMove()
{
  _mode = Move;
  setCursor( sizeAllCursor );
  setMouseTracking(false);
}

#ifdef KKK
void 
xQGanttBarViewPort::snapCursor(int x, int y)
///////////////////////////////////////////
{
  static int wx,wy;
  static QString txt;

  int wwx = worldX(x);
  int wwy = worldY(y);

  QDateTime dt(_maintask->getStart().addSecs(wwx*60).date());

  printf("dt  = %s \n", dt.toString().latin1() );

  double tmp = ((double) wwx)/((double) _snapgrid);
  wwx = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;


  /*
    tmp = wwy/_grid;
    wwy = ((int) (tmp + sgn(tmp) * 0.5)) * _grid;   
  */

  /*
    QPainter p(this);
    
    p.setPen(red);
    
    repaint(screenX(wx) - 10, screenY(wy) - 10, 20, 20 );
    
    wx = wwx;  wy = wwy;
    
    p.drawLine( screenX( wwx ) - 5, screenY( wwy ), screenX( wwx ) + 5, screenY( wwy ) );
    p.drawLine( screenX( wwx ), screenY( wwy ) - 5, screenX( wwx ), screenY( wwy ) + 5);
  */
  wx = wwx;  wy = wwy;

}
#endif


void 
xQGanttBarViewPort::popup(int index)
///////////////////////////////////
{

  switch(index) {

  case Select:
  case Zoom:
  case Move:

    setMode(index);
    break;

  case 10: // configure
      
    // setConfigDialog();
    // _config->show();

    break;

  }


}



void
xQGanttBarViewPort::zoom(double sfactor, int wx, int wy)
///////////////////////////////////////////////////////
{
  _scaleX *= sfactor;

  printf("xQGanttBarViewPort::zoom(); scaleX = %f \n", _scaleX );

  //  _scaleY *= sfactor;
  /*
  _offsetX = (int) (wx - (width()/2 / _scale));
  _offsetY = (int) (wy - (height()/2 / _scale));
  */

  // printf("adjustSize.\n");
 
  recalc();
  adjustSize();

  QWidget::update();

  // printf("emit zoomed.\n");
  // emit zoomed();

}



void 
xQGanttBarViewPort::zoomAll()
/////////////////////////////
{  
  printf("zoom all. scaleX = %f\n", _scaleX );

  _scaleX = ((double) (((QWidget*) parent())->width()*60))/
    ((double) _maintask->getStart().secsTo(_maintask->getEnd()));
  

  recalc();
  adjustSize();

  // printf("emit zoomed.\n");
  // emit zoomed();

}



void 
xQGanttBarViewPort::addHoliday(int y, int m, int d) 
/////////////////////////////////////////////////////
{
  QDate* date = new QDate(y,m,d);
  
  QDate* ptrDate; 
  int i=0;
  
  for(ptrDate = _holidays.first(); 
      ptrDate != 0; 
      ptrDate = _holidays.next() ) {
    
    if(*ptrDate > *date)	
      break;      
    
    i++;
    
  }
  
  _holidays.insert(i,date);
  /*
    i=0;
    for(ptrDate = _holidays.first(); 
    ptrDate != 0; 
    ptrDate = _holidays.next(), i++ )
    printf("[%d] %s \n", i, ptrDate->toString().latin1() );
  */

}



xQGanttBarViewPort::Position 
xQGanttBarViewPort::check(xQTask** foundtask, int x, int y)
/////////////////////////////////////////////////////////////
{
  // printf("check( %d / %d ) \n", x, y );
  
  QPtrDictIterator<xQTaskPosition> it(_gTaskList); 

  static int ty, ty2, tx, tx2, hx, hx2, hy, hy2;

  while ( it.current() ) {

    ty  = it.current()->_screenY;
    ty2 = ty + it.current()->_screenH;
    tx  = it.current()->_screenX;
    tx2 = tx + it.current()->_screenW;
   

    hx  = it.current()->_screenHandleX;
    hx2 = hx + it.current()->_screenHandleW;
    hy  = it.current()->_screenHandleY;
    hy2 = hy + it.current()->_screenHandleH;

    if(x>tx && x < tx2) {

      if(y > ty && y < ty2) {

	*foundtask = (xQTask*) it.currentKey();

	if(x > hx && x < hx2 &&
	   y > hy && y < hy2 )
	  return Handle;

	 if(x < (tx + 5)) 
	   return West;

	 if(x > (tx2 - 5))
	   return East;

	 return Center;
      }
            
    }

    ++it;

  }

  return Outside;

}


void
xQGanttBarViewPort::unselectAll()
{
  selectTask(_maintask, false);
  QWidget::update();
}



void
xQGanttBarViewPort::selectAll()
{
  selectTask(_maintask, true);
  QWidget::update();
}



void
xQGanttBarViewPort::selectTask(xQTask* task, bool f)
{
  task->select(f);

  for(xQTask* subtask = task->getSubTasks().first(); 
      subtask != 0; 
      subtask = task->getSubTasks().next() ) {
      
    selectTask(subtask, f);

  }

}
#include "xQGanttBarViewPort.moc"
