// Alarm Daemon for KOrganizer
// (c) 1998, 1999 Preston Brown
// $Id$	

#include <unistd.h>

#include <qtimer.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qtooltip.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include "config.h"
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "alarmdialog.h"
#include "calendarlocal.h"

#include "alarmdaemon.h"
#include "alarmdaemon.moc"

AlarmDockWindow::AlarmDockWindow(QWidget *parent, const char *name)
  : KSystemTray(parent, name)
{
  KGlobal::iconLoader()->addAppDir("korganizer");
  dPixmap1 = BarIcon("alarmd");
  dPixmap2 = BarIcon("alarmd_disabled");

  if (dPixmap1.isNull() || dPixmap2.isNull()) {
    KMessageBox::sorry(this, i18n("Can't load docking tray icon!"),
                             i18n("Alarm Monitor Error"));
  }
  setPixmap(dPixmap1);

  itemId = contextMenu()->insertItem(i18n("Alarms Enabled"),
				   this, SLOT(toggleAlarmsEnabled()));
  contextMenu()->setItemChecked(itemId, TRUE);

  QToolTip::add(this, i18n("Appointment Alarm Monitor"));
}

AlarmDockWindow::~AlarmDockWindow()
{
}

void AlarmDockWindow::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == LeftButton) {
    // start up a korganizer.
    KProcess proc;
    proc << "korganizer";
    proc.start(KProcess::DontCare);
  } else
    KSystemTray::mousePressEvent(e);
}

void AlarmDockWindow::closeEvent(QCloseEvent *e)
{
  kapp->quit();
}

///////////////////////////////////////////////////////////////////////////////

AlarmDaemon::AlarmDaemon(const QString &fn, QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(this)
{
  kdDebug() << "AlarmDaemon::AlarmDaemon()" << endl;

  docker = new AlarmDockWindow;
  docker->show();

  calendar = new CalendarLocal;
  calendar->showDialogs(FALSE);

  mAlarmDialog = new AlarmDialog;
  connect(mAlarmDialog,SIGNAL(suspendSignal(int)),SLOT(suspend(int)));

  calendar->load(fn);

  // set up the alarm timer
  QTimer *alarmTimer = new QTimer(this);
  mSuspendTimer = new QTimer(this);

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

  mSuspendTimer->stop();
  mAlarmDialog->clearEvents();

  calendar->close();
  config.setGroup("General");
  QString fileName = config.readEntry("Active Calendar");

  kdDebug() << "AlarmDaemon::reloadCal(): '" << fileName << "'" << endl;

  calendar->load(fileName);
}

void AlarmDaemon::showAlarms(QList<KOEvent> &alarmEvents)
{
  // leave immediately if alarms are off
  if (!docker->alarmsOn()) return;

  KOEvent *anEvent;

  for (anEvent = alarmEvents.first(); anEvent; anEvent = alarmEvents.next()) {
    mAlarmDialog->appendEvent(anEvent);
  }

  showDialog();
}

bool AlarmDaemon::process(const QCString &fun, const QByteArray &,
			  QCString &replyType, QByteArray &)
{
  if (fun == "reloadCal()") {
    reloadCal();
    replyType = "void";
    return true;
  } else {
    kdDebug() << "AlarmDaemon::process got unknown DCOP message." << endl;
    return false;
  }
}

void AlarmDaemon::suspend(int duration)
{
//  kdDebug() << "AlarmDaemon::suspend() " << duration << " minutes" << endl;

  connect(mSuspendTimer,SIGNAL(timeout()),SLOT(showDialog()));

  mSuspendTimer->start(1000*60*duration,true);
}

void AlarmDaemon::showDialog()
{
  KNotifyClient::beep();
  mAlarmDialog->show();
  mAlarmDialog->eventNotification();
}
