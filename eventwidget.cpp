/*
 $Id$
 (C) 1998 F.Zigterman, Preston Brown
*/

#include <stdio.h>
#include <stdlib.h>

#include <qpopupmenu.h>
#include <qpainter.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>

#include "calobject.h"
#include "vcaldrag.h"
#include "eventwidget.h"
#include "eventwidget.moc"

#define DRAGTHRESHOLD 10
#define DRAGBORDER 5

#define MAXPMSIZE 15
#define PMMARGIN 5
#define HANDLESIZE 12

/*
 * KTextBox Widget
 */
KTextBox::KTextBox( QWidget *parent, char *name )
  : QMultiLineEdit( parent, name )
{
  clearTableFlags( Tbl_autoScrollBars );
  setFrameStyle( NoFrame );
}

void KTextBox::fitText()
{
  int startindex, endindex, countindex, testlength, line, column;
  
  // Map the text to the width of the widget
  cursorPosition( &line, &column );
  countindex =-1;
  QString testText = text().simplifyWhiteSpace() + " ";
  startindex = 0;
  testlength = testText.length();
  countindex = testText.find(" ", 0);
  while ((endindex = testText.find(" ", countindex+1)) > -1) {
    QString middle;
    int len;
    len = endindex - startindex;
    middle = testText.mid( startindex, len );

    if (textWidth( middle ) > width()) {
      testText.replace( countindex, 1, "\n" );
      startindex = countindex;
    }
    countindex = endindex;
  }

  setText( testText.stripWhiteSpace() );

  setCursorPosition( line, column );
}

/* 
 * EWHandleFrame Widget
 */
EWHandleFrame::EWHandleFrame(EventWidget *_pew, const char *name)
  : QFrame(_pew, name)
{
}

EWHandleFrame::~EWHandleFrame()
{
}

void EWHandleFrame::drawContents(QPainter *p)
{
  QFrame::drawContents(p);
  QRect cr(contentsRect());

  static const QPixmap pxmp = BarIcon("smallclock");
  p->drawPixmap(cr.left(), cr.top()+2, pxmp);
}

/*
 * EWIconBoxFrame Widget
 */
EWIconBoxFrame::EWIconBoxFrame(EventWidget *_pew, const char *name) 
  : QFrame(_pew, name)
{
  pew = _pew;
  updateIcons();
}
  
EWIconBoxFrame::~EWIconBoxFrame()
{
}

void EWIconBoxFrame::updateIcons()
{
  ni = 0;
  ni += ((showReadOnly = pew->getEvent()->isReadOnly()) ? 1 : 0);
  ni += ((showRecurs = pew->getEvent()->doesRecur()) ? 1 : 0);
  ni += ((showAlarm = pew->getEvent()->getAlarmRepeatCount()) ? 1 : 0);
}

void EWIconBoxFrame::drawContents(QPainter *p)
{
  static const QPixmap alarmPxmp = BarIcon("bell");
  static const QPixmap recurPxmp = BarIcon("recur");
  static const QPixmap readonlyPxmp = BarIcon("readonlyevent");
  QRect cr(contentsRect());
  register int xOff, yOff;

  updateIcons();

  xOff = cr.left();
  yOff = cr.top();

  if (showReadOnly) {
    p->drawPixmap(xOff, yOff, readonlyPxmp);
    xOff += readonlyPxmp.width() + PMMARGIN;
  }

  if (showRecurs) {
    p->drawPixmap(xOff, yOff, recurPxmp);
    xOff += recurPxmp.width() + PMMARGIN;
  }

  if (showAlarm) {
    p->drawPixmap(xOff, yOff, alarmPxmp);
    xOff += recurPxmp.width() + PMMARGIN;
  }
  
}

/* 
 * EventWidget -- main widget for displaying events
 */
