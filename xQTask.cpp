//
//  file    : xQTask.C
//  date    : 26 oct 2000
//  changed : 26 dec 2000
//  author  : jh
//


#include "xQTask.h"


QBrush xQTask::_selectBrush(QColor(255,0,0));


xQTask::xQTask(xQTask* parentTask, const QString& text, 
	 const QDateTime& start, const QDateTime& end)
  : QObject()
////////////////////////////////////////////////////////
{
  init(parentTask,text, start,end);
}



xQTask::xQTask(xQTask* parentTask, const QString& text, 
	 const QDateTime& start, long durationMin)
  : QObject()
////////////////////////////////////////////////////////
{  
  init(parentTask, text, start, start.addSecs( durationMin * 60));
}



void 
xQTask::init(xQTask* parentTask, const QString& text,
	     const QDateTime& start, const QDateTime& end) 
///////////////////////////////////////////////////////////////
{

  _style = DrawAll - DrawHandle;
  _open = true;
  _selected = false;

  _mode = Normal;

  _brush = QBrush(QColor(140,140,255));
  _pen = QPen(QColor(100,100,100));
  _textPen = QPen(QColor(black));

  _height = 20;

  _text = text;

  _start = start; _minDateTime = start;
  _end = end; _maxDateTime = end;

  _parentTask = parentTask;

  if(_parentTask)
    _parentTask->registerTask(this);
  
}



xQTask::~xQTask()
/////////////////
{
  if(_parentTask)
    _parentTask->unregisterTask(this);

  _subtasks.setAutoDelete(true);
  _subtasks.clear();

}



void 
xQTask::endTransaction()
///////////////////////////
{
  blockSignals(false);
  emit changed(this, (Change) 2047);
}



void 
xQTask::registerTask(xQTask* task)
////////////////////////////////////////
{
  // printf("[%s] register task : %s \n", getText().latin1(), task->getText().latin1() );

  _subtasks.append(task);
  
  connect(task, SIGNAL(changed(xQTask*, xQTask::Change)),
	  this, SLOT(subTaskChanged(xQTask*, xQTask::Change)) );


  bool minChanged = false;
  bool maxChanged = false;


  // update min/man

  if(_subtasks.count() == 1) {

    _minDateTime = task->getStart();
    _maxDateTime = task->getEnd();
    
    minChanged = true;
    maxChanged = true;

  }
  else {

    if(task->getEnd() > _maxDateTime) {
      _maxDateTime = task->getEnd();
      maxChanged = true;
    }
    
    if(_minDateTime > task->getStart()) {
      _minDateTime = task->getStart();
      minChanged = true;
    }
    
  } // else



  //  increase start/end if necessary
  int change = adjustStartEnd();


  if(_mode == Rubberband) {
    if(minChanged && !(change & StartChanged))
      change += StartChanged;
    if(maxChanged && !(change & EndChanged))
      change += EndChanged;
  }

  if( isOpen() ) {
    if(!(change & TotalHeightChanged))
      change += TotalHeightChanged;
  }

  if(change != NoChange)
    emit changed(this,(Change) change);

}



void 
xQTask::unregisterTask(xQTask* task)
///////////////////////////////////////////
{
  _subtasks.remove(task);
  disconnect(task);

  int change = adjustMinMax();

  if( isOpen() ) {
    if(!(change & TotalHeightChanged))
      change += TotalHeightChanged;
  }

  if(change != NoChange)
    emit changed(this,(Change) change);

}


QDateTime 
xQTask::getStart()
///////////////////
{ 
  if(_mode == Rubberband && _subtasks.count()>0)
    return _minDateTime;
  else
    return _start; 
}




QDateTime 
xQTask::getEnd()
/////////////////
{
  if(_mode == Rubberband && _subtasks.count()>0)
    return _maxDateTime;
  else
    return _end; 
}



