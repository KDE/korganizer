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

void KOAlarm::setAudioFile(const QString &audioAlarmFile)
{
  if (mAlarmReadOnly) return;
  mAudioAlarmFile = audioAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::audioFile() const
{
  return mAudioAlarmFile;
}

void KOAlarm::setProgramFile(const QString &programAlarmFile)
{
  if (mAlarmReadOnly) return;
  mProgramAlarmFile = programAlarmFile;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::programFile() const
{
  return mProgramAlarmFile;
}

void KOAlarm::setMailAddress(const QString &mailAlarmAddress)
{
  if (mAlarmReadOnly) return;
  mMailAlarmAddress = mailAlarmAddress;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::mailAddress() const
{
  return mMailAlarmAddress;
}

void KOAlarm::setText(const QString &alarmText)
{
  if (mAlarmReadOnly) return;
  mAlarmText = alarmText;
  mParent->emitEventUpdated(mParent);
}

const QString &KOAlarm::text() const
{
  return mAlarmText;
}

void KOAlarm::setTime(const QDateTime &alarmTime)
{
  if (mAlarmReadOnly) return;
  mAlarmTime = alarmTime;
  mParent->emitEventUpdated(mParent);
}

const QDateTime &KOAlarm::time() const
{
  return mAlarmTime;
}

void KOAlarm::setSnoozeTime(int alarmSnoozeTime)
{
  if (mAlarmReadOnly) return;
  mAlarmSnoozeTime = alarmSnoozeTime;
  mParent->emitEventUpdated(mParent);
}

int KOAlarm::snoozeTime() const
{
  return mAlarmSnoozeTime;
}

void KOAlarm::setRepeatCount(int alarmRepeatCount)
{
  if (mAlarmReadOnly) return;
  mAlarmRepeatCount = alarmRepeatCount;
  mParent->emitEventUpdated(mParent);
}

int KOAlarm::repeatCount() const
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
