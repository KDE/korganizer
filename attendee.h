#ifndef _ATTENDEE_H
#define _ATTENDEE_H
// $Id$
//
// Attendee class

#include <qstring.h>

// TODO: remove const char * functions,
// TODO: remove get from get routines
// TODO: Rename memeber variables with leading m
class Attendee
{
  public:
    enum { NEEDS_ACTION = 0, ACCEPTED = 1, SENT = 2, TENTATIVE = 3,
	   CONFIRMED = 4, DECLINED = 5, COMPLETED = 6, DELEGATED = 7 };

    Attendee(const QString& n, const QString & e = QString::null,
             bool _rsvp=FALSE, int s = NEEDS_ACTION, int r = 0);
//    Attendee(const Attendee &);
    virtual ~Attendee();

    void setName(const char *n) { mName = n; }
    void setName(const QString &n) { mName = n; }
    QString getName() const { return name(); }
    QString name() const { return mName; }

    void setEmail(const char *e) { mEmail = e; }
    void setEmail(const QString e) { mEmail = e; }
    QString getEmail() const { return email(); }
    QString email() const { return mEmail; }

    void setRole(int r) { mRole = r; }
    int getRole() const { return role(); }
    int role() const { return mRole; }
    QString getRoleStr() const { return roleStr(); }
    QString roleStr() const;

    void setStatus(int s) { mStatus = s; }
    void setStatus(const char *s);
    int getStatus() const { return status(); }
    int status() const { return mStatus; }
    QString getStatusStr() const { return statusStr(); }
    QString statusStr() const;

    void setRSVP(bool r) { mRSVP = r; }
    void setRSVP(const char *r);
    bool RSVP() const { return mRSVP; }

    void setFlag(bool f) { mFlag = f; }
    bool flag() const { return mFlag; }

  private:
    bool mRSVP;
    int mRole;
    int mStatus;
    QString mName;
    QString mEmail;

    // used to tell whether we have need to mail this person or not.
    bool mFlag;
};

#endif