EventWidget::EventWidget( KOEvent *_event, CalObject *_calendar,
			  QDate _date, QWidget *parent, const char *_name )
  : QFrame( parent, _name )
{
  widgetEvent = _event;
  widgetDate = _date;
  calendar = _calendar;
  ASSERT(widgetEvent);

  // set up widgets.
  useHandle = TRUE;
  handleFrame = new QFrame(this, "handleFrame");
  handleFrame->setFrameStyle(Panel | Sunken);
  handleFrame->setLineWidth(2);
  handleFrame->setFixedWidth(HANDLESIZE);
  handleFrame->setMouseTracking(TRUE);
  handleFrame->setFocusProxy(this);

  textBox = new KTextBox(this, "textBox");
  textBox->setFrameStyle(NoFrame);
  textBox->setMouseTracking(TRUE);

  useIcons = TRUE;
  iconBox = new EWIconBoxFrame(this, "iconBox");
  iconBox->setFrameStyle(NoFrame);
  iconBox->setMinimumWidth(2*PMMARGIN+MAXPMSIZE);
  iconBox->setMouseTracking(TRUE);
  iconBox->setFocusProxy(this);

  // we filter ALL events.
  setFrameStyle(Box | Plain);
  setLineWidth(1);
  setFocusPolicy(ClickFocus);
  setMouseTracking(TRUE);
  installEventFilter(this);
  handleFrame->installEventFilter(this);
  textBox->installEventFilter(this);
  iconBox->installEventFilter(this);

  // set flags
  setState(NoState);
  // what type of resizing is going on
  ResizeStatus = 0;
  
  widgetSelected = FALSE;

  // RMB popup menu
  rmbPopup = new QPopupMenu();
  rmbPopup->insertItem(i18n("&Edit"));
  QPixmap pm = BarIcon("delete");
  rmbPopup->insertItem(pm, i18n("&Delete"));
  pm = BarIcon("bell");
  rmbPopup->insertItem(pm, i18n("Toggle Alarm"));
  connect(rmbPopup, SIGNAL(activated(int)), 
	  SLOT(popupSlot(int)));

  updateConfig();

  setSelected(FALSE);

  // set text
  if (!widgetEvent->getSummary().isEmpty())
    setText(widgetEvent->getSummary());

  if (widgetEvent->isReadOnly()) {
    setEnabled(FALSE);
    iconBox->setEnabled(FALSE);
    textBox->setEnabled(FALSE);
    setBackgroundColor(palette().disabled().base());
  }
}

EventWidget::~EventWidget()
{
  //  debug("eventWidget %s deleted",name());
}

