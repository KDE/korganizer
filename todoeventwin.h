#ifndef _APPTDLG_H
#define _APPTDLG_H

// 	$Id$	
#include <ktabctl.h>
#include <ktmainwindow.h>
#include <klined.h>

#include <qmlined.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qdatetm.h>

#include "calobject.h"
#include "eventwingeneral.h"
#include "eventwindetails.h"
#include "eventwinrecur.h"
#include "catdlg.h"
#include "kptbutton.h"
#include "eventwin.h"

/**
  * This is the class to add/edit a new appointment.
  *
  * @short Creates a dialog box to create/edit an appointment
  * @author Preston Brown
  * @version $Revision$
  */
class TodoEventWin : public EventWin
{
  Q_OBJECT

public:
/**
  * Constructs a new appointment dialog.
  *
  */  
  TodoEventWin( CalObject *calendar);
  virtual ~TodoEventWin(void);

//  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

/** Clear eventwin for new event, and preset the dates and times with hint */
  void newEvent( QDateTime due, KOEvent *relatedEvent = 0,
                 bool allDay = false );

/** Edit an existing event. */
  void editEvent( KOEvent *, QDate qd=QDate::currentDate());

public slots:

signals:
  void eventChanged(KOEvent *, int);
  void closed(QWidget *);

protected slots:
  KOEvent* makeEvent();
  void updateCatEdit(QString _str);
  void prevEvent();
  void nextEvent();
  void deleteEvent();

protected:
  // methods
  void initConstructor();
  void fillInFields();
  void fillInDefaults(QDateTime due, KOEvent *relatedEvent, bool allDay);
  bool queryClose();
//  void closeEvent(QCloseEvent *);
  void setDuration();

private:
  KOEvent *mRelatedEvent;
};

#endif
