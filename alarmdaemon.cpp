// Alarm Daemon for KOrganizer
// (c) 1998, 1999 Preston Brown
// $Id$	

#include <unistd.h>

#include <qtimer.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qmsgbox.h>
#include <qtooltip.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kwm.h>
#include <kprocess.h>
#include <kaudioplayer.h>

#include "config.h"
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "alarmdaemon.h"
#include "alarmdaemon.moc"

#ifndef HAVE_BASENAME
#include "basename.c"
#endif

AlarmDockWindow::AlarmDockWindow(QWidget *parent, const char *name)
  : KDockWindow(parent, name)
{
  dPixmap1 = KGlobal::iconLoader()->loadIcon("alarmd", 
					     KIcon::User);
  dPixmap2 = KGlobal::iconLoader()->loadIcon("alarmd-disabled", 
					     KIcon::User);

  if (dPixmap1.isNull() || dPixmap2.isNull()) {
    QMessageBox::warning(this, i18n("Alarm Monitor Error"),
			 i18n("Can't load docking tray icon!"));
  }
  setPixmap(dPixmap1);

  itemId = contextMenu()->insertItem(i18n("Alarms Enabled"),
				   this, SLOT(toggleAlarmsEnabled()));
  contextMenu()->setItemChecked(itemId, TRUE);
  contextMenu()->insertItem(i18n("Close Alarm Monitor"), kapp,
			    SLOT(quit()));

  QToolTip::add(this, i18n("Appointment Alarm Monitor"));
}

AlarmDockWindow::~AlarmDockWindow()
{
}

/*void DockWidget::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == RightButton) {
    QPoint pt = this->mapToGlobal(QPoint(0, 0));
    pt = pt - QPoint(30, 30);
    popupMenu->popup(pt);
    popupMenu->exec();
  }
  if (e->button() == LeftButton) {
    // start up a korganizer.
    KProcess proc;
    proc << "korganizer";
    proc.start(KProcess::DontCare);
  }
  }*/

///////////////////////////////////////////////////////////////////////////////

AlarmDaemon::AlarmDaemon(const char *fn, QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(this)
{
  docker = new AlarmDockWindow;
  calendar = new CalObject;
  calendar->showDialogs(FALSE);
  fileName = fn;

  calendar->load(fileName);

  // set up the alarm timer
  QTimer *alarmTimer = new QTimer(this);

  connect(alarmTimer, SIGNAL(timeout()),
    calendar, SLOT(checkAlarms()));
  connect(calendar, SIGNAL(alarmSignal(QList<KOEvent> &)),
    this, SLOT(showAlarms(QList<KOEvent> &)));

  // timeout every minute.
  alarmTimer->start(1000*60);
}

AlarmDaemon::~AlarmDaemon()
{
  delete calendar;
  delete docker;
}

void AlarmDaemon::reloadCal()
{
  KSimpleConfig config("korganizerrc", true);

  calendar->close();
  config.setGroup("General");
  newFileName = config.readEntry("Current Calendar");
  calendar->load(newFileName.data());
}

void AlarmDaemon::showAlarms(QList<KOEvent> &alarmEvents)
{
  KOEvent *anEvent;
  QString messageStr, titleStr;
  QDateTime tmpDT;

  // leave immediately if alarms are off
  if (!docker->alarmsOn())
    return;

  tmpDT = alarmEvents.first()->getAlarmTime();

  titleStr.sprintf(i18n("Alarm Monitor: %s\n"),tmpDT.toString().data());
  messageStr += i18n("The following events triggered alarms:\n\n");

  for (anEvent = alarmEvents.first(); anEvent;
      anEvent = alarmEvents.next()) {
    messageStr += anEvent->getSummary() + "\n";
    if (!anEvent->getProgramAlarmFile().isEmpty()) {
      KProcess proc;
      proc << anEvent->getProgramAlarmFile().data();
      proc.start(KProcess::DontCare);
    }

    if (!anEvent->getAudioAlarmFile().isEmpty()) {
      KAudioPlayer::play(anEvent->getAudioAlarmFile().data());
    }
  }

  kapp->beep();
  QMessageBox::information(0, titleStr.data(),
        messageStr.data(),
        QMessageBox::Ok | QMessageBox::Default);
}

bool AlarmDaemon::process(const QCString &fun, const QByteArray &data,
			  QCString &replyType, QByteArray &replyData)
{
  if (fun == "reloadCal()") {
    reloadCal();
    replyType = "void";
    return true;
  } else {
    qDebug("AlarmDaemon::process got unknown DCOP message.");
    return false;
  }
}
