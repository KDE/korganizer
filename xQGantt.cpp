//
//  file    : xQGantt.C
//  date    : 26 oct 2000
//  changed : 24 nov 2000
//  author  : jh
//


#include "xQGantt.h"

#include <qcolor.h>
#include <qwindowsstyle.h> 
#include <qcdestyle.h>
#include <qscrollview.h> 


xQGantt::xQGantt(xQTask* maintask = 0, QWidget* parent = 0, 
		 const char * name=0, WFlags f=0 )
  : QWidget(parent,name,f)
/////////////////////////////////////////////////////////
{ 
  printf("xQGantt::xQGantt()\n");

  if(maintask == 0) {
    _task = new xQTask(0, "maintask",
		       QDateTime::currentDateTime(),
		       QDateTime::currentDateTime() );
    _task->setMode(xQTask::Rubberband);
    _deleteTask = true;
  }
  else {
    _task = maintask;
    _deleteTask = false;
  }

  setBackgroundColor(QColor(white));

  _splitter = new QSplitter(this);
  _splitter->setStyle(new QCDEStyle());

  QPalette pal1(_splitter->palette());
  QPalette pal(_splitter->palette());
  QColorGroup cg(pal.active());
  cg.setColor( QColorGroup::Foreground, blue );
  cg.setColor( QColorGroup::Background, white );
  pal.setActive( cg );

  _splitter->setPalette(pal);
  
  _ganttlist = new xQGanttListView(_task, _splitter); 
  _ganttlist->setMinimumWidth(1);
  _ganttlist->setPalette(pal1);

  _ganttbar = new xQGanttBarView(_task, _splitter);
  _ganttbar->setPalette(pal1);

  connect(_ganttbar, SIGNAL(contentsMoving(int,int)),
	  _ganttlist, SLOT(contentsMoved(int,int)));

  _ganttlist->setBarView(_ganttbar);

}



xQGantt::~xQGantt()
///////////////////
{
  if(_deleteTask)
    delete _task;
}




void 
xQGantt::dumpTasks()
/////////////////////////
{
  QTextOStream cout(stdout);

  cout << "\n<Gantt>\n";
  cout << " start : " << _task->getStart().toString() << endl;
  cout << " end :   " << _task->getEnd().toString() << endl;

  _task->dump(cout, "  ");

  cout << "</Gantt>\n\n";

}



