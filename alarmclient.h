#ifndef ALARMCLIENT_H
#define ALARMCLIENT_H

#include <qobject.h>

class KProcess;

/**
 * This class is used by KOrganizerApp and KOrganizerPart to initialize
 * KOrganizer alarmd usage. It's possible that more code could be moved
 * to this class later.
 *
 * Since this is code that should only run during initialization, it's a 
 * singleton class. Actually only static methods are public now, so the
 * instance() method is private too. Change this if necessary.
 *
 * When code is shared between init of the two, it should go into this class.
 *
 */
class AlarmClient : public QObject {
  Q_OBJECT

public:
  /** Start alarm daemon from KDE binary directory */
  static void startAlarmDaemon();

  /** Start alarm client from KDE binary directory */
  static void startAlarmClient();

private slots:
  void startCompleted( KProcess * );

private:
  static AlarmClient* instance();

  AlarmClient() : QObject() {}
  AlarmClient( const AlarmClient& );
  AlarmClient& operator=( const AlarmClient& );

  class gccShouldNotWarnAboutPrivateConstructors;
  friend class gccShouldNotWarnAboutPrivateConstructors;
};

#endif // ALARMCLIENT_H
