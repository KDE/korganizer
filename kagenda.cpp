// 	$Id$	

#include <klocale.h>
#include "kglobal.h"

#include "optionsdlg.h"

#include "kagenda.h"
#include "kagenda.moc"

#define DAYMARGIN 1
#define DATELABELSIZE 24
#define CELLWIDTH 40

KAgenda::KAgenda( CalObject *cal, QWidget *parent, const char *name )
		: QFrame( parent, name )
{
	// global Calendar object
	AgendaCal = cal;
	config = kapp->config();

	// increment size for dragging/resizing events.
	timeGridSize = 15;
	
	// we want to delete dynamically allocated dates when we clear...
	selectedDates.setAutoDelete(TRUE);
	selectedDates.append(new QDate(QDate::currentDate()));

	// setup header widgets at top and left, and main view
	dayFrame = new QLabel( this,"dayFrame" );
	allDayFrame = new QFrame( this,"allDayFrame" );
	sheetFrame = new QLabel( this,"sheetFrame" );
	agendaSheet = new KAgendaSheet( KAgendaSheet::Time_24, CELLWIDTH, 
					1, sheetFrame );
	scrollBar = new QScrollBar( QScrollBar::Vertical, this,
				    "eventScrollBar" );

//	QPixmap px;
//	px = loader.loadIcon("1leftarrow" );
//	prevdatbt = new KPButton( px, dayFrame );
//	px = loader.loadIcon("dayicon" );
//	viewbt = new KPButton( px, dayFrame );
//	px = loader.loadIcon("1rightarrow" );
//	nextdatbt = new KPButton( px, dayFrame );
//	connect( prevdatbt, SIGNAL( clicked() ), SLOT( slotPrevDates() ) );
//	connect( nextdatbt, SIGNAL( clicked() ), SLOT( slotNextDates() ) );
//	connect( viewbt, SIGNAL( clicked() ), SLOT( slotViewChange() ) );

	dayFrame->setFixedHeight( DATELABELSIZE );
	allDayFrame->setFixedHeight( 1 );
	//	allDayFrame->setFrameStyle(Panel|Raised);
	//	allDayFrame->setBackgroundMode(PaletteBase);
	scrollBar->setFixedWidth( 16 );
	connect( scrollBar, SIGNAL( valueChanged(int) ), this, SLOT( slotScrollBar(int) ) );
	
	// Agenda init
	WidgetList.setAutoDelete(TRUE);
	dateLabels.setAutoDelete(TRUE);
	setFrameStyle( Box | Sunken );
	setMinimumSize( 100,100 );
	
	// sheet settings
//	agendaSheet->setFocusPolicy( ClickFocus );

	QFont f;
	f = agendaSheet->font();
	f.setFamily("Lucida");
	f.setPointSize(18);
	agendaSheet->setFont( f );

	// create menu and connect to handler
	QPixmap pm;
	pm = loader.loadIcon("delete");

	connect( agendaSheet, SIGNAL( popupActivated(int) ), this, SLOT( sheetPopupSlot( int ) ) );

	connect(agendaSheet, SIGNAL(addNewEventSignal(int, QTime)),
				    this, SLOT(addNewEvent(int, QTime)));

	// read settings from KConfig
	updateConfig();

	// set view type to a 5 day view starting on monday
	agendaSheet->startDate = QDate::currentDate();
	selectedWidget = 0L;
	ViewType = DAY;
	slotViewChange();

	resizeEvent(0);

	scrollBar->setValue(startScrollVal);	
}


