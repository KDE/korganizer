// $Id$

#include <qpainter.h>
#include <qkeycode.h>
#include <qprinter.h>
#include <qscrollbar.h>
#include <qcheckbox.h>
#include <qpoint.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qdrawutil.h>
#include <qframe.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>

#include "optionsdlg.h"

#include "kotodoview.h"
#include "kotodoview.moc"

KOTodoViewItem::KOTodoViewItem(QListView *parent, KOEvent *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

KOTodoViewItem::KOTodoViewItem(KOTodoViewItem *parent, KOEvent *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

void KOTodoViewItem::construct()
{
  setOn(mEvent->getStatus() == KOEvent::NEEDS_ACTION ? false : true );
  setText(0, mEvent->getSummary());
  setText(1, QString::number(mEvent->getPriority()));
  if (mEvent->hasDueDate()) {
    setText(2, mEvent->getDtDue().date().toString());
    if (mEvent->doesFloat()) setText(3,"");
    else setText(3,mEvent->getDtDue().time().toString());
  } else {
    setText(2,"");
    setText(2,"");
  }
}

KOTodoView::KOTodoView(CalObject *calendar,QWidget* parent,const char* name) :
  QWidget(parent,name), mCalendar(calendar)
{
  QBoxLayout *topLayout = new QVBoxLayout(this);
  
  QLabel *title = new QLabel(i18n("To-Do Items"),this);
  topLayout->addWidget(title);
  
  mTodoListView = new QListView(this);
  topLayout->addWidget(mTodoListView);

  mTodoListView->setRootIsDecorated(true);
  mTodoListView->setAllColumnsShowFocus(true);
  
  mTodoListView->addColumn(i18n("Summary"));
  mTodoListView->addColumn(i18n("Priority"));
  mTodoListView->setColumnAlignment(1,AlignHCenter);
  mTodoListView->addColumn(i18n("Due Date"));
  mTodoListView->addColumn(i18n("Due Time"));
  
  mItemPopupMenu = new QPopupMenu;
  mItemPopupMenu->insertItem(BarIcon("todolist"), i18n("New To-Do"), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("New Sub-To-Do"), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertItem(i18n("Edit To-Do"), this,
                             SLOT (editTodo()));
  mItemPopupMenu->insertItem(BarIcon("delete"), i18n("Delete To-Do"), this,
                             SLOT (deleteTodo()));
  mItemPopupMenu->insertItem(i18n("Purge Completed"), this,
                             SLOT(purgeCompleted()));
                       
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(BarIcon("todolist"), i18n("New To-Do"), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
  
  // Double clicking conflicts with opening/closing the subtree                   
//  QObject::connect(mTodoListView,SIGNAL(doubleClicked(QListViewItem *)),
//                   this,SLOT(editItem(QListViewItem *)));
  QObject::connect(mTodoListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                   const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));

}

void KOTodoView::updateView()
{
//  qDebug("KOTodoView::updateView()");
  mTodoListView->clear();

  QList<KOEvent> todoList = mCalendar->getTodoList();

  // Put for each KOEvent a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  KOEvent *todo;
  for(todo = todoList.first(); todo; todo = todoList.next()) {
    if (!mTodoMap.contains(todo)) {
      insertTodoItem(todo);
    }
  }
}

QMap<KOEvent *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(KOEvent *todo)
{
  KOEvent *relatedTodo = todo->getRelatedTo();
  if (relatedTodo) {
    QMap<KOEvent *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
      itemIterator == insertTodoItem (relatedTodo);
    }
    KOTodoViewItem *todoItem = new KOTodoViewItem(*itemIterator,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  } else {
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  }
}


void KOTodoView::updateConfig()
{
  // to be implemented.
}

KOEvent *KOTodoView::getSelected()
{
  // to be implemented.
  return 0;
}

void KOTodoView::changeEventDisplay(KOEvent *, int)
{
  // play dumb and update all todos.
  updateView();
}

void KOTodoView::editItem(QListViewItem *item)
{
  emit editEventSignal(((KOTodoViewItem *)item)->event());
}

void KOTodoView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOTodoViewItem *)item;
  if (item) mItemPopupMenu->popup(QCursor::pos());
  else mPopupMenu->popup(QCursor::pos());
}

void KOTodoView::newTodo()
{
  emit newTodoSignal();
}

void KOTodoView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->event());
  }
}

