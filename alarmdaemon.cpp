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
#include <kio/netaccess.h>

//#include "config.h"
//#ifdef HAVE_LIBGEN_H
//#include <libgen.h>
//#endif

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

//  QToolTip::add(this, i18n("Appointment Alarm Monitor"));
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

void AlarmDockWindow::closeEvent(QCloseEvent *)
{
  kapp->quit();
}

void AlarmDockWindow::addToolTip(const QString &filename)
{
  QString txt = i18n("KOrganizer Alarm Monitor");

  if (!filename.isEmpty()) txt += "\n" + filename;

  QToolTip::add(this,txt);
}

///////////////////////////////////////////////////////////////////////////////

AlarmDaemon::AlarmDaemon(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(this)
{
  kdDebug() << "AlarmDaemon::AlarmDaemon()" << endl;

  mDocker = new AlarmDockWindow;
  mDocker->show();

  mCalendar = new CalendarLocal;
  mCalendar->showDialogs(FALSE);

  mAlarmDialog = new AlarmDialog;
  connect(mAlarmDialog,SIGNAL(suspendSignal(int)),SLOT(suspend(int)));

  // set up the alarm timer
  mAlarmTimer = new QTimer(this);
  mSuspendTimer = new QTimer(this);

  connect(mAlarmTimer,SIGNAL(timeout()),mCalendar,SLOT(checkAlarms()));
  connect(mCalendar, SIGNAL(alarmSignal(QList<Event> &)),
          SLOT(showAlarms(QList<Event> &)));
}

AlarmDaemon::~AlarmDaemon()
{
  delete mCalendar;
  delete mDocker;
}

void AlarmDaemon::reloadCal()
{
  KSimpleConfig config("korganizerrc", true);

  mSuspendTimer->stop();
  mAlarmDialog->clearEvents();

  mCalendar->close();
  config.setGroup("General");
  QString urlString = config.readEntry("Active Calendar");

  kdDebug() << "AlarmDaemon::reloadCal(): '" << urlString << "'" << endl;

  KURL url(urlString);

  if (!url.isEmpty()) {
    QString tmpFile;
    if(KIO::NetAccess::download(url,tmpFile)) {
      kdDebug() << "--- Downloaded to " << tmpFile << endl;
      bool success = mCalendar->load(tmpFile);
      KIO::NetAccess::removeTempFile(tmpFile);
      if (success) {
        mDocker->addToolTip(url.prettyURL());
      
        // timeout every minute.
        mAlarmTimer->start(1000*60);
        return;
      } else {
        kdDebug() << "Error loading calendar file '" << tmpFile << "'" << endl;
      }
    } else {
      QString msg;
      msg = i18n("Cannot download calendar from '%1'.").arg(url.prettyURL());
      KMessageBox::error(0,msg);
    }
  }

  mDocker->addToolTip(i18n("No calendar loaded."));
}

void AlarmDaemon::showAlarms(QList<Event> &alarmEvents)
{
  // leave immediately if alarms are off
  if (!mDocker->alarmsOn()) return;

  Event *anEvent;

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
