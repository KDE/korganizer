#ifndef _ALARMAPP_H
#define _ALARMAPP_H

#include <kuniqueapp.h>

class AlarmApp : public KUniqueApplication
{
  Q_OBJECT
public:
  AlarmApp(int &argc, char **argv, const QCString &rAppName)
    : KUniqueApplication(argc, argv, rAppName) {};
  virtual ~AlarmApp() {};

  int newInstance(QValueList<QCString> params);
};

#endif
