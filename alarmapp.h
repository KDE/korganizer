#ifndef _ALARMAPP_H
#define _ALARMAPP_H

#include <kuniqueapp.h>

class AlarmDaemon;

class AlarmApp : public KUniqueApplication
{
    Q_OBJECT
  public:
    AlarmApp();
    virtual ~AlarmApp();

    int newInstance();
    
  private:
    AlarmDaemon *mAd;
};

#endif