bool EventWidget::eventFilter( QObject *ob, QEvent *ev )
{
  QPoint mousePos, pmousePos;
  QWidget *wob = (QWidget *) ob;
  int xPos, yPos, buttonState;
  unsigned int kCode;
  VCalDrag *vcd;

  switch(ev->type()) {
  case QEvent::MouseButtonPress:
    mousePos = ((QMouseEvent *) ev)->pos();
    if (ob != this)
      mousePos = wob->mapToParent(mousePos);

    xPos = DragCenterX = mousePos.x();
    yPos = DragCenterY = mousePos.y();
    buttonState = ((QMouseEvent *) ev)->button();

    if ((State == Edit) && 
	(buttonState != RightButton) &&
	(wob == textBox)) {
      // clicked inside the text box, and we are in edit mode.
      // let it through.
      return FALSE;
    }

    if (!widgetSelected)
      emit selected(this);
    setState(NoState);
    // if it was the left or middle button, we are possibly in resize mode.
    // otherwise, we are not; we are in 'right mouse button menu mode.'
    // or regular select mode
    if ((buttonState == LeftButton) || (buttonState == MidButton)) {
      if( xPos < DRAGBORDER )
	ResizeStatus = RESIZELEFT;
      else if( xPos > width()- DRAGBORDER )
	ResizeStatus = RESIZERIGHT;
      else if( yPos < DRAGBORDER )
	ResizeStatus = RESIZETOP;
      else if( yPos > (height() - DRAGBORDER) )
	ResizeStatus = RESIZEBOTTOM;
      else
	// mouse inside valid boundaries, don't resize
	ResizeStatus = 0;
      
      if (ResizeStatus)
	setState(Resize);
      else if (wob == textBox) {
	// click was inside the text editor.  Switch to edit mode
	// and let event through
	setState(Edit);
	return FALSE;
      }
      return TRUE;
    } else { // it was a right mouse button click
      // right mouse button has no associated state
      rmbPopup->popup(QCursor::pos());
      return TRUE;
    }
    debug("error! should never reach here in EventWidget::eventFilter()\n");
    return FALSE; // should never get here, but...
    break;

  case QEvent::MouseButtonDblClick:
    setState(NoState);
    if (!widgetSelected)
      emit selected(this);
    emit editEventSignal( widgetEvent );
    return TRUE;
    break;

  case QEvent::MouseButtonRelease:
    if ((State == Drag) || (State == Resize)) {
      setState(NoState);
      setCursor(ArrowCursor);
    }
    return FALSE;
    break;
      
  case QEvent::MouseMove: 
    mousePos = ((QMouseEvent *) ev)->pos();
    if (ob != this) 
      mousePos = wob->mapToParent(mousePos);
    pmousePos = mapToParent(mousePos);
    xPos =  mousePos.x();
    yPos =  mousePos.y();
    DragX = pmousePos.x();
    DragY = pmousePos.y() - DragCenterY;
    ResizeY = pmousePos.y();

    buttonState = ((QMouseEvent *) ev)->state();

    // test if we want to do DND, not regular dragging
    if (State == Drag && ob == handleFrame) {
      vcd = calendar->createDrag(widgetEvent, handleFrame);
      // we really should be doing event deletion here if the move
      // is successful, but dragMove() always appears to be returning
      // false at the moment.  So it is up to the dropEvent handlers.  BAD.
      setState(NoState);
      vcd->dragMove();
      emit eventChanged(widgetEvent, EVENTDELETED);
      return TRUE;
    }

    // make sure cursor is the proper one.
    // ONLY need to set the cursor if we aren't already in resize mode.
    if (State != Resize && State != Drag && !buttonState) {
      if (xPos < DRAGBORDER)
	setCursor(SizeHorCursor);
      else if (xPos > width()- DRAGBORDER)
	setCursor(SizeHorCursor);
      else if (yPos < DRAGBORDER)
	setCursor(SizeVerCursor);
      else if (yPos > height() - DRAGBORDER)
	setCursor(SizeVerCursor);
      else
	setCursor(ArrowCursor);
    }

    switch (State) {
      //    case Edit:
      //      return FALSE;
      //      break;
    case Resize:
      // if we need to tell that we are resizing, send signal. 
      if (ResizeStatus != 0) 
	/*	if (wob != this)
	  emit widgetState((EventWidget *) wob->parentWidget(), ResizeStatus);
	  else*/
	  emit widgetState(this, ResizeStatus);
      return TRUE;
      break;
    case Drag:
      // we may only want to do this if we have moved past a certain
      // threshold, or maybe only on mouse button release.  Think about it.
      /*      if (wob != this)
	emit widgetState((EventWidget *) wob->parentWidget(), MOVE);
	else*/
	emit widgetState(this, MOVE);
      return TRUE;
      break;
    default:
      // see if we need to turn on dragging mode.
      if (((buttonState & LeftButton) || (buttonState & MidButton)) &&
	  ((abs(xPos - DragCenterX) > DRAGTHRESHOLD) ||
	  (abs(yPos - DragCenterY) > DRAGTHRESHOLD)) &&
	  (widgetEvent->doesRecur() == FALSE) &&
	  (widgetEvent->isMultiDay() == FALSE)) {
	setState(Drag);
	if (wob == handleFrame) // get outta here now if we are setting up DND
	  return TRUE;
	setCursor(SizeAllCursor);
	textBox->setCursor(SizeAllCursor);
	/*	if (wob != this)
	  emit widgetState((EventWidget *) wob->parentWidget(), MOVE);
	  else*/
	  emit widgetState(this, MOVE);
	return TRUE;
      }
    }
    return FALSE;
    break;
    
  case QEvent::FocusIn:
    if (!widgetSelected) {
      emit selected(this);
    }
    return FALSE;
    break;

  case QEvent::FocusOut:
    if (State == Edit)
      endEdit();
    return FALSE;
    break;

  case QEvent::KeyPress:
    kCode = ((QKeyEvent*)ev)->key();
    switch(kCode) {
    case Key_Return:
    case Key_Enter:
      if (State == Edit)
		endEdit();
      else 
		startEdit();
      return TRUE;
      break;
    case Key_Delete:
      emit deleteEventSignal( widgetEvent );
      return TRUE;
      break;
    case Key_Plus:
      widgetEvent->setDtEnd( widgetEvent->getDtEnd().addSecs( 5*60 ) );
      emit widgetState( this, DTCHANGE );
      return TRUE;
      break;
    case Key_Minus:
      widgetEvent->setDtEnd( widgetEvent->getDtEnd().addSecs( -5*60 ) );
      emit widgetState( this, DTCHANGE );
      return TRUE;
      break;

	case Key_Up:
	case Key_Down:
	case Key_PageUp:
	case Key_PageDown:
		;
    }

    ((QKeyEvent *) ev)->ignore();
    return FALSE;
    break;
    
  default:
    return FALSE;
  }
}

void EventWidget::resizeEvent( QResizeEvent *re )
{
  // outta here if no changes necessary
  if (re && (re->size() == re->oldSize()))
    return;

  register int xOff, yOff;
  register int w, h;
  QRect cr(contentsRect());
  QRect fr(frameRect());

  xOff = fr.left(); yOff = fr.top();
  if (useHandle) {
    handleFrame->setGeometry(xOff, yOff, 
			     HANDLESIZE, fr.height());
    xOff += HANDLESIZE;
  } else {
    handleFrame->hide();
  }

  yOff = cr.top();
  w = fr.width() - (HANDLESIZE + frameWidth());
  // we need to do this initial resize to the new width of the window so
  // we see how many lines of text it will take up.  Then we can adjust the
  // height based on this.
  textBox->resize(w, textBox->height());
  textBox->fitText();

  h = textBox->lines() * textBox->fontMetrics().height();
  textBox->setGeometry(xOff, yOff, w, 
		       h <= cr.height() ? h : cr.height());
  yOff += h <= cr.height() ? h : cr.height();

  h = textBox->height() + MAXPMSIZE;
  if (useIcons) {
    iconBox->setGeometry(xOff, yOff, 
			 w, h <= cr.height() ? 
			 MAXPMSIZE : cr.height() - textBox->height());
  } else {
    iconBox->hide();
  }
}

