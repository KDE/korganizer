#ifndef _KOAPP_H
#define _KOAPP_H

#include <kuniqueapp.h>

class KOrganizerApp : public KUniqueApplication
{
  Q_OBJECT
public:
  KOrganizerApp();
  ~KOrganizerApp();

  /** Create new instance of KOrganizer. If there is already running a
    KOrganizer only an additional main window is opened. */
  int newInstance();

private:
  /** Print events for numDays days from calendar loaded from file to screen.*/
  void displayImminent(const QString & file,int numdays);
  /** Start alarm daemon from KDE binary directory */
  void startAlarmDaemon();
  /** process calendar from file. If numDays is 0, open a new KOrganizer window,
    if is is greater print events from corresponding number of dates to the
    screen. If active is true the file is the active calendar. */
  void processCalendar(const QString & file,int numDays,bool active=false);
};

#endif
