// 	$Id$	

#include <qstring.h>
#include <qfontmet.h>

#include "ksmalldaydisp.h"
#include "ksmalldaydisp.moc"

void KNoScrollListBox::keyPressEvent(QKeyEvent *e) 
{
  switch(e->key()) {
  case Key_Right:
    setXOffset(xOffset()+2);
    break; 
  case Key_Left:
    setXOffset(xOffset()-2);
    break;
  case Key_Up:
    setCurrentItem((currentItem()+count()-1)%count());
    if(!itemVisible(currentItem())) {
      if((unsigned int) currentItem() == (count()-1)) {
	setTopItem(currentItem()-numItemsVisible()+1);
      } else {
	setTopItem(topItem()-1);
      }
    }
    break;
  case Key_Down:
    setCurrentItem((currentItem()+1)%count());
    if(!itemVisible(currentItem())) {
      if(currentItem() == 0) {
	setTopItem(0);
      } else {
	setTopItem(topItem()+1);
      }
    }
  case Key_Shift:
    emit shiftDown();
    break;
  default:
    break; 
  }
}

void KNoScrollListBox::keyReleaseEvent(QKeyEvent *e) 
{
  switch(e->key()) {
  case Key_Shift:
    emit shiftUp();
    break;
  default:
    break;
  }
}
void KNoScrollListBox::mousePressEvent(QMouseEvent *e)
{
  if(e->button() == RightButton) {
    qDebug("right mouse button pressed");
  } 
  QListBox::mousePressEvent(e);
}

const char *monthnames[] = {"January", "February", "March", "April", "May",
			    "June", "July", "August", "September", "October",
			    "November", "December"};

KDPSmallDayDisp::KDPSmallDayDisp(QWidget    *parent, 
				 CalObject  *calendar, 
				 QDate       qd,
				 int         index,
				 bool        showFullHeader,
				 const char *name) 
  : QFrame(parent, name)
{
  shiftPressed = FALSE;
  setFrameStyle(NoFrame);

  header = new KNoScrollListBox(this);

  QFont tmpFont = header->font();
  tmpFont.setBold(TRUE);

  header->setFont(tmpFont);
  header->setFrameStyle(NoFrame);
//   header->setFixedHeight(header->fontMetrics().height());
  header->setFixedHeight(20);
  header->move(0,5);

  summaries = new  KNoScrollListBox(this);
  summaries->setFrameStyle(NoFrame);
  summaries->move(0, header->geometry().bottomLeft().y());
  connect(summaries, SIGNAL(selected(int)),    this, SLOT(eventSelected(int)));
  connect(summaries, SIGNAL(highlighted(int)), this, SLOT(eventHilited(int)));
  connect(summaries, SIGNAL(shiftDown()),      this, SLOT(shiftDown()));
  connect(summaries, SIGNAL(shiftUp()),        this, SLOT(shiftUp()));
  connect(header, SIGNAL(highlighted(int)), this, SLOT(daySelected(int)));
  connect(header, SIGNAL(shiftDown()),      this, SLOT(shiftDown()));
  connect(header, SIGNAL(shiftUp()),        this, SLOT(shiftUp()));

  currIdxs = new QIntDict<KOEvent>(101); /* nobody should have more
					     than 101 events on any
					     given day. */

  myCal = calendar;
  myDate = qd;
  myIndex = index;
  selected = FALSE;
  evtSelected = -1;

  setBackgroundColor(QColor(white));
  header->setBackgroundColor(QColor(white));
  summaries->setBackgroundColor(QColor(white));

  this->showFullHeader = showFullHeader;
  updateDisp();
}


KDPSmallDayDisp::~KDPSmallDayDisp()
{
}

QDate KDPSmallDayDisp::getDate()
{
  return(myDate);
}

