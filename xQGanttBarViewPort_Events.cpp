//
//  file    : xQGanttBarViewPort_Events.C
//  date    : 11 nov 2000
//  changed : 27 dec 2000
//  author  : jh
//

#include "math.h"

#include "xQGanttBarViewPort.h"


int _currentMButton = 0;
bool _Mousemoved = FALSE;
bool _selectTask = false;
xQTask* _currentTask = NULL;
int _timediff;

bool _changeEnd = false, _changeStart = false;
int oldw = -1, oldx = -1;

QDateTime _tmpStartDateTime, _tmpEndDateTime;


void 
xQGanttBarViewPort::mousePressEvent(QMouseEvent* e)
//////////////////////////////////////////
{
  //  set _currentTask to pushed mousebutton
  _currentMButton = e->button();

  _Mousemoved = false;

  //  right mousebutton & control -> popup menu
  if(e->button() == RightButton && e->state() == ControlButton ) {
    _menu->popup(e->globalPos());
    return;
  }


  if(e->button() == LeftButton && _mode == Select) {

    _currentTask = NULL;
    _timediff = 0;

    Position pos = check(&_currentTask, e->x(), e->y());

    if(_currentTask) {

      if(pos == Handle)
	_currentTask->open( !_currentTask->isOpen() );
      
      if(pos == Center) {
	
	_changeEnd   = true;
	_changeStart = true;
	
	if(e->state() == ShiftButton) {
	  QString tmp; tmp.sprintf("%s\n", _currentTask->getText().latin1() );
	  
	  tmp += _currentTask->getStart().toString();
	  tmp += " - ";
	  tmp += _currentTask->getEnd().toString();
	  
	  _taskInfo->setText( tmp );
	  _taskInfo->adjustSize();
	  
	  _taskInfo->move(e->x() + 5, _gTaskList.find(_currentTask)->_screenY - 50 );
	  _taskInfo->show();
	}
	else
	  _selectTask = true;
	
      } //  if(pos == xQTask::Center)
      
      if(pos == East) {
	_changeEnd = true;
	_changeStart = false;
      }
      
      if(pos == West) {
	_changeStart = true;
	_changeEnd = false;
      }
    
    } //  if(_currentTask)
    else {
      _taskInfo->hide();
      unselectAll();
    }
    
  }
  
  _startPoint->setX( e->x() );
  _startPoint->setY( e->y() );

  _endPoint->setX( e->x() );
  _endPoint->setY( e->y() );

}



void
xQGanttBarViewPort::mouseReleaseEvent(QMouseEvent* e)
////////////////////////////////////////////
{
  int wx = worldX(e->x());
  int wy = worldY(e->y());

  switch(_mode) {

  case Select: {

    if(_Mousemoved == true) {
      
      _taskInfo->hide();

      if(_changeStart == true || _changeEnd == true) {
      
	if(_changeStart == true) {
	  _currentTask->setStart( _tmpStartDateTime );
	}

	if(_changeEnd == true) {
	  _currentTask->setEnd( _tmpEndDateTime );
	}

	_changeEnd   = false;
	_changeStart = false;
	
	oldx = -1; oldw = -1;
	
	recalc();
	QWidget::update();
	
      }
    
    }
    else {
      if(_currentTask && _selectTask) {


	if(e->state() & ControlButton) {
	  _currentTask->select( !_currentTask->isSelected() );
	}
	else {
	  bool state = _currentTask->isSelected();
	  unselectAll();
	  _currentTask->select( !state );
	}

	QWidget::update();
	_selectTask = false;

      }
    }

  }
  break;
  

  case Zoom:
    
    if(!_Mousemoved) {
      
      if(e->button() ==  LeftButton)
	zoom(1.4, wx, wy );
      
      
      if(e->button() ==  RightButton)
	zoom(0.7,wx,wy);
      

      if(e->button() ==  MidButton)
	zoomAll();

    }
    else {

      if(_currentMButton ==  LeftButton) {

	QPainter p(this);
	QPen pen(DashLine);
	pen.setColor(red);
	p.setRasterOp(XorROP);      
	p.setPen( pen );

	p.drawRect(_startPoint->x(),
		   _startPoint->y(), 
		   _endPoint->x()-_startPoint->x(),
		   _endPoint->y() - _startPoint->y());

	double x1 = _startPoint->x();
	double y1 = _startPoint->y();

	double x2 = _endPoint->x();
	double y2 = _endPoint->y();

	double sys_height = fabs(y2 - y1);
	double sys_width  = fabs(x2 - x1);

	int rand = 10;
	
	double mass = ((width() - 2 * rand) / sys_width) <
	  ((height() - 2 * rand) / sys_height) ?
	  ((width() - 2 * rand)/ sys_width) :
	  ((height() - 2 * rand) / sys_height);
	
	zoom(mass, worldX(x1 + sys_width/2), worldY(y1 + sys_height/2));


      }
    }

    break;


  default:
    break;

  }

  _Mousemoved = false;
  _currentMButton = 0;

}



