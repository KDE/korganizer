/* 	$Id$	 */

#ifndef _ALARMDAEMON_H
#define _ALARMDAEMON_H

#include <qlist.h>
#include <kdockwindow.h>
#include <kpopmenu.h>
#include <dcopobject.h>

#include "calobject.h"
#include "koevent.h"

class AlarmDockWindow : public KDockWindow
{
  Q_OBJECT

public:
  AlarmDockWindow(QWidget *parent = 0, const char *name = 0);
  virtual ~AlarmDockWindow();

  bool alarmsOn() { return contextMenu()->isItemChecked(itemId); }
  //  void mousePressEvent(QMouseEvent *);
  //  void mouseReleaseEvent(QMouseEvent *);

public slots:
  void toggleAlarmsEnabled() 
  {
    contextMenu()->setItemChecked(itemId,
				  !contextMenu()->isItemChecked(itemId));
    setPixmap(contextMenu()->isItemChecked(itemId) ? dPixmap1 : dPixmap2);
  }
 
 protected:
  QPixmap dPixmap1, dPixmap2;
  int itemId;
};


class AlarmDaemon : public QObject, DCOPObject {
  Q_OBJECT

public:
  AlarmDaemon(const char *fn, QObject *parent = 0, const char *name = 0);
  virtual ~AlarmDaemon();

  bool process(const QCString &fun, const QByteArray &data,
	       QCString &replyType, QByteArray &replyData);

public slots:
  void showAlarms(QList<KOEvent> &alarmEvents);
  void reloadCal();

signals:
  void alarmSignal(QList<KOEvent> &);
  
private:
  AlarmDockWindow *docker;
  CalObject *calendar;
  const char *fileName;
  QString newFileName;
};

#endif