void KDPSmallDayDisp::updateDisp()
{
  QString datestring, sumstring, ampm, tmpstring;
  KOEvent *anEvent;
  QTime t1;
  int h, i;
  QStrList *summarylist;
  QFontMetrics fontmetric(font());

//   if (myDate.month() % 2) {
//     setBackgroundColor(QColor(lightGray));
//     header->setBackgroundColor(QColor(lightGray));
//     summaries->setBackgroundColor(QColor(lightGray));
//   } else {
//     setBackgroundColor(QColor(white));
//     header->setBackgroundColor(QColor(white));
//     summaries->setBackgroundColor(QColor(white));
//   }
//   this->update();

  // make the header text.
  if(showFullHeader || myDate.day() == 1) {
    // we need a string of the form "January 23"
    datestring = monthnames[myDate.month()-1];
    datestring += " ";
    tmpstring.setNum(myDate.day());
    datestring += tmpstring;
  } else {
    // we just need the day number.
    datestring.setNum(myDate.day());
  }

  // pad out the date so that it is right aligned. Not cute, but seems
  // to work.
  while(fontmetric.width(datestring) < header->width()-10) {
    datestring.insert(0, " ");
  }

  if(header->count()) {
    header->removeItem(0);
  }
  
  header->insertItem(datestring);
  if(selected) {
    header->setSelected(0, TRUE);
  }
  header->repaint();

  // make the summary list.
  if(myCal == 0) {
    // there won't be any summaries... since there isn't a calendar
    // object.
    return;
  }
  summaries->clear();
  summarylist = new QStrList;
  currIdxs->clear();
  events = myCal->getEventsForDate(myDate);

  for(i = 0, anEvent = events.first(); 
      anEvent != 0; 
      i++, anEvent = events.next()) {
    t1 = anEvent->getDtStart().time();
    h = t1.hour();
    if(h == 0) {
      h = 12;
      ampm = "am";
    } else if(h > 11) {
      ampm = "pm";
      if(h != 12) {
	h -= 12;
      } 
    } else {
      ampm = "am";
    }
    sumstring.sprintf("%2d:%02d%s %s", h, t1.minute(), ampm.data(),
		      anEvent->getSummary().data());
    sumstring.detach();
    summarylist->append(sumstring);
    currIdxs->insert(i, anEvent);
  }

  summaries->insertStrList(summarylist);
  if(evtSelected > -1 && summaries->count() > (unsigned int) evtSelected) {
    summaries->setCurrentItem(evtSelected);
    summaries->centerCurrentItem();
  }
  summaries->repaint();
}

void KDPSmallDayDisp::setDate(QDate qd)
{
  myDate = qd;
  updateDisp();
}

void KDPSmallDayDisp::setShowFullHeader(bool on) 
{
  if(showFullHeader == on) {
    // no need to update.
    return;
  }
  showFullHeader = on;
  updateDisp();
}

void KDPSmallDayDisp::setSelected(bool on)
{
//   qDebug("entered setSelected for date: %s, with on = %d",
// 	 myDate.toString().data(), on?1:0);

  if(selected == on) {
    return;
  }

  selected = on;
  
  if(on) {
    header->setSelected(0, TRUE);
    return;
  } 
  // otherwise, clear all of the selections.
  summaries->clearSelection();
  header->clearSelection();
}

void KDPSmallDayDisp::eventSelected(int i)
{
  KOEvent *anEvent;

  anEvent = currIdxs->find(i);
  if (!anEvent)
    qDebug("error, event not found in dictionary");
  else
    emit editEventSignal(anEvent);
}

void KDPSmallDayDisp::eventHilited(int i)
{
  evtSelected = i;
  if(shiftPressed) {
    emit shiftDaySelectedSignal(myDate, myIndex);
  } else {
    emit daySelectedSignal(myDate, myIndex);
  }
}

void KDPSmallDayDisp::daySelected(int)
{
  if(shiftPressed) {
    emit shiftDaySelectedSignal(myDate, myIndex);
  } else {
    emit daySelectedSignal(myDate, myIndex);
  }
}

void KDPSmallDayDisp::resizeEvent(QResizeEvent *e) 
{
//   qDebug("in KDPSmallDayDisp resizeEvent");

  header->resize(e->size().width(), header->height());
  summaries->resize(e->size().width(),
		    e->size().height()-header->height());
		    
  updateDisp();
}

void KDPSmallDayDisp::shiftDown() 
{
  shiftPressed = TRUE;
}

void KDPSmallDayDisp::shiftUp() 
{
  shiftPressed = FALSE;
}