void 
xQTask::setStart(const QDateTime& start) 
/////////////////////////////////////////
{
  //  if there are no subtasks, just set _start and _minDateTime
  if(_subtasks.count()==0) {

    if(_start != start) {
      _start = start;
      _minDateTime = _start;
      emit changed(this,StartChanged);
    }
    
  }
  else {

    //  if there are subtasks, just change start if
    //  mode is not 'rubberband' and start is less than _minDateTime

    if(_mode != Rubberband) {

      if(start < _minDateTime)
	_start = start;
      else
	_start = _minDateTime;
    
      emit changed(this,StartChanged);

    }
      
  }
  
}



void 
xQTask::setEnd(const QDateTime& end) 
//////////////////////////////////////
{ 

  //  if there are no subtasks, just set _end and _maxDateTime
  if(_subtasks.count()==0) {

    if(_end != end) {
      _end = end;
      _maxDateTime = _end;
      emit changed(this,EndChanged);
    }

  }
  else {

    //  if there are subtasks, just change end if
    //  mode is not 'rubberband' and end is greater than _maxDateTime

    if(_mode != Rubberband) {

      if(end > _maxDateTime)
	_end = end;
      else
	_end = _maxDateTime;

      emit changed(this,EndChanged);

    }
    
  }
  
}




xQTask::Change
xQTask::adjustStartEnd()
//////////////////////////
{
  //  first update _min and _max of subtasks

  int c = adjustMinMax();

  if(_start > _minDateTime) {
    _start = _minDateTime;
    if(!(c & StartChanged))
      c += StartChanged;
  }
  
  if(_end < _maxDateTime) {
    _end = _maxDateTime;
    if(!(c & EndChanged))
      c += EndChanged;
  }  
  
  return (Change)c;
  
}



xQTask::Change 
xQTask::adjustMinMax()
//////////////////////////
{
  //
  //  calculate _min and _max by 
  //  traversing the subtasks. if there are no subtasks
  //  _min = start and _max = end.
  //

  QDateTime min = _minDateTime;
  QDateTime max = _maxDateTime;
  int c = NoChange;

  if(_subtasks.count()==0) {

    _minDateTime = _start;
    _maxDateTime = _end;

    if(min != _minDateTime) c  = MinChanged;
    if(max != _maxDateTime) c += MaxChanged;

  }
  else {
    
    // get min/max date and time
    
    xQTask* task = _subtasks.first();
    
    _minDateTime = task->getStart();
    _maxDateTime = task->getEnd();
    
    task = _subtasks.next();
    
    for(; task != 0; task = _subtasks.next() ) {
      
      if(_minDateTime > task->getStart()) {
	_minDateTime = task->getStart();
      }
      
      if(task->getEnd() > _maxDateTime) {
	_maxDateTime = task->getEnd();
      }
      
    } // for()
    
    
    if(min != _minDateTime) c  = MinChanged;
    if(max != _maxDateTime) c += MaxChanged;
  
  }

  return (Change) c;

}



void 
xQTask::subTaskChanged(xQTask* task, Change change)
/////////////////////////////////////////////////////
{  
  if(change & StyleChanged)
    emit changed(this, change);
  
  if( (change & Opened) || (change & Closed) || 
      (change & TotalHeightChanged) || (change & HeightChanged) )
    emit changed(this, TotalHeightChanged);

  if( (change & StartChanged) || 
      (change & EndChanged) ) {

    int c = adjustStartEnd();
    
    if(_mode == Rubberband) {
      if(c & MinChanged && !(c & StartChanged)) 
	c += StartChanged;
      if(c & MaxChanged && !(c & EndChanged))
	c += EndChanged;
    }

    if(c != NoChange)
      emit changed(this, (Change) c);
   
  }
}



void 
xQTask::setText(const QString& text) 
///////////////////////////////////////
{ 
  if(text != _text) {
    _text = text; 
    emit changed(this,TextChanged);
  }
}



