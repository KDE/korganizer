#ifndef KOMAILCLIENT_H
#define KOMAILCLIENT_H
// $Id$

#include <qstring.h>


#include <incidence.h>

class KURL;

using namespace KCal;

class KOMailClient
{
  public:
    KOMailClient();
    virtual ~KOMailClient();
    
    bool mailAttendees(Incidence *,const QString &attachment=QString::null);
    
  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from */
    bool send(const QString &from,const QString &to,const QString &subject,
              const QString &body,bool bcc=false,
              const QString &attachment=QString::null);

    QString createBody(Incidence *incidence);

    int kMailOpenComposer(const QString& to, const QString& cc,
                          const QString& bcc, const QString& subject,
                          const QString& body, int hidden,
                          const QString& attachName, const QCString& attachCte,
                          const QCString& attachData,
                          const QCString& attachType,
                          const QCString& attachSubType,
                          const QCString& attachParamAttr,
                          const QString& attachParamValue,
                          const QCString& attachContDisp );
    int kMailOpenComposer(const QString& arg0,const QString& arg1,
                          const QString& arg2,const QString& arg3,
                          const QString& arg4,int arg5,const KURL& arg6);
};

#endif
