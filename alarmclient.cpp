#include "alarmclient.h"

#include "kalarmd/alarmdaemoniface_stub.h"

#include <kstandarddirs.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qstring.h>
#include <qfile.h>


void AlarmClient::startAlarmDaemon()
{
  if( kapp->dcopClient()->isApplicationRegistered( "kalarmd" ) )
    // Alarm daemon already registered
    return;

  // Start alarmdaemon. It is a KUniqueApplication, that means it is
  // automatically made sure that there is only one instance of the alarm daemon
  // running.
  KProcess proc;
  proc << "kalarmd";
  proc.start(KProcess::Block, KProcess::NoCommunication);
}

void AlarmClient::startAlarmClient()
{
  if( kapp->dcopClient()->isApplicationRegistered( "korgac" ) )
    // Alarm daemon already registered
    return;

  KProcess *proc = new KProcess;
  *proc << "korgac";
  *proc << "--miniicon" <<  "korganizer";
  connect( proc, SIGNAL( processExited( KProcess* ) ),
           instance(), SLOT( startCompleted( KProcess* ) ) );
  if( !proc->start() )
    delete proc;

  // Register this application with the alarm daemon
  AlarmDaemonIface_stub stub( "kalarmd", "ad" );
  stub.registerApp( "korgac", "KOrganizer", "ac", 3, true );
  if( !stub.ok() )
    kdDebug(5850) << "AlarmClient::startAlarmClient(): dcop send failed" << endl;
}

void AlarmClient::startCompleted( KProcess* process )
{
  delete process;
}

AlarmClient* AlarmClient::instance()
{
  static AlarmClient *singleton = 0;

  if( singleton == 0 )
    singleton = new AlarmClient();

  return singleton;
}
