// eventwidget.h
// (c) F.Zigterman, Preston Brown 1998

#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <qlabel.h>
#include <qstring.h>
#include <qmultilinedit.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <qpixmap.h>

#include "koevent.h"

class CalObject;
class EventWidget;
class QPopupMenu;

/** This widget offers an edit box without scrollbars */
class KTextBox : public QMultiLineEdit
{
  Q_OBJECT
public:
  KTextBox(QWidget *parent = 0, char *name = 0);
  void home(bool mark=FALSE) { QMultiLineEdit::home(mark); };
  int lines() const 
    { return text().contains("\n") + 1; };
  void fitText();
 protected:
};

class EWHandleFrame : public QFrame
{
 public:
  EWHandleFrame(EventWidget *, const char *name = 0);
  ~EWHandleFrame();
 protected:
  virtual void drawContents(QPainter *);
};

class EWIconBoxFrame : public QFrame
{
  Q_OBJECT
public:
  EWIconBoxFrame(EventWidget *, const char *name = 0);
  ~EWIconBoxFrame();
  int numIcons() const { return ni; };

public slots:
  void updateIcons();

protected:
  void drawContents(QPainter *);
  int ni;

  bool showAlarm;
  bool showRecurs;
  bool showReadOnly;
  EventWidget *pew;
};

/** this widget displays the time and summary of events */
class EventWidget : public QFrame
{
  Q_OBJECT
public:
  /** Constructor	*/
  EventWidget( KOEvent *, CalObject *, QDate, 
	       QWidget *parent = 0, const char *name = 0 );
  
  /** Destructor */
  virtual ~EventWidget();
  
  /** Return date for event display */
  QDate date() const { return widgetDate; }
  void setDate(const QDate d) { widgetDate = d; }

  /* Set the text in the body (textbox) */
  void setText(const QString &_text)  { textBox->setText(_text); };
  QString text() const { return textBox->text(); };
  
  /** Set font (textbox)*/
  void setFont(const QFont &_font) { textBox->setFont(_font); };
  QFont font() const { return textBox->font(); };
  
  /** Set the tooltip for the entire widget */
  void setToolTip( QString );
  QString toolTip() { return toolTipText; };
  
  /** return event */
  KOEvent *getEvent() const { return widgetEvent; };
  
  /** return if widget is selected */
  bool selected();
  
  /** If the widget is dragged, contains the new desired X position */
  int dragPosX() { return DragX; };
  
  /** If the widget is dragged, contains the new desired Y position */
  int dragPosY() { return DragY; };
  
  /** if the widget is resized, contains the new desired Y position */
  int resizePosY() { return ResizeY; };
  
  enum { MOVE, RESIZETOP, RESIZEBOTTOM,
	 RESIZELEFT, RESIZERIGHT,
	 DTCHANGE, CONTENTS };
  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };
  
public slots:
  virtual void setEnabled(bool _en) { textBox->setEnabled(_en); };
  /** select this widget if TRUE unselect if FALSE */
 void setSelected(bool sel);
 /** select the textbox and start editing this event's summary */
 void endEdit();
 void startEdit();
 void updateConfig();
 void showHandle() { useHandle = TRUE; handleFrame->show(); };
 void hideHandle() { useHandle = FALSE; handleFrame->hide(); };
 void showIcons() { useIcons = TRUE; iconBox->show(); };
 void hideIcons() { useIcons = FALSE; iconBox->hide(); };
signals:
 void selected(EventWidget *);
 void editEventSignal( KOEvent *);
 void deleteEventSignal( KOEvent *);
 void widgetState(EventWidget *, int state);
 void eventChanged(KOEvent *, int action);
 
protected slots:	
  void popupSlot(int);

protected:
  CalObject *calendar;
  KOEvent *widgetEvent;
  QDate widgetDate;

  QList<EventWidget> associates;
  
  // sub widgets
  QFrame *handleFrame; // EWHandleFrame not working perfectly yet
  EWIconBoxFrame *iconBox;
  KTextBox *textBox;

  // event filters/handlers
  void setState(int _state)
    { 
      if (State == Edit && _state != Edit)
	endEdit();
      if (_state == Edit)
	textBox->setCursor(ibeamCursor);
      State = _state; 
    };
  bool eventFilter(QObject *, QEvent *);
  virtual void resizeEvent( QResizeEvent * );	
  virtual void frameChanged() { resizeEvent((QResizeEvent *) 0L); };

	void fontChange( QFont );
	void paletteChange( QPalette );
	  
  // misc
  QPopupMenu *rmbPopup;
  bool widgetSelected;
  bool useHandle;
  bool useIcons;
  enum { Edit = 1, Drag = 2, Resize = 4, NoState = 0 };
  int State; // State of Widget: drag, edit, etc.
  int Threshold;	// counting value for drag enable
  int DragX, DragY, ResizeY; 	// new desired coords.
  int DragCenterX, DragCenterY; 	// drag center point
  int dragging_enabled;
  int moving_enabled;
  int ResizeStatus;

  QColor inactiveHandle, activeHandle, selectedHandle;

  QString toolTipText;

};

#endif