void 
xQTask::open(bool f)
//////////////////////
{
  if(f != _open) {
    _open = f;
    if(_open)
      emit changed(this, Opened);
    else
      emit changed(this, Closed);
  }
}



void
xQTask::select(bool f)
///////////////////////
{
  if(f != _selected) {
    _selected = f;
    if(_selected)
      emit changed(this, Selected);
    else
      emit changed(this, Unselected);
  }
}



void 
xQTask::setMode(Mode flag) 
////////////////////////////
{
  if(_mode != flag) {
    _mode = flag;
    emit changed(this,ModeChanged);
  }
  
}



void
xQTask::setStyle(int flag, bool includeSubTasks) 
///////////////////////////////////////////////
{
  if(_style != flag) {

    _style = flag;
 
    if(includeSubTasks)
      for(xQTask* task = _subtasks.first(); 
	  task != 0; 
	  task = _subtasks.next() )
	task->setStyle(flag,true);

    emit changed(this,StyleChanged);

  }

}



void
xQTask::setBrush(const QBrush& brush)
///////////////////////////////////////
{
  _brush = brush;
}



void
xQTask::setPen(const QPen& pen)
///////////////////////////////
{
  _pen = pen;
}



void
xQTask::setHeight(int h)
/////////////////////////
{
  if(_height != h) {
    _height = h;
    emit changed(this,HeightChanged);
  }
}



int 
xQTask::getTotalHeight()
////////////////////////////////////////
{
  int h = _height;

  if( isOpen() ) {
    for(xQTask* task = _subtasks.first(); task != 0; task = _subtasks.next() ) {
      h += task->getTotalHeight();
    }
  }
  return h;
}



int
xQTask::getWidth()
//////////////////
{
  //  int width = _start.secsTo(_end)/60;

  int width = getStart().secsTo(getEnd())/60;
 
  // printf("width[%s] = %d \n", (const char*) getID(), width );

  return width;
}



void 
xQTask::dump(QTextOStream& cout, const QString& pre) 
////////////////////////////////////////////////////
{
  cout << pre << "<Task. text = [" << _text << "]>\n";
  cout << pre << "|  start : " << getStart().toString() << "  (" <<_start.toString() << ")" << endl;
  cout << pre << "|  end :   " << getEnd().toString() << "  (" <<_end.toString() << ")" << endl;
  
  if(_mode == Rubberband)
    cout << pre << "|  mode = 'rubberband'" << endl;
  else
    cout << pre << "|  mode = 'normal'" << endl;

  cout << pre << "|  min date/time : " << _minDateTime.toString() << endl;
  cout << pre << "|  max date/time : " << _maxDateTime.toString() << endl;
  
  for(xQTask* task = _subtasks.first(); task != 0; task = _subtasks.next() )
    task->dump(cout, pre + "|   ");
  
  cout << pre << "</Task>\n";

}


QString
xQTask::ChangeAsString(Change c)
//////////////////////////////////
{
  QString ret;

  if(c & StartChanged)       ret += "StartChanged, ";
  if(c & EndChanged)         ret += "EndChanged,  ";
  if(c & HeightChanged)      ret += "HeightChanged,  ";
  if(c & TotalHeightChanged) ret += "TotalHeightChanged,  ";
  if(c & StyleChanged)  ret += "StyleChanged,  ";
  if(c & TextChanged)   ret += "TextChanged,  ";
  if(c & ModeChanged)   ret += "ModeChanged,  ";
  if(c & MinChanged)    ret += "MinChanged,  ";
  if(c & MaxChanged)    ret += "MaxChanged,  ";
  if(c & Opened)        ret += "Opened,  ";
  if(c & Closed)        ret += "Closed,  ";
  if(c & Selected)      ret += "Selected, ";
  if(c & Unselected)      ret += "Unselected, ";
  return ret;

}
#include "xQTask.moc"
