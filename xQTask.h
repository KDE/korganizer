#ifndef _XQTASK_H_
#define _XQTASK_H_
 
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

    file    : xQTask.h
    date    : 26 oct 2000


    changelog : 26 dec 2000, jh

*/


#include <qobject.h>
#include <qdatetime.h> 
#include <qtextstream.h> 
#include <qlist.h> 
#include <qpainter.h>




///  Task.
/*!
 *   This class describes a task. It contains dates on which the task starts and
 *   ends. It also contains attributes that gouverns the graphical representation
 *   in a gantt diagramm.
 */
///////////////////////////////
class xQTask : public QObject
///////////////////////////////
{

  Q_OBJECT


public:
  
  enum Change { NoChange        = 0,
		StartChanged    = 1,
		EndChanged      = 2,
	
		///  Height for this task has changed. The
		///  height doesn't include the subtasks.
		HeightChanged   = 4,
	
		///  Total height has changed. This 
		///  happens if task was opened, closed
		///  or subtasks has been added or removed while
		///  task is open.
		TotalHeightChanged = 8,

		///  Style for drawing has changed.
                StyleChanged    = 16,
                TextChanged     = 32,
		ModeChanged     = 64,
		MinChanged      = 128,
		MaxChanged      = 256,
	
		/// Draw task including subtasks.
		Opened          = 512,
	
		/// Draw task without subtasks.
		Closed          = 1024,
		Selected        = 2048,
		Unselected      = 4096 };



  enum Style { /// Draw border.
               DrawBorder      = 1, 

	       //  Fill task with brush.
	       DrawFilled      = 2, 
	       DrawText        = 4,

	       //  Draw handlke for opening/closing task.
	       DrawHandle      = 16,
	    
	       /// Draw handle only if task contains subtasks
	       DrawHandleWSubtasks = 32,

               DrawAll         = 255 };


  enum Mode { Normal, 
	      Rubberband };


  ///  Constructor.
  /*!
   * 
   */
  xQTask(xQTask* parentTask, const QString& text, 
	 const QDateTime& start, const QDateTime& end);



  ///  Constructor.
  /*!
   * 
   */
  xQTask(xQTask* parentTask, const QString& text, 
	 const QDateTime& start, long durationMin);



  ///   Destructor.
  /*
   *
   */
  ~xQTask();

  

  ///  Return true if task is open (subtasks has to be drawn)
  /*!
   *
   */
  bool isOpen() {
    return _open;
  }



  ///  Open / Close task
  /*!
   *   Draw/don't draw subtasks.
   */
  void open(bool f);



  ///
  /*!
   *
   */
  bool isSelected() {
    return _selected;
  }



  ///
  /*!
   *
   */
  void select(bool f);



  ///  Set mode.
  /*!
   *   If mode is 'Rubberband' and the number of subtaks is greater than 0,
   *   the start and end of the task is determined by the start and end of the
   *   earliest/latest subtask. <br>
   *   Default is 'Normal'.
   */
  void setMode(Mode flag);



  ///  Set drawing style.
  /*!  
   *   Set drawing style.
   */
  void setStyle(int flag, bool includeSubTasks = false);



  ///  Get drawing style.
  /*!
   *
   */
  int getStyle() {
    return _style;
  }


  ///  Set brush for filling
  /*!
   *
   */
  void setBrush(const QBrush& brush);



  ///  Get brush that is used for filling the task.
  /*!
   *
   */
  QBrush& getBrush() {
    return _brush;
  }



  QBrush& getSelectBrush() {
    return _selectBrush;
  }



  ///  Set pen for border.
  /*!
   *
   */
  void setPen(const QPen& pen);



  ///
  /*!
   *
   */
  QPen& getPen() {
    return _pen;
  }



  ///
  /*!
   *
   */
  void setTextPen(const QPen& pen) {
    _textPen = pen;
  }



  ///
  /*!
   *
   */
  QPen& getTextPen() {
    return _textPen;
  }



  ///  Set text.
  /*!
   *
   */
  void setText(const QString& text);



  ///
  /*!
   *
   */
  QString getText() { return _text; }
  


  ///  Get date of starting.
  /*!
   *   If mode == ´Rubberband´ and this task contains
   *   subtasks, start of the task is determined by the start of the
   *   earliest subtask. <br>
   */
  QDateTime getStart();



  ///  Get date of ending.
  /*!
   *
   */
  QDateTime getEnd();



  ///  Set time/date of start.
  /*!
   *
   */
  void setStart(const QDateTime& start);



  ///  Set time/date of end.
  /*!
   *
   */
  void setEnd(const QDateTime& end);



  ///  Set height.
  /*!
   *   Set height in pixel. These are scaled when tasks are draw
   *   by the barview.
   */
  void setHeight(int h);



  ///  Get height.
  /*!
   *  Returns the height in pixel of this task. This does not include the height 
   *  of any subatasks; getTotalHeight() returns that if the subatasks have
   *  to drawn.
   */
  int getHeight() {
    return _height;
  }



  ///  Get total height.
  /*!
   *   Returns the total height of this object in pixel, including any 
   *   visible subtasks. Notice, that the pixels are no screen pixel since
   *   the barview scales the height of a task.
   */
  int getTotalHeight();

  

  ///  Get width in minutes.
  /*!
   *   
   */
  int getWidth();



  ///  Get list of subtasks.
  /*!
   *
   */
  QList<xQTask>& getSubTasks() {
    return _subtasks;
  }



  /// 
  /*!
   *   adding a lot of subtasks -> block signals
   */
  void startTransaction(){
    blockSignals(true);
  }



  ///
  /*!
   *
   */
  void endTransaction();


  ///  Return a given change as a string.
  /*!
   *
   */
  static QString ChangeAsString(Change c);




  ///  Dump to cout.
  /*!
   *
   */
  void dump(QTextOStream& cout, const QString& pre);


signals:

  ///  Task has changed.
  /*!
   *   This signal is emitted if any of the tasks
   *   properties have been changed.
   */
  void changed(xQTask*, xQTask::Change);
  

private slots:
 
  void subTaskChanged(xQTask*, xQTask::Change);


private:

  void registerTask(xQTask* task);
  void unregisterTask(xQTask* task);

  void init(xQTask* parentTask, const QString& text,
	    const QDateTime& start, const QDateTime& end);


  //  set min/max date and time according to subtasks
  Change adjustMinMax();

  /*  if min < start set start to _min,
      if max > end set end to max */      
  Change adjustStartEnd();


  // is task open/closed
  bool _open;
  bool _selected;

  int _height, _style, _mode;

  xQTask* _parentTask;
  QList<xQTask> _subtasks;  


  // start/end date. 
  // start must always be earlier then _minDateTime
  // end must always be later then _maxDateTime
  QDateTime _start, _end, _minDateTime, _maxDateTime;
  
  QString _text;

  QBrush _brush;
  QPen _pen, _textPen;

  static QBrush _selectBrush;
 

};

#endif