void KOTodoView::editTodo()
{
  if (mActiveItem) {
    emit editEventSignal(mActiveItem->event());
  }
}

void KOTodoView::deleteTodo()
{
  if (mActiveItem) {
    if (mActiveItem->childCount()) {
      QMessageBox::warning(this,"Delete To-Do",
                           "Can not delete To-Do which has children.",
                           QMessageBox::Ok,0);
    } else {
      emit deleteEventSignal(mActiveItem->event());
    }
  }
}

void KOTodoView::purgeCompleted()
{
  QList<KOEvent> todoCal = mCalendar->getTodoList();

  KOEvent *aTodo;
  for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next())
  {
    if (aTodo->getStatus() != KOEvent::NEEDS_ACTION)
      mCalendar->deleteTodo(aTodo);
  }
  updateView();
}



// code below is obsolete

TodoView::TodoView(CalObject *cal, QWidget *parent, const char *name)
        : QWidget(parent, name )
{
  todoBox = new todoViewIn(cal,this);
  connect(todoBox, SIGNAL(editEventSignal(KOEvent *)), 
	  SIGNAL(editEventSignal(KOEvent *)));
  label = new QLabel(i18n("To-Do Items"), this);
  label->setFrameStyle(QFrame::Panel | QFrame::Raised);
}

void TodoView::resizeEvent(QResizeEvent *e)
{
  topH = fontMetrics().height()+4;
  
  todoBox->setGeometry(0,topH,width(),height()-topH);
  label->setGeometry(0,0,width(),topH);
}

todoViewIn::todoViewIn( CalObject *cal, QWidget *parent, 
			      const char *name )
  : QTableView(parent,name)
{
    calendar = cal;

    curRow = curCol = 0;			
    setFocusPolicy( StrongFocus );		
    setBackgroundMode( PaletteBase );		
    setNumCols( 1 );			        
    setCellWidth( 100 );

    setTableFlags(Tbl_clipCellPainting |
                  Tbl_smoothScrolling |
                  Tbl_autoVScrollBar );

    todoIdList.setAutoDelete(TRUE);

    updateConfig(); // set font

    curSortMode = PRIORITY;
    updateView();

    editor = new QLineEdit(this);
    editor->hide();
    connect(editor, SIGNAL(returnPressed()),
            this, SLOT(updateSummary()));
    connect(editor, SIGNAL(returnPressed()),
            editor, SLOT(hide()));
    connect(editor, SIGNAL(textChanged(const QString&)),
            this, SLOT(changeSummary(const QString&)));

    priList = new QListBox(this);
    priList->hide();
    priList->insertItem("1");
    priList->insertItem("2");
    priList->insertItem("3");
    priList->insertItem("4");
    priList->insertItem("5");
    priList->setFixedHeight(priList->itemHeight()*5+5);
    priList->setFixedWidth(priList->maxItemWidth()+5);
    priList->setFocusPolicy(NoFocus);

    connect(priList,SIGNAL(highlighted(int)),
	    SLOT(changePriority(int)));
    connect(priList,SIGNAL(highlighted(int)),
	    priList, SLOT(hide()));


    QPixmap pixmap;
    rmbMenu1 = new QPopupMenu;
    pixmap = BarIcon("todolist");
    rmbMenu1->insertItem(pixmap, i18n("New To-Do"), this,
                         SLOT(newTodo()));
    pixmap = BarIcon("delete");
    rmbMenu1->insertItem(pixmap, i18n("Purge Completed "), this,
                         SLOT(purgeCompleted()));

    rmbMenu2 = new QPopupMenu;
    pixmap = BarIcon("todolist");
    rmbMenu2->insertItem(pixmap, i18n("New To-Do"), this,
                         SLOT (newTodo()));
    rmbMenu2->insertItem(i18n("Edit To-Do"), this,
                         SLOT (editTodo()));
    pixmap = BarIcon("delete");
    rmbMenu2->insertItem(pixmap, i18n("Delete To-Do"), this,
                         SLOT (deleteTodo()));
    rmbMenu2->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
    rmbMenu2->insertItem(i18n("Sort by Priority"), this,
			 SLOT(sortPriority()));
}

