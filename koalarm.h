// $Id$

#ifndef _KOALARM_H
#define _KOALARM_H

#include <qstring.h>

class Incidence;

class KOAlarm {
  public:
    /** constructs a new event with variables initialized to "sane" values. */
    KOAlarm(Incidence *parent);
    ~KOAlarm();

    void setAlarmStart(QDateTime start) { mAlarmStart = start; }
    void setAlarmReadOnly(bool readOnly ) { mAlarmReadOnly = readOnly; }

    /** set the event to have this file as the noise for the alarm. */
    void setAudioAlarmFile(const QString &audioAlarmFile);
    /** return the name of the audio file for the alarm */
    const QString &audioAlarmFile() const;

    /** set this program to run when an alarm is triggered */
    void setProgramAlarmFile(const QString &programAlarmFile);
    /** return the name of the program to run when an alarm is triggered */
    const QString &programAlarmFile() const;

    /** send mail to this address when an alarm goes off */
    void setMailAlarmAddress(const QString &mailAlarmAddress);
    /** return the address to send mail to when an alarm goes off */
    const QString &mailAlarmAddress() const;

    /** set the text to display when an alarm goes off */
    void setAlarmText(const QString &alarmText);
    /** return the text string that displays when an alarm goes off */
    const QString &alarmText() const;

    /** set the time to trigger an alarm */
    void setAlarmTime(const QDateTime &alarmTime);
    /** return the date/time when an alarm goes off */
    const QDateTime &alarmTime() const;

    /** set the interval between snoozes for the alarm */
    void setAlarmSnoozeTime(int alarmSnoozeTime);
    /** get how long the alarm snooze interval is */
    int alarmSnoozeTime() const;

    /** set how many times an alarm is to repeat itself (w/snoozes) */
    void setAlarmRepeatCount(int alarmRepeatCount);
    /** get how many times an alarm repeats */
    int alarmRepeatCount() const;

    /** toggles the value of alarm to be either on or off.
        set's the alarm time to be x minutes before dtStart time. */
    void toggleAlarm();

  private:
    QString mAudioAlarmFile;              // url/filename of sound to play
    QString mProgramAlarmFile;            // filename of program to run
    QString mMailAlarmAddress;            // who to mail for reminder
    QString mAlarmText;                   // text to display/mail for alarm

    QDateTime mAlarmTime;                 // time at which to display the alarm
    int mAlarmSnoozeTime;                 // number of minutes after alarm to
                                          // snooze before ringing again
    int mAlarmRepeatCount;                // number of times for alarm to repeat

    QDateTime mAlarmStart;
    bool mAlarmReadOnly;

    Incidence *mParent;
};

#endif