void KAgenda::fillAgenda()
{
//	Fill Agenda View day with events
  //  int scrollStart = 0;
  const QString weekday[] = { i18n("Mon"),i18n("Tue"),i18n("Wed"),
                              i18n("Thu"),i18n("Fri"),
                              i18n("Sat"),i18n("Sun") };

  KOEvent *currentevent = 0L;
  // kill obsolete eventwidgets
  setSelected( 0 );
  agendaSheet->setFocus();
  WidgetList.clear();
  floatEvents.clear();
  floatCount = 0;
debug("::fillAgenda begin");
  if( selectedDates.count() ) {
    QLabel *lb;
    QString datetext;
    if ( ! dateLabels.count() ) {
      dateLabels.append( lb = new QLabel( dayFrame ));
    }
    dateLabels.first();		
    // Go through the list making date labels and new event widgets		
    for( selectedDates.first(); selectedDates.current(); selectedDates.next() ) {
      if( (lb = dateLabels.current()) ) {
        dateLabels.next();
      } else {
        lb = new QLabel( dayFrame );
        dateLabels.append( lb );
        dateLabels.next();
      }
      lb->setFrameStyle( Raised | Panel );
      lb->setAlignment( AlignTop | AlignHCenter );
      if (!AgendaCal->getHolidayForDate(*selectedDates.current()).isEmpty()) {
        datetext = QString("%1 %2 - %3").arg
                          (weekday[ selectedDates.current()->dayOfWeek() -1 ]).arg
                          (selectedDates.current()->day()).arg
                          (AgendaCal->getHolidayForDate(*selectedDates.current()));
//        lb->setText(  datetext.sprintf("%s %d - %s",
//                      weekday[ selectedDates.current()->dayOfWeek() -1 ],
//                      selectedDates.current()->day(),
//                      AgendaCal->getHolidayForDate(*selectedDates.current()).data()));
      } else {
        datetext = QString("%1 %2").arg
                          (weekday[ selectedDates.current()->dayOfWeek() -1 ]).arg
                          (selectedDates.current()->day());
//        lb->setText(  datetext.sprintf("%s %d",
//                      weekday[ selectedDates.current()->dayOfWeek() -1 ],
//                      selectedDates.current()->day()));
      }
      lb->setText(datetext);

      QFont newFont(lb->font());
      if (*selectedDates.current() == QDate::currentDate())
        newFont.setBold(TRUE);
      else
        newFont.setBold(FALSE);

      lb->setFont(newFont);

    }		

    // Get events for this date and put em in
    for ( selectedDates.first() ; selectedDates.current() ; selectedDates.next() ) {
      QList<KOEvent> dateevents(AgendaCal->getEventsForDate( *selectedDates.current() ));
      // make a widget for each event
      for ( currentevent = dateevents.first(); currentevent; currentevent = dateevents.next() )
        newEventWidget(currentevent, *selectedDates.current());
    }
  }

  // set grid dimensions
  agendaSheet->setColumns( selectedDates.count() );
  // and layout the widgets
  doLayout();
debug("::fillAgenda end");
}

EventWidget *KAgenda::newEventWidget(KOEvent *currEvent, QDate displayDate)
{
  EventWidget *ew;

  if (currEvent->isMultiDay()) {
    if (floatEvents.findRef(currEvent) == -1) {
      ++floatCount;
      floatEvents.append(currEvent);
    } else {
      return 0;
    }
  }

  // also need to add to floatcount all those events w/no time associated.
  if (currEvent->doesFloat() && (floatEvents.findRef(currEvent) == -1))
    ++floatCount;

  QString wName;
  if (currEvent->doesFloat() || currEvent->isMultiDay()) {
    ew = new EventWidget(currEvent,AgendaCal, displayDate,allDayFrame,
			 (wName.sprintf("%d",rand())).data());
  } else {
    ew = new EventWidget(currEvent,AgendaCal, displayDate,agendaSheet,
			 (wName.sprintf("%d",rand())).data());
  }
  
  connect(ew, SIGNAL( selected(EventWidget*)), 
	     this, SLOT(setSelected(EventWidget*)));

  connect(ew, SIGNAL(editEventSignal(KOEvent *)), 
	  this, SIGNAL(editEventSignal(KOEvent *)));

  connect(ew, SIGNAL(deleteEventSignal(KOEvent *)), 
	  this, SIGNAL(deleteEventSignal(KOEvent *)));

  connect(ew, SIGNAL(widgetState(EventWidget *, int)),
	  this, SLOT(updateEventWidget(EventWidget *, int)));

  connect(ew, SIGNAL(eventChanged(KOEvent *, int)),
	  this, SLOT(changeEventDisplay(KOEvent *, int)));

  // this has been supplanted by a configure option.
  //  ew->setHeaderFont( font() );
  //  ew->setBodyFont( font() );

  QString ttext;
  if (currEvent->doesFloat())
    ttext = i18n("All Day Event");
  else
    ttext = ew->text();

  ew->setToolTip(ttext);
  
  WidgetList.append( ew );

  return ew;
}

