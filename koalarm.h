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
    void setAudioFile(const QString &audioAlarmFile);
    /** return the name of the audio file for the alarm */
    const QString &audioFile() const;

    /** set this program to run when an alarm is triggered */
    void setProgramFile(const QString &programAlarmFile);
    /** return the name of the program to run when an alarm is triggered */
    const QString &programFile() const;

    /** send mail to this address when an alarm goes off */
    void setMailAddress(const QString &mailAlarmAddress);
    /** return the address to send mail to when an alarm goes off */
    const QString &mailAddress() const;

    /** set the text to display when an alarm goes off */
    void setText(const QString &alarmText);
    /** return the text string that displays when an alarm goes off */
    const QString &text() const;

    /** set the time to trigger an alarm */
    void setTime(const QDateTime &alarmTime);
    /** return the date/time when an alarm goes off */
    const QDateTime &time() const;

    /** set the interval between snoozes for the alarm */
    void setSnoozeTime(int alarmSnoozeTime);
    /** get how long the alarm snooze interval is */
    int snoozeTime() const;

    /** set how many times an alarm is to repeat itself (w/snoozes) */
    void setRepeatCount(int alarmRepeatCount);
    /** get how many times an alarm repeats */
    int repeatCount() const;

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
