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

// TODO: remove all the getBlah() functions.
// TODO: make KORecurrence and KOAlarm members instead of base classes

class Incidence : public QObject, public KORecurrence, public KOAlarm
{
    Q_OBJECT
  public:
    Incidence();
    ~Incidence();
    
    /** Recreate event. The event is made a new unique event, but already stored
    event information is preserved. Sets uniquie id, creation date, last
    modification date and revision number. */
    void recreate();
    
    /** sets the event to be read only or not */
    void setReadOnly(bool readonly);
    /** returns the event's read only status */
    bool isReadOnly() const { return mReadOnly; }

    const QDateTime &dtStart() const;

    void setLastModified(const QDateTime &lm);
    const QDateTime &lastModified() const;
    const QDateTime &getLastModified() const { return lastModified(); }

    /** set creation date */
    void setCreated(QDateTime);
    /** return time and date of cration. */
    QDateTime created() const;

    /** set the unique text string for the event */
    void setVUID(const QString &);
    /** get the unique text string for the event */
    const QString &VUID() const;
    const QString &getVUID() const { return VUID(); }

    /** set the number of revisions this event has seen */
    void setRevision(int rev);
    /** return the number of revisions this event has seen */
    int revision() const;

    /** sets the organizer for the event */
    void setOrganizer(const QString &o);
    const QString &organizer() const;
    const QString &getOrganizer() const { return organizer(); }

    /** attendee stuff */
    void addAttendee(Attendee *a);
    void removeAttendee(Attendee *a);
    void removeAttendee(const char *n);
    void clearAttendees();
    Attendee *getAttendee(const char *n) const;
    // TODO: Remove get from function name
    const QList<Attendee> &getAttendeeList() const { return mAttendees; };
    int attendeeCount() const { return mAttendees.count(); };

    /** sets the event's lengthy description. */
    void setDescription(const QString &description);
    /** sets the event's lengthy description. */
//    void setDescription(const char *);
    /** returns a reference to the event's description. */
    const QString &description() const;
    const QString &getDescription() const { return description(); }

    /** sets the event's short summary. */
    void setSummary(const QString &summary);
    /** sets the event's short summary. */
//    void setSummary(const char *);
    /** returns a reference to the event's summary. */
    const QString &summary() const;
    const QString &getSummary() const { return summary(); }

    /** set event's applicable categories */
    void setCategories(const QStringList &categories);
    /** set event's categories based on a comma delimited string */
    void setCategories(const QString &catStr);
    /** return categories in a list */
    const QStringList &categories() const;
    const QStringList &getCategories() const { return categories(); }
    /** return categories as a comma separated string */
    QString categoriesStr();
    QString getCategoriesStr() { return categoriesStr(); }

    /** point at some other event to which the event relates. This function should
     *  only be used when constructing a calendar before the related KOEvent
     *  exists. */
    void setRelatedToVUID(const QString &);
    /** what event does this one relate to? This function should
     *  only be used when constructing a calendar before the related KOEvent
     *  exists. */
    const QString &relatedToVUID() const;
    const QString &getRelatedToVUID() const { return relatedToVUID(); }
    /** point at some other event to which the event relates */
    void setRelatedTo(Incidence *relatedTo);
    /** what event does this one relate to? */
    Incidence *relatedTo() const;
    Incidence *getRelatedTo() const { return relatedTo(); }
    /** All events that are related to this event */
    const QList<Incidence> &relations() const;
    const QList<Incidence> &getRelations() const { return relations(); }
    /** Add an event which is related to this event */
    void addRelation(Incidence *);
    /** Remove event that is related to this event */
    void removeRelation(Incidence *);

    /** returns the list of dates which are exceptions to the recurrence rule */
    const QDateList &exDates() const;
    const QDateList &getExDates() const { return exDates(); }
    /** sets the list of dates which are exceptions to the recurrence rule */
    void setExDates(const QDateList &_exDates);
    void setExDates(const char *dates);
    void addExDate(const QDate &date);

    /** returns true if there is an exception for this date in the recurrence
     rule set, or false otherwise. */
    bool isException(const QDate &qd) const;

    /** returns TRUE if the date specified is one on which the event will
     * recur. */
    bool recursOn(const QDate &qd) const;

    void emitEventUpdated(Incidence *i) { emit eventUpdated(i); }

  signals:
    void eventUpdated(Incidence *);

  protected:
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
    
    bool mReadOnly;
};

#endif