void KAgenda::addNewEvent(int col, QTime t)
{
  KOEvent *newEvent = new KOEvent;
  EventWidget *ew;
  int endhour, endminute;

  QDate newDate = agendaSheet->startDate.addDays(col);
  endhour = t.hour()+1;
  endminute = t.minute();
  newEvent->setDtStart(QDateTime(newDate, t));
  newEvent->setDtEnd(QDateTime(newDate,
			       QTime(endhour, endminute, 0)));
  newEvent->setFloats(FALSE);
  AgendaCal->addEvent(newEvent);
  // do we want to reverse these?
  ew = newEventWidget(newEvent, newDate);
  doLayout();
  setSelected(ew);
  ew->startEdit();
}

void KAgenda::slotScrollBar( int value )
{
	agendaSheet->move( 0, -value );
}

void KAgenda::sheetPopupSlot( int index )
{
  switch( index )
    {
    case 0:
      emit newEventSignal( QDateTime(agendaSheet->getSelectedDate(), 
				     agendaSheet->getSelectedTime()) );
      return;
    case 1:
      emit optionsSignal( 3 );
      PropDlg->show();
      return;
    default:
      return;
    }
}

void KAgenda::resizeEvent( QResizeEvent * )
{
	// debug("begin resize event");
	// position the prev/next/view buttons
//	prevdatbt->setGeometry(0, 0, 
//			       (agendaSheet->gridOffset()-DATELABELSIZE)/2, 
//			       dayFrame->height());
//	nextdatbt->setGeometry((agendaSheet->gridOffset()-DATELABELSIZE)/2 +
//			       DATELABELSIZE, 0, (agendaSheet->gridOffset() - 
//						  DATELABELSIZE)/2, 
//			       dayFrame->height());

//	viewbt->setGeometry(prevdatbt->width(), 0, 
//			    DATELABELSIZE,DATELABELSIZE);

	agendaSheet->resize( sheetFrame->width(), 100 );

	// and realign event widgets
	doLayout();
}

void KAgenda::keyPressEvent( QKeyEvent *kev )
{
	switch( kev->key() )
	{
	case Key_PageDown:
		scrollBar->addPage();
		break;
	case Key_PageUp:
		scrollBar->subtractPage();
		break;
	case Key_Down:
		scrollBar->addLine();
		break;
	case Key_Up:
		scrollBar->subtractLine();
		break;
	default:
		;
	}
}

void KAgenda::doLayout()
{
  EventWidget *ew;
  QString time;
  QLabel *lb;
  int i = 0;
  static QHBoxLayout *topLayout = 0;
  static QVBoxLayout *subLayout = 0;

  // hide the widgets for faster update
  for( WidgetList.first(); (ew = WidgetList.current()); WidgetList.next() )
    ew->hide();

  // floatCount is the number of floating events.
  allDayFrame->setFixedHeight(1+agendaSheet->rowSize()*floatCount);

  if (topLayout) {
    delete topLayout; topLayout = 0;
//    delete subLayout; subLayout = 0;
  }

  // the layout managers for the view
  topLayout = new QHBoxLayout( this, 2, 1, "KAgenda::topLayout" );
  subLayout = new QVBoxLayout( topLayout, 1, "KAgenda::subLayout" );
  topLayout->addWidget( scrollBar );
  subLayout->addWidget( dayFrame );
  subLayout->addWidget( allDayFrame );
  subLayout->addWidget( sheetFrame );
  // don't activate unless we have some stuff (kind of hacked)
  if (floatCount > 0)
    topLayout->activate();

  // layout the date headers
  for( dateLabels.first(); (lb = dateLabels.current());
       dateLabels.next() )
    {
      int lw = agendaSheet->colSize();
      int lx = agendaSheet->gridOffset() + 
               dateLabels.at() *
               agendaSheet->colSize();

      lb->setGeometry( lx,0,lw, DATELABELSIZE );
      lb->show();
    }
  
  // walk through all event widgets
  WidgetList.first();
  while((ew = WidgetList.current()) != 0 )
    {
      int x,y,w,h, daycount = 0;
      const KOEvent *event;
      
      // skip empty dates
      selectedDates.first();
      while( selectedDates.current() && ew->date() != 
	     *selectedDates.current() ) {
        selectedDates.next();
        daycount++;
      }
      if ( ! selectedDates.current() ) {
        debug("PARSE ERROR in Layout Event widgets" );
        break;
      }
      
      // get event + data from widget
      event = ew->getEvent();
      QTime starttime = event->getDtStart().time();
      QTime endtime = event->getDtEnd().time();
      
      // calculate proper geometry
      if (event->doesFloat() || event->isMultiDay())
      {
        // for multi-day events, make width span whole range of displayed dates
        uint days = event->getDtStart().daysTo(event->getDtEnd().date());
        // we also need to put a fix in here for recurring events,
        // but most multi-day events DON'T recur.  This hack should fix
        // probably 75% of the times that it displays too "wide" an appt.
        if (!event->doesRecur())
          if (event->getDtStart().date() != *selectedDates.current())
            days = selectedDates.current()->daysTo(event->getDtEnd().date());

        if (days > selectedDates.count())
          days = selectedDates.count();
        w = ((1 + days) * agendaSheet->colSize()) - 2;
        h = agendaSheet->rowSize();
        x = agendaSheet->gridOffset() + daycount * agendaSheet->colSize() + 1;
        y = i * h;
        i++;
        agendaSheet->setWidgetGeometry( ew, x, y, w, h);
      } else {
        agendaSheet->setWidgetGeometry( ew, starttime, endtime, daycount );
      }
      
      WidgetList.next();
    }

  // sync scrollbar dimensions with view
  scrollBar->setSteps( agendaSheet->rowSize(), 
		       sheetFrame->height() );
  if(agendaSheet->height() - sheetFrame->height() > 0) {
    scrollBar->setFixedWidth( 16 );
    scrollBar->setRange(0, agendaSheet->height() - 
			sheetFrame->height());
    scrollBar->show();
  } else {
    scrollBar->setFixedWidth( 4 );
    scrollBar->hide();
  }

  for( WidgetList.first(); 
       (ew = WidgetList.current()); 
       WidgetList.next() )
    ew->show();
  
}

