#ifndef _EVENTWIN_H
#define _EVENTWIN_H

// 	$Id$	
//#include <ktabctl.h>
//#include <ktmainwindow.h>
#include <ktmainwindow.h>
#include <klined.h>

#include <qmlined.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qdatetm.h>
#include <qtabwidget.h>

#include "calobject.h"
#include "wingeneral.h"
#include "eventwindetails.h"
#include "eventwinrecur.h"
#include "catdlg.h"
#include "kptbutton.h"

/**
  * This is the class to add/edit a new appointment.
  *
  * @short Creates a dialog box to create/edit an appointment
  * @author Preston Brown
  * @version $Revision$
  */
class EventWin : public KTMainWindow
{
  Q_OBJECT

public:
/**
  * Constructs a new appointment dialog.
  *
  */  
  EventWin( CalObject *calendar);
  virtual ~EventWin(void);

  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };
  enum { TYPE_UNKNOWN, TYPE_TODO, TYPE_APPOINTMENT }; // what am I?

public slots:

/** Clear eventwin for new event, and preset the dates and times with hint */
  virtual void newEvent( QDateTime from, QDateTime to, bool allDay = FALSE );

/** Edit an existing event. */
  virtual void editEvent( KOEvent *, QDate qd=QDate::currentDate()) = 0;

/** cancel is public so TopWidget can call it to close the dialog. */
  void cancel();

/** Set the modified flag */
  void setModified();

signals:
  void eventChanged(KOEvent *, int);
  void closed(QWidget *);

protected slots:
  virtual KOEvent* makeEvent();
  void updateCatEdit(QString _str);
//  void recurStuffEnable(bool enable);
  virtual void prevEvent()=0;
  virtual void nextEvent()=0;
  void saveAndClose();
  virtual void deleteEvent()=0;

protected:
  // methods
  void initConstructor(); // this is NOT virtual
//  virtual void initMenus();
  virtual void initToolBar();
  virtual void fillInFields(QDate qd);
  virtual void fillInDefaults(QDateTime from, QDateTime to, bool allDay);
  // KTMainWindow implements closeEvent and provides queryClose.
  virtual bool queryClose();
//  virtual void closeEvent(QCloseEvent *);
  QDate *dateFromText(const char *text);
  virtual void setDuration() = 0;
  bool getModified();
  // setModified() is a slot
  void unsetModified();
  virtual void setTitle();

  // variables
protected:
  KPTButton *saveExit;
  KToolBarButton *btnDelete;
  WinGeneral *General;
  EventWinDetails *Details;
  EventWinRecurrence *Recurrence;
  QFrame  *m_frame;
  QTabWidget *tabWidget;
//  KTabCtl *tabCtl;
  KToolBar *toolBar;
  CategoryDialog *catDlg;

  CalObject *calendar;
  KOEvent *currEvent;               // the event being edited, if any.
  bool addFlag;                      // should we add a new event, or modify?

  bool prevNextUsed;

  int Type; // what am I?
  bool Modified;
};

#endif


