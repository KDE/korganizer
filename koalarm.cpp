// $Id$	

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "incidence.h"
#include "koprefs.h"

#include "koalarm.h"

KOAlarm::KOAlarm(Incidence *parent)
{
  mParent = parent;

  mAlarmReadOnly = false;

  mAudioAlarmFile = "";
  mProgramAlarmFile = "";
  mMailAlarmAddress = "";
  mAlarmText = "";

  mAlarmSnoozeTime = 5;
  mAlarmRepeatCount = 0; // alarm disabled
}

KOAlarm::~KOAlarm()
{
}

void KOAlarm::setAudioAlarmFile(const QString &audioAlarmFile)
{
  if (mAlarmReadOnly) return;
  mAudioAlarmFile = audioAlarmFile;
  mParent->emitEventUpdated(mParent);
}

void KOAlarm::setAudioAlarmFile(const char *audioAlarmFile)
{
  if (mAlarmReadOnly) return;
  mAudioAlarmFile = audioAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::audioAlarmFile() const
{
  return mAudioAlarmFile;
}

void KOAlarm::setProgramAlarmFile(const QString &programAlarmFile)
{
  if (mAlarmReadOnly) return;
  mProgramAlarmFile = programAlarmFile;
  mParent->emitEventUpdated(mParent);
}

void KOAlarm::setProgramAlarmFile(const char *programAlarmFile)
{
  if (mAlarmReadOnly) return;
  mProgramAlarmFile = programAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::programAlarmFile() const
{
  return mProgramAlarmFile;
}

void KOAlarm::setMailAlarmAddress(const QString &mailAlarmAddress)
{
  if (mAlarmReadOnly) return;
  mMailAlarmAddress = mailAlarmAddress;
  mParent->emitEventUpdated(mParent);
}

void KOAlarm::setMailAlarmAddress(const char *mailAlarmAddress)
{
  if (mAlarmReadOnly) return;
  mMailAlarmAddress = mailAlarmAddress;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::mailAlarmAddress() const
{
  return mMailAlarmAddress;
}

void KOAlarm::setAlarmText(const QString &alarmText)
{
  if (mAlarmReadOnly) return;
  mAlarmText = alarmText;
  mParent->emitEventUpdated(mParent);
}

void KOAlarm::setAlarmText(const char *alarmText)
{
  if (mAlarmReadOnly) return;
  mAlarmText = alarmText;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::alarmText() const
{
  return mAlarmText;
}

void KOAlarm::setAlarmTime(const QDateTime &alarmTime)
{
  if (mAlarmReadOnly) return;
  mAlarmTime = alarmTime;
  mParent->emitEventUpdated(mParent);
}

/*
void KOAlarm::setAlarmTime(const QString &alarmTimeStr)
{
  if (mAlarmReadOnly) return;
  mAlarmTime = strToDateTime(alarmTimeStr);
  mParent->emitEventUpdated(mParent);
}
*/

const QDateTime &KOAlarm::alarmTime() const
{
  return mAlarmTime;
}

void KOAlarm::setAlarmSnoozeTime(int alarmSnoozeTime)
{
  if (mAlarmReadOnly) return;
  mAlarmSnoozeTime = alarmSnoozeTime;
  mParent->emitEventUpdated(mParent);
}

int KOAlarm::alarmSnoozeTime() const
{
  return mAlarmSnoozeTime;
}

void KOAlarm::setAlarmRepeatCount(int alarmRepeatCount)
{
  if (mAlarmReadOnly) return;
  mAlarmRepeatCount = alarmRepeatCount;
  mParent->emitEventUpdated(mParent);
}

int KOAlarm::alarmRepeatCount() const
{
  return mAlarmRepeatCount;
}

void KOAlarm::toggleAlarm()
{
  if (mAlarmReadOnly) return;
  if (mAlarmRepeatCount) {
    mAlarmRepeatCount = 0;
  } else {
    mAlarmRepeatCount = 1;
    QString alarmStr(QString::number(KOPrefs::instance()->mAlarmTime));
    int pos = alarmStr.find(' ');
    if (pos >= 0)
      alarmStr.truncate(pos);
    mAlarmTime = mAlarmStart.addSecs(-60 * alarmStr.toUInt());
  }
  mParent->emitEventUpdated(mParent);
}