KAgenda::~KAgenda()
{
}

int KAgenda::getSelectedId()
{
  if( selectedWidget )
    return selectedWidget->getEvent()->getEventId();
  else
    return 0;
}

KOEvent *KAgenda::getSelected()
{
  // get event from selected event widget
  if( selectedWidget )
    return selectedWidget->getEvent();
  else
    return 0;
}

void KAgenda::setSelected( EventWidget *selew )
{
  // first deselect the old widget if not the same  
  if( selectedWidget && (selew != selectedWidget) )
    selectedWidget->setSelected(FALSE);
  
  // now select new widget if any
  selectedWidget = selew;
  if( selectedWidget )
    selectedWidget->setSelected(TRUE);
}

// new stuff
void KAgenda::updateEventWidget( EventWidget *ew, int action )
{
  QDate newdate;
  QTime newTime;
  QDateTime dtstart(ew->getEvent()->getDtStart()); 
  QDateTime dtend(ew->getEvent()->getDtEnd());
  bool doesFloat(ew->getEvent()->doesFloat());
  //  bool doesRecur(ew->getEvent()->doesRecur());
  bool isMultiDay(ew->getEvent()->isMultiDay());

  int dtsize = dtstart.secsTo( ew->getEvent()->getDtEnd() );
  int col = (ew->dragPosX() - agendaSheet->gridOffset()) / agendaSheet->colSize();
  int newY;

  if( col >= agendaSheet->columns() || col < -1 )
    col =-1;
  
  switch( action ) {
  case EventWidget::MOVE:
    if (doesFloat || isMultiDay)
      return;
    if (col != -1)
      newdate = *selectedDates.at( col);
    else
      newdate = dtstart.date();
    
    dtstart.setDate( newdate ); 
    
    // always keep moved event within view
    
    // scroll up?
    if (-agendaSheet->y() > ew->dragPosY()) {
      scrollBar->subtractLine();
      newY = scrollBar->value() + 1;
    }	// no bottom hit, scroll down ?
    else if (-agendaSheet->y() + sheetFrame->height() < 
	     ew->dragPosY() + ew->height()) {
      scrollBar->addLine();
      newY = scrollBar->value() + sheetFrame->height() - ew->height();
    } else {  // no scroll
      newY = ew->dragPosY();
    }

    // what happens when ew hits the bottom...
    if (newY + ew->height() >= agendaSheet->height() -1) {
      newY = agendaSheet->height() - ew->height() -1 ;
    }

    newTime = yposToTime( newY );
    ASSERT(newTime.isValid());
    if (newTime.isValid())
    {
    	dtstart.setTime(newTime);
    	dtend = dtstart.addSecs( dtsize ) ;
    }
    break;
    
  case EventWidget::RESIZETOP:
    if (doesFloat || isMultiDay)
      return;
    newTime = yposToTime(ew->resizePosY());
    if (newTime.isValid())
    {
      dtstart.setTime(newTime);
      col =-1; // <- i dont understand why this is necessary but it is.
      
    }
    break;
    
  case EventWidget::RESIZEBOTTOM:
    if (doesFloat || isMultiDay)
      return;
    newTime = yposToTime(ew->resizePosY());
    if (newTime.isValid()) {
      dtend.setTime(newTime);
      col =-1;
      
    }
    break;
  case EventWidget::RESIZELEFT:
  case EventWidget::RESIZERIGHT:
  case EventWidget::DTCHANGE:
  case EventWidget::CONTENTS:
    break;
  }
  // granularity moved to yposTotime()
  
  // set date/time -- it might or might not be more 
  // efficient to do this only if things have actually changed.
  ew->getEvent()->setDtStart( dtstart );
  ew->getEvent()->setDtEnd( dtend );
  // update widget's date, which may be different from dtstart date
  // because of recurrence (it is an instance, not the description
  // for the instance)
  if (col > -1)
    ew->setDate(*selectedDates.at( col));

  // generic update -> good idea !
  agendaSheet->setWidgetGeometry( ew, ew->getEvent()->getDtStart().time(),
				  ew->getEvent()->getDtEnd().time(),
				  col );
}

