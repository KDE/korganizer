#ifndef _KOAPP_H
#define _KOAPP_H

#include <kuniqueapp.h>

class CalObject;

class KOrganizerApp : public KUniqueApplication
{
  Q_OBJECT
public:
  KOrganizerApp();
  KOrganizerApp(int &argc, char **argv, const QCString &rAppName);
  ~KOrganizerApp();

//  int newInstance(QValueList<QCString> params);
  int newInstance();

private:
  void displayImminent(int numdays);
//  void help(); // depricated (KCmdLineArgs)
  void startAlarmDaemon();

  CalObject *cal;
  QCString fn;
};

#endif
