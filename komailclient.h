// The interface for komailclient class
// $Id$
#ifndef _KOMAILCLIENT_H
#define _KOMAILCLIENT_H

#define COMMA ","
#define XMAILER "X-Mailer: KOrganizer "
#define SUBJECT Meeting Notification

#include "koevent.h"
#include "calobject.h"

/**
 * A Class to maintain the Mailing Headers
 */
class MailMsgString 
{
 public:
    MailMsgString();
    virtual ~MailMsgString();
    void addAddressee(Attendee *);
    void addFrom(const char *);
    /**
     * Method to build a plain text (text/plain) message body
     */
    void buildTextMsg(KOEvent *);
    /**
     * This method returns the Headers in a QString object.
     */
    QString * getHeaders();
    /**
     * This method returns the body of a mail msg
     */
    QString * getBody() { return textString; }

    public slots:

 protected:
    int numberOfAddresses;
    QString * xMailer;
    QString * Addressee;
    QString * From;
    QString * Subject;
    QString * Headers;
    QString * textString;
};

class KoMailClient :  public QObject
{
    Q_OBJECT

public:
    KoMailClient(CalObject *cal);
    virtual ~KoMailClient();
    
public slots:
    void emailEvent(KOEvent *,QWidget *);

protected:

    CalObject * calendar;
};

#endif