QTime KAgenda::yposToTime(int y)
{
  int hour;
  int minutes;
  int rs;
  
  rs = agendaSheet->rowSize();
  hour = y / rs;

  minutes = (y - hour*rs ) *60/rs;
  minutes = minutes - minutes%timeGridSize;
  if (y != 0 && hour == 24)
    return QTime(23,59,0);
  else
    return QTime(hour, minutes, 0);
}


void KAgenda::updateConfig()
{
  int fmt;

  config->setGroup("Time & Date");
  fmt = config->readNumEntry("Time Format", 1);
  agendaSheet->setTimeStyle((fmt == 1 ? KAgendaSheet::Time_12:KAgendaSheet::Time_24));
  weekStartsMonday = config->readBoolEntry("Week Starts Monday", TRUE);
  
  config->setGroup("Views");
  QString startStr(config->readEntry("Day Begins", "8:00"));
  // handle case where old config files are in format "8" instead of "8:00".
  int colonPos = startStr.find(':');
  if (colonPos >= 0)
    startStr.truncate(colonPos);
  int startHour = startStr.toUInt();
  startScrollVal = scrollBar->minValue() + 
    (agendaSheet->rowSize() * startHour);

	config->setGroup("Fonts");
	agendaSheet->setTimeFont( config->readFontEntry( "TimeBar Font" ) );


	QListIterator<EventWidget> iter(WidgetList);
	EventWidget *tmpWidget;
	iter.toFirst();
	while ((tmpWidget = iter.current()))
	{
		tmpWidget->updateConfig();
		++iter;
	}
	
	agendaSheet->updateConfig();
	update();
}

void KAgendaSheet::paletteChange( QPalette )
{
	updateConfig();
}

void KAgendaSheet::updateConfig()
{
	KConfig *config = kapp->config();
	
	config->setGroup( "Colors" );
	//	QColor c(kapp->backgroundColor.dark(120));
	QColor c(koconf.backgroundColor.dark(120));
	setPalette( config->readColorEntry( "SheetBackground",
		    &c));

	update();
}

void KAgenda::changeEventDisplay(KOEvent *, int)
{
  // this should be re-written to be MUCH smarter.  Right now we
  // are just playing dumb.
  setView(ViewType);

  fillAgenda();
}

