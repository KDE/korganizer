#ifndef KOMAILCLIENT_H
#define KOMAILCLIENT_H
// $Id$

#include <qstring.h>

#include <kurl.h>

#include <incidence.h>

using namespace KCal;

class KOMailClient
{
  public:
    KOMailClient();
    virtual ~KOMailClient();
    
    bool mailAttendees(Incidence *);
    
  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from */
    bool send(const QString &from,const QString &to,const QString &subject,
              const QString &body,bool bcc=false);

    QString createBody(Incidence *incidence);

    int kMailOpenComposer(const QString& arg0,const QString& arg1,
                          const QString& arg2,const QString& arg3,
                          const QString& arg4,int arg5,const KURL& arg6);
};

#endif
