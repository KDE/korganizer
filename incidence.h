#ifndef INCIDENCE_H
#define INCIDENCE_H
// $Id$
//
// Incidence - base class of calendaring components
//

#include <qdatetime.h>
#include <qobject.h>

#include "attendee.h"
#include "korecurrence.h"
#include "koalarm.h"
#include "incidencevisitor.h"

class Incidence : public QObject
{
    Q_OBJECT
  public:
    /** enumeration for describing an event's secrecy. */
    enum { SecrecyPublic = 0, SecrecyPrivate = 1, SecrecyConfidential = 2 };

    Incidence();
    ~Incidence();

    virtual bool accept(IncidenceVisitor &) { return false; }
    
    /** Recreate event. The event is made a new unique event, but already stored
    event information is preserved. Sets uniquie id, creation date, last
    modification date and revision number. */
    void recreate();
    
    /** sets the event to be read only or not */
    void setReadOnly(bool readonly);
    /** returns the event's read only status */
    bool isReadOnly() const { return mReadOnly; }

    void setLastModified(const QDateTime &lm);
    const QDateTime &lastModified() const;

    /** set creation date */
    void setCreated(QDateTime);
    /** return time and date of cration. */
    QDateTime created() const;

    /** set the unique text string for the event */
    void setVUID(const QString &);
    /** get the unique text string for the event */
    const QString &VUID() const;

    /** set the number of revisions this event has seen */
    void setRevision(int rev);
    /** return the number of revisions this event has seen */
    int revision() const;

    /** sets the organizer for the event */
    void setOrganizer(const QString &o);
    const QString &organizer() const;

    /** for setting the event's starting date/time with a QDateTime. */
    void setDtStart(const QDateTime &dtStart);
    /** returns an event's starting date/time as a QDateTime. */
    const QDateTime &dtStart() const;
    /** returns an event's starting time as a string formatted according to the
     users locale settings */
    QString dtStartTimeStr() const;
    /** returns an event's starting date as a string formatted according to the
     users locale settings */
    QString dtStartDateStr(bool shortfmt=true) const;
    /** returns an event's starting date and time as a string formatted according
     to the users locale settings */
    QString dtStartStr() const;

    /** returns TRUE or FALSE depending on whether the event "floats,"
     * or doesn't have a time attached to it, only a date. */
    bool doesFloat() const;
    /** sets the event's float value. */
    void setFloats(bool f);

    /** attendee stuff */
    void addAttendee(Attendee *a);
//    void removeAttendee(Attendee *a);
//    void removeAttendee(const char *n);
    void clearAttendees();
//    Attendee *getAttendee(const char *n) const;
    // TODO: Remove get from function name
    const QList<Attendee> &attendees() const { return mAttendees; };
    int attendeeCount() const { return mAttendees.count(); };

    /** sets the event's lengthy description. */
    void setDescription(const QString &description);
    /** returns a reference to the event's description. */
    const QString &description() const;

    /** sets the event's short summary. */
    void setSummary(const QString &summary);
    /** returns a reference to the event's summary. */
    const QString &summary() const;

    /** set event's applicable categories */
    void setCategories(const QStringList &categories);
    /** set event's categories based on a comma delimited string */
    void setCategories(const QString &catStr);
    /** return categories in a list */
    const QStringList &categories() const;
    /** return categories as a comma separated string */
    QString categoriesStr();

    /** point at some other event to which the event relates. This function should
     *  only be used when constructing a calendar before the related Event
     *  exists. */
    void setRelatedToVUID(const QString &);
    /** what event does this one relate to? This function should
     *  only be used when constructing a calendar before the related Event
     *  exists. */
    const QString &relatedToVUID() const;
    /** point at some other event to which the event relates */
    void setRelatedTo(Incidence *relatedTo);
    /** what event does this one relate to? */
    Incidence *relatedTo() const;
    /** All events that are related to this event */
    const QList<Incidence> &relations() const;
    /** Add an event which is related to this event */
    void addRelation(Incidence *);
    /** Remove event that is related to this event */
    void removeRelation(Incidence *);

    /** returns the list of dates which are exceptions to the recurrence rule */
    const QDateList &exDates() const;
    /** sets the list of dates which are exceptions to the recurrence rule */
    void setExDates(const QDateList &_exDates);
    void setExDates(const char *dates);
    void addExDate(const QDate &date);

    /** returns true if there is an exception for this date in the recurrence
     rule set, or false otherwise. */
    bool isException(const QDate &qd) const;

    /** set the list of attachments/associated files for this event */
    void setAttachments(const QStringList &attachments);
    /** return list of associated files */
    const QStringList &attachments() const;

    /** sets the event's status the value specified.  See the enumeration
     * above for possible values. */
    void setSecrecy(int);
    /** return the event's secrecy. */
    int secrecy() const;
    /** return the event's secrecy in string format. */
    QString secrecyStr() const;
    /** return list of all availbale secrecy classes */
    static QStringList secrecyList();
    /** return human-readable name of secrecy class */
    static QString secrecyName(int);

    /** pilot syncronization routines */
    enum { SYNCNONE = 0, SYNCMOD = 1, SYNCDEL = 3 };
    void setPilotId(int id);
    int pilotId() const;
    
    void setSyncStatus(int stat);
    int syncStatus() const;

    /** returns TRUE if the date specified is one on which the event will
     * recur. */
    bool recursOn(const QDate &qd) const;

    void emitEventUpdated(Incidence *i) { emit eventUpdated(i); }

    // VEVENT and VTODO, but not VJOURNAL (move to EventBase class?):

    /** set resources used, such as Office, Car, etc. */
    void setResources(const QStringList &resources);
    /** return list of current resources */
    const QStringList &resources() const;

    /** set the event's priority, 0 is undefined, 1 highest (decreasing order) */
    void setPriority(int priority);
    /** get the event's priority */
    int priority() const;

    KOAlarm *alarm() const;
    KORecurrence *recurrence() const;

  signals:
    void eventUpdated(Incidence *);

  protected:
    QDate strToDate(const QString &dateStr);

    bool mReadOnly;

  private:
    // base components
    QDateTime mDtStart;
    QString mOrganizer;
    QString mVUID;
    int mRevision;
    QList<Attendee> mAttendees;

    // base components of jounal, event and todo
    QDateTime mLastModified;
    QDateTime mCreated;
    QString mDescription;
    QString mSummary;
    QStringList mCategories;
    Incidence *mRelatedTo;      
    QString mRelatedToVUID;   
    QList<Incidence> mRelations;
    QDateList mExDates;
    QStringList mAttachments;
    QStringList mResources;  

    int mSecrecy;
    int mPriority;                        // 1 = highest, 2 = less, etc.

    // PILOT SYNCHRONIZATION STUFF
    int mPilotId;                         // unique id for pilot sync
    int mSyncStatus;                      // status (for sync)

    bool mFloats;                         // floating means date without time
  
    KOAlarm *mAlarm;
    KORecurrence *mRecurrence;
};

#endif