void KAgenda::slotPrevDates()
{
  int datenum, count;

  switch( ViewType ) {
  case DAY:
    agendaSheet->startDate = agendaSheet->startDate.addDays(-1);
    selectedDates.clear();
    selectedDates.append(new QDate(agendaSheet->startDate));
    break;
  case WORKWEEK:
    agendaSheet->startDate = agendaSheet->startDate.addDays(-7);
    selectedDates.clear();
    for (count = 0; count < 5; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  case WEEK:
    agendaSheet->startDate = agendaSheet->startDate.addDays(-7);
    selectedDates.clear();
    for( count = 0; count < 7; count++ )
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  case LIST:
    datenum = selectedDates.count();
    agendaSheet->startDate = agendaSheet->startDate.addDays( -datenum );
    selectedDates.clear();
    for (count = 0; count < datenum; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  }
  emit datesSelected(selectedDates);
  setView(ViewType);
  fillAgenda();
}

void KAgenda::slotNextDates()
{
  int datenum, count;

  switch(ViewType) {
  case DAY:
    agendaSheet->startDate = agendaSheet->startDate.addDays(1);
    selectedDates.clear();
    selectedDates.append(new QDate(agendaSheet->startDate));
    break;
  case WORKWEEK:
    agendaSheet->startDate = agendaSheet->startDate.addDays(7);
    selectedDates.clear();
    for (count = 0; count < 5; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  case WEEK:
    agendaSheet->startDate = agendaSheet->startDate.addDays( 7 );
    selectedDates.clear();
    for( count = 0; count < 7; count++ )
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  case LIST:
    datenum = selectedDates.count();
    agendaSheet->startDate = selectedDates.last()->addDays( 1 );
    selectedDates.clear();
    for (count = 0; count < datenum; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));
    break;
  }
  emit datesSelected(selectedDates);
  setView(ViewType);
  fillAgenda();
}

void KAgenda::slotViewChange(int newView)
{
  int datenum, count;

  switch (newView) {
  case DAY:
    selectedDates.clear();
    selectedDates.append(new QDate(agendaSheet->startDate));

    break;

  case WORKWEEK:
    // find monday for this week
    if (agendaSheet->startDate.dayOfWeek() == 7) {
      if (weekStartsMonday)
        agendaSheet->startDate = agendaSheet->startDate.addDays(-6);
      else
        agendaSheet->startDate = agendaSheet->startDate.addDays(1);
    } else if (agendaSheet->startDate.dayOfWeek() > 1) {
      agendaSheet->startDate = agendaSheet->startDate.addDays(agendaSheet->startDate.dayOfWeek() * -1 + 1);
    }

    selectedDates.clear();
    for (count = 0; count < 5; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));

    break;

  case WEEK:
    // find the beginning of this week (could be monday or sunday)
    if (agendaSheet->startDate.dayOfWeek() == 7) {
      if (weekStartsMonday)
        agendaSheet->startDate = agendaSheet->startDate.addDays(-6);
    } else if (weekStartsMonday) {
      agendaSheet->startDate = agendaSheet->startDate.addDays(agendaSheet->startDate.dayOfWeek() * -1 + 1);
    } else {
      agendaSheet->startDate = agendaSheet->startDate.addDays(agendaSheet->startDate.dayOfWeek() * -1);
    }

    selectedDates.clear();
    for( count = 0; count < 7; count++ )
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));

    break;

  case LIST:
    datenum = selectedDates.count();
    selectedDates.clear();
    for (count = 0; count < datenum; count++)
      selectedDates.append(new QDate(agendaSheet->startDate.addDays(count)));

    break;

  }

  emit datesSelected(selectedDates);
  setView(newView);
  fillAgenda();
}

void KAgenda::setView( int view )
{
  // change view type if valid
  if( selectedDates.first() ) {
    if ((view >= DAY) && (view <= LIST))
      ViewType = view;
    else 
      ViewType = DAY;
  } else
    ViewType = DAY;
}

void KAgenda::selectDates( const QDateList list )
{
  selectedDates.clear();
  selectedDates = list;
  agendaSheet->startDate = *selectedDates.first();

  // if there are 5 dates and the first is a monday, we have a workweek.
  if ((selectedDates.count() == 5) && (selectedDates.first()->dayOfWeek() == 1) &&
      (selectedDates.first()->daysTo(*selectedDates.last()) == 4)) {
    setView(WORKWEEK);

  // if there are 7 dates and the first date is a monday, we have a regular week.
  } else if ((selectedDates.count() == 7) && (selectedDates.first()->dayOfWeek() == (weekStartsMonday ? 1 : 7)) &&
	   (selectedDates.first()->daysTo(*selectedDates.last()) == 6)) {
    setView(WEEK);

  } else if (selectedDates.count() == 1) {
    setView(DAY);

  } else {
    // for sanity, set viewtype to LIST for now...
    setView(LIST);

  }

  // and update the view
  fillAgenda();
}		

