// The interface for KOMailClient class
// $Id$
#ifndef _KOMailClient_H
#define _KOMailClient_H

#include <qobject.h>
#include <qstring.h>

class Event;
class Attendee;

/**
 * A Class to maintain the Mailing Headers
 */
class MailMsgString 
{
  public:
    MailMsgString();
    virtual ~MailMsgString();

    /** Set addresse of mail */
    void setAddressee(Attendee *);
    /** Return addresse of mail message. */
    QString addressee() { return mAddressee; }

    /**
     * Method to build a plain text (text/plain) message body
     */
    void buildTextMsg(Event *);

    /**
     * This method returns the body of a mail msg
     */
    QString body() { return mBody; }

  private:
    QString mAddressee;
    QString mBody;
};

class KOMailClient : public QObject
{
    Q_OBJECT
  public:
    KOMailClient();
    virtual ~KOMailClient();
    
  public slots:
    void emailEvent(Event *);

  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from */
    bool sendMail(const QString &from,const QString &to,const QString &subject,
                  const QString &body,bool bcc=false);
};

#endif