todoViewIn::~todoViewIn()
{

}

void todoViewIn::paintCell( QPainter* p, int row, int col )
{
    int w = viewWidth(); // not cellWidth(col), we want scrollbar to count
    int h = cellHeight( row );
    int x2 = w;        // cell width for drawing
    int y2 = h - 1;    // cell height - seperation line for drawing

    QFontMetrics fm(todoFont);
    int priorityWidth=fm.width("0")+2;  // width of priority

    //draw outlines
    
    p->setPen(koconf.windowColor.light(80));
    p->drawLine( x2, 0, x2, y2 );
    p->drawLine( 0, y2, x2, y2 );

    // draw focus rect

    int checkBoxHeight=h-2;
    p->setPen(koconf.textColor);
    
    if ( (row == curRow) && (col == curCol) )
    {	
        if ( hasFocus() )
        {
            p->setBrush(koconf.selectColor);
            p->setPen(koconf.selectColor);
            p->drawRect( checkBoxHeight+priorityWidth, 0, x2, y2 );
            p->drawRect( 0, 0, priorityWidth, y2 );
            p->setPen(koconf.selectTextColor);
        }
    }
    
    // calculate sizes

    int dueHight=fm.height();
    int oldFontSize=todoFont.pointSize();
    //    int dueWidth=fm.width("00:00");
    int dueWidth = fm.width("");

    //calculate font for Due date

    todoFont.setPointSize(oldFontSize-(oldFontSize/3));
    p->setFont(todoFont);

    aTodo = calendar->getTodo(*todoIdList.at(row));
    //    int month = aTodo->getDtEnd().date().month();
    //    int day = aTodo->getDtEnd().date().day();
    
    //    QString due;
    //    due.sprintf("%02d.%02d",month,day);
    //    p->drawText( 0, -1, w, h, AlignRight, due );

    todoFont.setPointSize(oldFontSize);  // set font back to normal

    bool check=FALSE;

    // draw text lines

    p->setFont(todoFont);
    p->drawText( checkBoxHeight+priorityWidth, -1, 
		 w-(dueWidth+priorityWidth+checkBoxHeight), 
    		 h, AlignLeft, aTodo->getSummary() );
    todoFont.setWeight(QFont::Bold);
    p->setFont(todoFont);
    QString pr;
    pr.sprintf("%d",aTodo->getPriority());
    p->drawText( 1,-1, h,h,AlignLeft, pr);
    todoFont.setWeight(QFont::Normal);
    int textWidth=fm.width(aTodo->getSummary());
    if (check) p->drawLine(checkBoxHeight+priorityWidth,dueHight/2,
			   checkBoxHeight+priorityWidth+textWidth, dueHight/2);

    // draw checkbox
    
    checkBoxHeight=h-4;
    p->setPen(QColor(100,100,100));
    p->drawLine(priorityWidth,0,priorityWidth+checkBoxHeight,0);
    p->drawLine(priorityWidth,0,priorityWidth,y2-1);
    p->setPen(QColor(200,200,200));
    p->drawLine(priorityWidth+checkBoxHeight,y2-1, priorityWidth+checkBoxHeight,1);
    p->drawLine(priorityWidth+checkBoxHeight,y2-1, priorityWidth+1, y2-1);
    p->setPen(QColor(20,20,20));
    p->drawLine(priorityWidth+1,1,priorityWidth+checkBoxHeight-1,1);
    p->drawLine(priorityWidth+1,1,priorityWidth+1,y2-2);
    p->setPen(QColor(180,180,180));
    p->drawLine(priorityWidth+checkBoxHeight-1,y2-2, priorityWidth+checkBoxHeight-1,2);
    p->drawLine(priorityWidth+checkBoxHeight-1,y2-2, priorityWidth+2, y2-2);

    // draw check
    
    if (aTodo->getStatusStr() != "NEEDS ACTION")
    {
        p->setPen(koconf.textColor.dark());
        QPen pen;
        pen.setWidth(y2/8);
        p->setPen(pen);
        p->drawLine(priorityWidth+(checkBoxHeight/2), y2-4, priorityWidth+4, y2-4-(y2/4));
        p->drawLine(priorityWidth+(checkBoxHeight/2), y2-4, (priorityWidth+checkBoxHeight)-4, 4);
    }
}