KAgendaSheet::KAgendaSheet( int style, int size, int cols, 
			    QWidget *parent, char *name )
  : QWidget( parent, name )
{
  selRow = 0;
  TimeZone = new QFrame( this );
  setTimeStyle( style );
  setRowSize( size );
  setColumns( cols );
  setFocusPolicy( ClickFocus );
  setPalettePropagation( NoChildren );
  
  TimeZone->installEventFilter( this );
  TimeZone->setMouseTracking( TRUE );
  
  // create menu and connect to handler
  sheetPopup = new QPopupMenu();
  QPixmap pixmap = BarIcon("newevent");
  sheetPopup->insertItem(pixmap,i18n("New appointment") );
  //sheetPopup->insertItem( "Options" );
  connect( sheetPopup, SIGNAL( activated(int) ), 
	   this, SIGNAL( popupActivated( int ) ) );
}

KAgendaSheet::~KAgendaSheet()
{
}

void KAgendaSheet::setWidgetGeometry( QWidget *widget, int x, int y, 
				      int w, int h )
{
  //	debug("set ew geom time");
  int xpix = x ;//+ gridOffset();
  widget->setGeometry( xpix,y,w/*colSize()-1*/,h );
}

void KAgendaSheet::setWidgetGeometry( QWidget *widget, 
				      QTime start, QTime end, int col )
{
  int x;
  int y = (start.hour()*60+start.minute() )*40/60;
  //	debug("set ew xywh %d",y);
  if( col > -1 )
    x = col * colSize() + gridOffset() + 1;
  else	x = widget->x();
  int w = colSize() - 1;
  int h = (end.hour()*60+end.minute() )*40/60 - y;
  widget->setGeometry( x,y,w,h );
}

void KAgendaSheet::paintEvent( QPaintEvent *pev )
{
  int t;
  int p_start, p_end, p_wid;
  //	int p_hi, p_count;
  QString time;
  QPainter paint;
  
  debug( "sheet paintEvent");
  
  paint.begin( this );
  
  p_start = pev->rect().top();
  p_end = pev->rect().bottom();
  p_wid = colSize();
  //	p_hi = height();
  
  // draw vertical lines
  for( t=0 ; t <= Columns; t++ ) {
    int x;
    x = t * p_wid + gridOffset();
    //		debug("vert:%d,%d",t,x );
    paint.setPen( palette().normal().dark() );
    paint.drawLine( x, p_start, x, p_end );
  }
  
  p_start = pev->rect().top()/rowSize();
  p_end = pev->rect().bottom()/rowSize();
  p_wid = width();

  int y;
  for (t= p_start; t <= p_end; t++ ) {
    y = t * rowSize() ;
    paint.setPen(palette().normal().dark());
    paint.drawLine( gridOffset(), y, p_wid, y );
  }

  paint.end(); 
}

void KAgendaSheet::setTimeFont( QFont f )
{
	QFont temp = f;
	QFontMetrics tfm( temp );
	
	_timeFont = f;

	int hourWidth = tfm.width("88");
	temp.setPointSize( _timeFont.pointSize()*2/3 );
	tfm = QFontMetrics( temp );
	int extWidth = tfm.width("88");
	
	_timeWidth = hourWidth + extWidth + 8;
	
	resizeEvent(0);
}

int KAgendaSheet::timeWidth()
{
	return _timeWidth;
}

QFont KAgendaSheet::timeFont()
{
	return _timeFont;
}