void
xQGanttBarViewPort::mouseMoveEvent(QMouseEvent* e)
///////////////////////////////////////////////////
{
  if(fabs(_startPoint->x() - e->x()) < 2 &&
     fabs(_startPoint->y() - e->y()) < 2 )
    return;

  _Mousemoved = true;

  switch(_mode) {
    
  case Select: {
    
    if(_currentMButton == LeftButton && _currentTask) {

      QPainter p(this);
      p.setRasterOp(XorROP);
      QPen pen(DashLine);
      pen.setColor(red);
      p.setPen( pen );

      QString stmp;
      stmp.sprintf("%s\n", _currentTask->getText().latin1() );

      int pixeldiff = e->x() - _startPoint->x();
      _timediff = (int) ((double) pixeldiff / _scaleX + 0.5 );
      
      xQTaskPosition* tpos = _gTaskList[_currentTask];

      int x = tpos->_screenX; int w = tpos->_screenW;
      
      if(_changeStart && _changeEnd) {
	double tmp = (double) _timediff/(double) _snapgrid;
	_timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;
	stmp += _currentTask->getStart().addSecs(_timediff*60).toString();
	stmp += " - ";
	stmp += _currentTask->getEnd().addSecs(_timediff*60).toString();	
	x += (int) (_timediff * _scaleX);

	_tmpStartDateTime = _currentTask->getStart().addSecs(_timediff*60);
	_tmpEndDateTime = _currentTask->getEnd().addSecs(_timediff*60);

      }
      else {

	if(_changeStart) {

	  QDateTime movedStart( _currentTask->getStart().addSecs(_timediff*60) );

	  _tmpStartDateTime.setDate( movedStart.date() );
	  _tmpEndDateTime.setTime(QTime(0,0,0,0));

	  double diff = _tmpStartDateTime.secsTo(movedStart)/60;

	  double tmp = diff/(double) _snapgrid;
	  _timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;
	  
	  _tmpStartDateTime = _tmpStartDateTime.addSecs(_timediff*60);
	  _timediff = _currentTask->getStart().secsTo(_tmpStartDateTime)/60;

	  stmp += _tmpStartDateTime.toString().latin1();
	  stmp += " - ";
	  stmp += _currentTask->getEnd().toString();

	  x += (int) (_timediff * _scaleX);
	  w -= (int) (_timediff * _scaleX);
	}
	
	if(_changeEnd) {

	  QDateTime movedEnd( _currentTask->getEnd().addSecs(_timediff*60) );

	  _tmpEndDateTime.setDate( movedEnd.date() );
	  _tmpEndDateTime.setTime(QTime(0,0,0,0));

	  double diff = _tmpEndDateTime.secsTo(movedEnd)/60;

	  double tmp = diff/(double) _snapgrid;
	  _timediff = ((int) (tmp + sgn(tmp) * 0.5)) * _snapgrid;

	  _tmpEndDateTime = _tmpEndDateTime.addSecs(_timediff*60);
	  _timediff = _currentTask->getEnd().secsTo(_tmpEndDateTime)/60;

	  stmp += _currentTask->getStart().toString();
	  stmp += " - ";
	  stmp += _tmpEndDateTime.toString().latin1();

	  w += (int) (_timediff * _scaleX);

	}

      }

      _taskInfo->setText( stmp );
      _taskInfo->adjustSize();
      _taskInfo->move(e->x() + 5, _gTaskList.find(_currentTask)->_screenY - 50);
      _taskInfo->show();

      if(oldx > 0) {
	p.fillRect(oldx, _gTaskList.find(_currentTask)->_screenY, 
		   oldw, _gTaskList.find(_currentTask)->_screenH,
		   QBrush(QColor(50,50,50), Dense4Pattern));
	p.drawRect(oldx, _gTaskList.find(_currentTask)->_screenY, 
		   oldw, _gTaskList.find(_currentTask)->_screenH);
      }

      p.fillRect(x, _gTaskList.find(_currentTask)->_screenY, 
		 w, _gTaskList.find(_currentTask)->_screenH,
		 QBrush(QColor(50,50,50), Dense4Pattern) );
      p.drawRect(x, _gTaskList.find(_currentTask)->_screenY, 
		 w, _gTaskList.find(_currentTask)->_screenH);

      oldx = x; oldw = w;

    }
    else {
      
      static Position _pos = Outside;
      
      xQTask* task = NULL;
      
      Position pos = check(&task, e->x(), e->y());
      
      if(_pos != pos) {
	
	_pos = pos;
	
	if(pos == West || pos == East) {
	  setCursor( splitHCursor );
	  break;
	}
	if(pos == North || pos == South) {
	  setCursor( splitVCursor );
	  break;
	}
	if(pos == Center) {
	  setCursor( upArrowCursor);
	  break;
	}
	if(pos == Handle) {
	  setCursor(pointingHandCursor);
	  break;
	}
	
	setCursor(arrowCursor);
	
      }
    }
  }
  break;


  case Zoom: {

    if(_currentMButton == LeftButton) {

      static QString strpos;

      strpos = "";

      int s = worldX(_startPoint->x());
      QDateTime d1 = _maintask->getStart().addSecs(s*60);

      s = worldX(e->x());
      QDateTime d2 = _maintask->getStart().addSecs(s*60);

      strpos += d1.date().toString();
      strpos += " - ";
      strpos += d2.date().toString();

      emit message(strpos);

      QPainter p(this);
      QPen pen(DashLine);
      pen.setColor(red);

      p.setRasterOp(XorROP);

      p.setPen( pen );
            
      p.drawRect(_startPoint->x(),
		 _startPoint->y(),
		 _endPoint->x()-_startPoint->x(),
		 _endPoint->y() - _startPoint->y());    

      QBrush _selectedbrush( QColor(50,50,50), Dense4Pattern );

      p.fillRect( _startPoint->x(), _startPoint->y(), 
		  _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y(),
		  _selectedbrush );

      _endPoint->setX( e->x() );
      _endPoint->setY( e->y() );


      p.drawRect(_startPoint->x(), _startPoint->y(), 
		 _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y());

      p.fillRect( _startPoint->x(), _startPoint->y(),
		  _endPoint->x()-_startPoint->x(), _endPoint->y() - _startPoint->y(),
		  _selectedbrush );
    }

  }

  break;

  case Move: {
    emit scroll(_startPoint->x() - e->x(), _startPoint->y() - e->y() );
  }
  break;


  default :
    break;

  }
}


void 
xQGanttBarViewPort::keyPressEvent(QKeyEvent* e)
//////////////////////////////////////////////// 
{
  int dx = 15;
  
  if(e->state() == ControlButton)
    dx *= 10;
  
  switch(e->key()) {
    
  case Key_Left:
    
    emit scroll(-dx,0);
    break;
    
  case Key_Right:
    
    emit scroll(dx,0);
    break;
    
  case Key_Up:
    
    emit scroll(0,-dx);
    break;
    
  case Key_Down:
    
    emit scroll(0, dx);
    break;

  case 43:  // +

    zoom(1.4,0,0);
    break;

  case 45: // -

    zoom(0.7,0,0);
    break;

  }
  
} 


void 
xQGanttBarViewPort::paintEvent(QPaintEvent * e) 
/////////////////////////////////////////////////
{
  // printf("xQGanttBarViewPort::paintEvent()\n");
  update(e->rect().left(), e->rect().top(),
	 e->rect().right(), e->rect().bottom() );
}
