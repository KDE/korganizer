#ifndef _XQGANTT_H_
#define _XQGANTT_H_
 
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

    file    : xQGantt.h
    date    : 26 oct 2000


    changelog : 23 nov 2000, jh

                24 nov 2000, jh

*/


#include <qwidget.h>
#include <qsplitter.h> 
#include <qtoolbar.h> 
#include <qpopupmenu.h>


#include "xQTask.h"
#include "xQGanttListView.h"
#include "xQGanttBarView.h"



/**
 *  \mainpage Gantt Module <br> <IMG SRC="gantt.png">
 *
 *  The gantt module contains several classes (xQTask, xQGantt)
 *  for drawing gantt-diagramms.
 *
 *  This example shows how to use the gantt module:
 *  \code
 *  #include "xQGantt.h"
 *  
 *  int main(int args, char* argv[])
 *  {
 *     xQTask* main = new xQTask(0, "project with a lot of subtasks", 
 *                               QDateTime::currentDateTime(),
 *                               QDateTime::currentDateTime());
 *
 *     main->setStyle(xQTask::DrawBorder | xQTask::DrawSubtasks | xQTask::DrawText |
 *                    xQTask::DrawHandle );
 *
 *     xQGantt* gantt = new xQGantt(main);
 *
 *     xQTask* t1 = new xQTask(gantt->getMainTask(), "task 1, no subtasks", 
 *                             QDateTime::currentDateTime().addDays(10),
 *                             QDateTime::currentDateTime().addDays(20) );
 *
 *     xQTask* t2 = new xQTask(gantt->getMainTask(), "task 2, subtasks, no rubberband", 
 *                             QDateTime(QDate(2000,10,1)),
 *                             QDateTime(QDate(2000,10,31)) );
 *
 *     gantt->setCaption( "- gantt -" );
 *     gantt->show();
 *
 *  }
 *  \endcode
 *
 *  You just have to create an object of class xQGantt and add several objects of
 *  class xQTask.
 *
 */



///  Gantt Widget.
/*!
 *   A gantt widget contains two parts, a list view and a
 *   bar view.
 */
////////////////////////////////
class xQGantt : public QWidget
////////////////////////////////
{

  Q_OBJECT


public:  


  ///  Constructor.
  /*!
   *
   */
  xQGantt(xQTask* maintask = 0, QWidget* parent = 0,  
	  const char * name=0, WFlags f=0 );


  ///  Destructor.
  /*!
   *
   */
  ~xQGantt();



  ///  Set main task.
  /*!
   *   If no main task was specified at construction of this widget a
   *   main task was created. This will be deleted by setting a new
   *   main task. A main task that was passed to the constructor will
   *   not be deleted.
   */
  void setMainTask(xQTask* task) {

    if(_deleteTask)
      delete _task;

    _task = task; 

  }



  ///  Get main task.
  /*!
   *
   */
  xQTask* getMainTask() { 
    return _task; 
  }



  ///  Get bar view of tasks.
  /*!
   *
   */
  xQGanttBarView* barView() {
    return _ganttbar;
  }



  ///  Get list view of tasks.
  /*!
   *
   */
  xQGanttListView* listView() {
    return _ganttlist;
  }



  ///  Get menu item.
  /*!
   *
   */
  QPopupMenu* menu() {
    return _ganttbar->viewport()->menu();
  }


  ///  Add gantt toolbar to mainwindow.
  /*!
   *   If you want to embed a toolbar with specific actions
   *   like zooming or coniguring the gantt, you can add a toolbar
   *   automatically by invoking this method. You have to pass your
   *   mainwindow as a parameter.
   */
  QToolBar* toolbar(QMainWindow* mw) {
    return _ganttbar->viewport()->toolbar(mw);
  }



  ///  Print to stdout.
  /*
   *
   */
  void dumpTasks();



  void addHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



  void removeHoliday(int y, int m, int d) {
    _ganttbar->viewport()->addHoliday(y,m,d);
  }



protected:


  void resizeEvent(QResizeEvent*) {
    _splitter->resize(width(),height());
  };


private:

  bool _deleteTask;
   
  //  maintask
  xQTask* _task;

  QSplitter *_splitter;

  xQGanttBarView* _ganttbar;
  xQGanttListView* _ganttlist;


};


#endif