void KAgendaSheet::timePaintEvent( QPaintEvent *pev )
{
	QString time = "88", timeExt = "88";
	register int hour, y;
	QPainter paint;
	int hourWidth;
	int hourHeight;
	int extHeight;
    QFont hourFont = _timeFont;
	QFont extFont = _timeFont;
	extFont.setPointSize( _timeFont.pointSize()*2/3 );

    QFontMetrics hourFm( hourFont );
	QFontMetrics extFm( extFont );
	
	// set time extension am/pm or 00
	hourWidth = hourFm.width( time );
	
	// remember these are NEGATIVE numbers !
	hourHeight = hourFm.boundingRect( time.data() ).top() ;
	extHeight = extFm.boundingRect( timeExt.data() ).top() ;

	//debug("hourheight:%d extHeight:%d hourWidth:%d", hourHeight, extHeight, hourWidth );

	paint.begin(TimeZone);
	int p_start = pev->rect().top()/rowSize() - 1;
	int p_end = pev->rect().bottom()/rowSize() + 1;
	int p_wid = pev->rect().width() - 10;
  
	// Draw time bar rect that needs update
	for( hour = p_start; hour < p_end; hour++ )
	  {
	    
	    if( TimeStyle == Time_12 )
	      {
		if (hour == 0)
		  {
				//hour = 12;
		    timeExt = i18n("am");
		  } 
		else if (hour > 11)
		  {
		    timeExt = i18n("pm");
				//if (hour != 12)
				//	hour -= 12;
		  } else 
		    timeExt = i18n("am");
	      }
	    else
	      timeExt = "00";
	    
	    if (TimeStyle == Time_12)
	      if (hour == 0)
		time.setNum(12);
	      else
		time.setNum( (hour <= 12) ? hour : (hour - 12) );
	    else
	      time.setNum(hour);

	    // draw line
	    y = hour * rowSize();
	    paint.setPen(PaletteDark);
	    paint.drawLine( 5, y, p_wid, y );
	    
		// draw hour
	    paint.setFont( hourFont );
	    paint.drawText(2, y+2, 
			   abs(hourWidth), rowSize(),
			   AlignTop | AlignRight, time );

		// draw extension
	    paint.setFont( extFont );
	    paint.drawText( hourWidth + 2, y - extHeight + 2, timeExt );

	    // draw half line
	    y += rowSize() / 2;
	    paint.drawLine( 5+p_wid/2, y , p_wid, y );
	    paint.setPen(black);
    
	}
    paint.end();
}

bool KAgendaSheet::eventFilter( QObject */*obj*/, QEvent *ev )
{
  // why the hell does this happen in an eventFilter?  Should just be
  // called from KAgendaSheet paintEvent method.
  // This needs to be done here to catch paint events from the timebar 
  // itself. - Fester
  if( ev->type() == QEvent::Paint ) {
      timePaintEvent( (QPaintEvent *)ev );
  } 
  return FALSE;
}

void KAgendaSheet::resizeEvent( QResizeEvent *)
{
  TimeZone->setGeometry( 0,0, timeWidth(), height() - 2 );
}

void KAgendaSheet::mousePressEvent( QMouseEvent *mev )
{
  emit clearSelection();
  selTime.setHMS((int) mev->y() / RowSize, 0, 0);
  selRow = (int) mev->y() / RowSize;
  selDate = startDate.addDays((int) (mev->x() - gridOffset()) 
			      / colSize());
  if( mev->button() == RightButton )
    sheetPopup->popup( QCursor::pos() );
  
}

void KAgendaSheet::mouseDoubleClickEvent(QMouseEvent *mev)
{
  if (mev->button() == LeftButton) {
    int clickedHour = (int) mev->y() / RowSize;
    int halfHour = (int) ((mev->y() % RowSize) / ((int) (RowSize / 2)));
    int col = (mev->x() - gridOffset()) / colSize();
    // this is wrong when we have multiple dates displayed at once
    // at least as far as the date goes (the time should be correct)
    emit addNewEventSignal(col, QTime(clickedHour,(halfHour > 0 ? 30 : 0), 0));
						       
  }  
}

void KAgendaSheet::setTimeStyle( int style )
{
  TimeStyle = style;
  TimeZone->update();
}

int KAgendaSheet::timeStyle()
{	return TimeStyle; }

void KAgendaSheet::setColumns( int col )
{
  if( col > 0 ) Columns = col;
  else Columns = 1;
  update();
}

int KAgendaSheet::columns()
{	return Columns;}

void KAgendaSheet::setRowSize( int rowsize )
{
  RowSize = rowsize;
  setFixedHeight( RowSize * 24 );
  update();
}

int KAgendaSheet::rowSize()
{	return RowSize; }

int KAgendaSheet::colSize()
{	return ( width() - gridOffset() - 1 ) / Columns; }

int KAgendaSheet::gridOffset()
{
  //debug("TimeZone:%x",TimeZone );
  return TimeZone->width();
}