void todoViewIn::mousePressEvent( QMouseEvent* e )
{
  if (editor->isVisible()) {
    updateSummary();
    editor->hide();
  }

    int oldRow = curRow;
    int oldCol = curCol;
    QPoint clickedPos = e->pos();
    curRow = findRow( clickedPos.y() );
    curCol = findCol( clickedPos.x() );
    int x=clickedPos.x();

    if ( (curRow != oldRow) || (curCol != oldCol) )
    {
        updateCell( oldRow, oldCol );
        updateCell( curRow, curCol );
    }
    else
    {
        if (e->button()!=RightButton && curRow>=0)
        {
        aTodo = calendar->getTodo(*todoIdList.at(curRow));
        QFontMetrics fm(todoFont);
        int priorityWidth=fm.width("0")+2;
	//        int dueWidth=fm.width("00.00");
	int dueWidth = fm.width("");

        if (e->button() == LeftButton)
        {
	  updatingRow = curRow;

	  if (x >= 0 && x < priorityWidth) {
	    int cy = 0;
	    rowYPos(curRow, &cy);
	    priList->move(0, cy);
	    priList->clearSelection();
	    priList->show();
	  }
	  
	  if (x>priorityWidth && x<(priorityWidth+cellHeight()-2))
            {
	      if (aTodo->getStatusStr() != "NEEDS ACTION")
                {
		  aTodo->setStatus(QString("NEEDS ACTION"));
		  updateCell( curRow, curCol );
                }
	      else
                {
		  aTodo->setStatus(QString("COMPLETED"));
		  updateCell( curRow, curCol );
                }
            }
	  if (x>(priorityWidth+cellHeight()-2) && x<cellWidth()-dueWidth )
            {
	      aTodo = calendar->getTodo(*todoIdList.at(curRow));
	      editor->setText(aTodo->getSummary());
	      
	      int cx=0, cy=0;
	      colXPos(curCol, &cx);
	      rowYPos(curRow, &cy);
	      
	      editor->move((priorityWidth+cellHeight()-2), cy);
	      editor->setFixedWidth((cellWidth() - dueWidth) - 
				     (priorityWidth + cellHeight() - 2));
	      editor->setFixedHeight(cellHeight(curRow));
	      editor->setFocus();
	      editor->show();
            }
        }
        }
    }
    if (e->button()==RightButton)
    {
        if (curRow<0)
        {
            rmbMenu1->popup(QCursor::pos());
        }
        else
        {
            rmbMenu2->popup(QCursor::pos());
        }
    }
}

void todoViewIn::keyPressEvent( QKeyEvent* e )
{
    int oldRow = curRow;			
    int oldCol = curCol;
    int edge = 0;
    switch( e->key() ) {			
        case Key_Left:				
            if( curCol > 0 ) {			
                curCol--;     			
                edge = leftCell();		
                if ( curCol < edge )		
                    setLeftCell( edge - 1 );	
            }
            break;
        case Key_Right:				
            if( curCol < numCols()-1 ) {
                curCol++;
                edge = lastColVisible();
                if ( curCol >= edge )
                    setLeftCell( leftCell() + 1 );
            }
            break;
        case Key_Up:
            if( curRow > 0 ) {
                curRow--;
                edge = topCell();
                if ( curRow < edge )
                    setTopCell( edge - 1 );
            }
            break;
        case Key_Down:
            if( curRow < numRows()-1 ) {
                curRow++;
                edge = lastRowVisible();
                if ( curRow >= edge )
                    setTopCell( topCell() + 1 );
            }
            break;
        default:				
            e->ignore();			
            return;
    }

    if ( (curRow != oldRow) 			
            || (curCol != oldCol)  ) {
        updateCell( oldRow, oldCol );		
        updateCell( curRow, curCol );		
    }
}

void todoViewIn::focusInEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );		
}

void todoViewIn::focusOutEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );		
}

int todoViewIn::indexOf( int row, int col ) const
{
    return (row * numCols()) + col;
}