void EventWidget::paletteChange(QPalette)
{
	debug("paletteChange!\n");
	updateConfig();
}

void EventWidget::fontChange( QFont)
{
	updateConfig();
}


void EventWidget::startEdit()
{
	State = Edit; textBox->home(); textBox->setFocus();
}

void EventWidget::endEdit()
{
  State = NoState; 
  // try to fix the stupid bug that is killing our summaries.  Something
  // to do with focus.  This is a HACK and will not allow you to set an
  // empty summary. FIX ME
  if (textBox->text().simplifyWhiteSpace().isEmpty()) {
    textBox->setText(widgetEvent->getSummary().data());
  } else {
    widgetEvent->setSummary(textBox->text().simplifyWhiteSpace());
  }
  setFocus();
  setToolTip(textBox->text());
};


void EventWidget::setSelected(bool _sel) 
{
  if (widgetSelected = _sel) {
    handleFrame->setPalette(QPalette( selectedHandle ));
    handleFrame->setLineWidth(2);
    handleFrame->setFrameStyle(Panel | Raised);
  } else {
    handleFrame->setPalette(QPalette( inactiveHandle ));
    handleFrame->setLineWidth(1);
    handleFrame->setFrameStyle(Box | Plain);
  }
}

void EventWidget::setToolTip( QString tiptext )
{
  toolTipText = tiptext.simplifyWhiteSpace();
  if (widgetEvent->isReadOnly())
    toolTipText += " (Read Only)";
  
  QToolTip::add( this, tiptext );
}

void EventWidget::updateConfig()
{
  	KConfig *config;
  
	config = kapp->config();

	config->setGroup("Fonts");
	setFont(config->readFontEntry("Schedule Font"));

	config->setGroup( "Colors" );

	selectedHandle = config->readColorEntry( "AptSelected" );
	inactiveHandle = config->readColorEntry( "AptUnselected" );
	activeHandle = config->readColorEntry( "AptActive" ); 

	QColorGroup normalGroup(
		black, //kapp->windowColor, // foreground, used for the edge
		//kapp->windowColor,
		config->readColorEntry( "AptBackground" ), 
			// background, used for non text area in textbox
		green, //config->readColorEntry( "AptUnselected" ),// light
		red, // dark
		red, // mid
		config->readColorEntry( "AptText" ), // text
		//kapp->windowColor
		config->readColorEntry( "AptBackground" )
			// base used for background behind text
	);

	// for displaying currently happening event ...
	QColorGroup activeGroup(
		selectedHandle, // for the edge
		config->readColorEntry( "AptBackground" ), 
		green, // light
		red, // dark
		red, // mid
		config->readColorEntry( "AptText" ), // text
		config->readColorEntry( "AptBackground" )
	);

	QColorGroup disabledGroup(
		black, //kapp->windowColor, // foreground, used for the edge
		//kapp->windowColor,
		config->readColorEntry( "AptBackground" ).dark(120), 
			// background, used for non text area in textbox
		green, //config->readColorEntry( "AptUnselected" ),// light
		red, // dark
		red, // mid
		config->readColorEntry( "AptText" ), // text
		//kapp->windowColor
		config->readColorEntry( "AptBackground" ).dark(120)
			// base used for background behind text
	);

	QPalette myPalette;
	myPalette.setNormal( normalGroup );
	myPalette.setActive( activeGroup );
	myPalette.setDisabled(disabledGroup);

	setPalette(myPalette);

	// we don't want the hilite border for other widgets except
	// the main widget, so set the active color group to normalGroup.
	myPalette.setActive(normalGroup);
	textBox->setPalette( myPalette );
	iconBox->setPalette( myPalette );

	setSelected( widgetSelected ); // to set handle color
}

	
void EventWidget::popupSlot(int index)
{
  if (!widgetSelected) {
    debug("widget not selected, ERROR!");
    qApp->beep();
    return;
  }
  switch (index) {
  case 0: // EDIT
    emit editEventSignal(widgetEvent);
    break;
  case 1: // DELETE
    emit deleteEventSignal(widgetEvent);
    break;
  case 2: { // toggle alarm
    widgetEvent->toggleAlarm();
    iconBox->update();
    break;
  }
  default:
    break;
  }
}