void todoViewIn::resizeEvent(QResizeEvent *e)
{
    const QScrollBar *scrollBar = verticalScrollBar();
    int scrollBarWidth=0;
    if (scrollBar->isVisible()) scrollBarWidth=scrollBar->width();
    setCellWidth( width()-scrollBarWidth);
    QTableView::resizeEvent(e);
    QTableView::repaint();
    QTableView::update();
}

void todoViewIn::changePriority(int pri)
{
  pri++;
  if (pri > 5) pri = 1;
  aTodo->setPriority(pri);
  
  priList->clearSelection();
  updateCell(updatingRow, 0);
}

void todoViewIn::changeSummary(const QString &newsum)
{
    tmpSummary = newsum;
}

void todoViewIn::updateSummary()
{
  updateCell(updatingRow,2);
  aTodo->setSummary(tmpSummary);
  editor->setText(""); 
}

void todoViewIn::editTodo()
{
  if(aTodo!=0) {
    emit editEventSignal(aTodo);
  } else {
    qApp->beep();
  }
}

void todoViewIn::updateView()
{
    todoIdList.clear();

    QList<KOEvent> todoCal(calendar->getTodoList());
    int count=0;
    bool inserted;
    KOEvent *aTodo;

    for (aTodo = todoCal.first(); aTodo;
            aTodo = todoCal.next())
    {
      inserted = FALSE;
      switch(curSortMode) {
      case PRIORITY: {
	int pri1, pri2;
	pri1 = aTodo->getPriority();
	for (uint j = 0; j < todoIdList.count(); j++) {
	  pri2 = calendar->getTodo(*todoIdList.at(j))->getPriority();
	  if (pri1 < pri2) {
	    todoIdList.insert(j, new int(aTodo->getEventId()));
	    inserted = TRUE;
	    break;
	  }
	}
	if (!inserted) {
	  todoIdList.append(new int(aTodo->getEventId()));
	}
	break;
      }
      default:
	todoIdList.append(new int(aTodo->getEventId()));
      }
      count++;
    }

    setNumRows( count );

    repaint();
}

void todoViewIn::updateConfig()
{
    KConfig *config(kapp->config());

    config->setGroup("Fonts");

    QFont f = font();
    f=config->readFontEntry("Todo Font",&f);

    todoFont=f;
    QFontMetrics fm(f);
    int h=fm.height();
    if (h<17) h=17;
    setCellHeight(h);
    updateView();
    repaint();
}

void todoViewIn::newTodo()
{
  editor->setText("");
  KOEvent *newTodo;
  QFontMetrics fm(todoFont);
  int priorityWidth=fm.width("0")+2;
  int dueWidth = fm.width("");

  aTodo = newTodo = new KOEvent;
  
  newTodo->setStatus(QString("NEEDS ACTION"));
  newTodo->setPriority(1);
  calendar->addTodo(newTodo);
  todoIdList.append(new int(newTodo->getEventId()));
  setNumRows(todoIdList.count());
  updatingRow = numRows();

  //int cx=0, cy=0;
  int cy=0;
  //  colXPos(curCol, &cx);
  rowYPos(numRows()-1, &cy);

  editor->move((priorityWidth+cellHeight()-2), cy);
  editor->setFixedWidth((cellWidth() - dueWidth) - 
			(priorityWidth + cellHeight() - 2));
  editor->setFixedHeight(cellHeight(numRows()-1));
  editor->setFocus();
  editor->show();
}

void todoViewIn::deleteTodo()
{
  calendar->deleteTodo(aTodo);
  updateView();
}

void todoViewIn::purgeCompleted()
{
    todoIdList.clear();
    QList<KOEvent> todoCal(calendar->getTodoList());
    int count = 0;
    KOEvent *aTodo;

    for (aTodo = todoCal.first(); aTodo;
            aTodo = todoCal.next())
    {
        if (aTodo->getStatusStr() == "NEEDS ACTION")
        {
            todoIdList.append(new int(aTodo->getEventId()));
            count++;
        } else {
            calendar->deleteTodo(aTodo);
        }
    }
    setNumRows( count );
    repaint();
}

void todoViewIn::sortPriority()
{
  curSortMode = PRIORITY;
  updateView();
}

