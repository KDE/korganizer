#ifndef KOEXCEPTIONS_H
#define KOEXCEPTIONS_H
// $Id$
//
// Exception classes of KOrganizer.
//
// We don't use actual C++ exceptions right now. These classes are currently
// returned by an error function, but we can build upon them, when we start
// to use C++ exceptions.

#include <qstring.h>

/** KOrganizer exceptions base class */
class KOException {
  public:
    KOException(const QString &message=QString::null);
    virtual ~KOException();
    
    virtual QString message();
    
  protected:
    QString mMessage;
};

/** Calendar format related error class */
class KOErrorFormat : public KOException {
  public:
    enum ErrorCodeFormat { LoadError,ParseError,CalVersion1,CalVersion2,
                           CalVersionUnknown,
                           Restriction };
  
    KOErrorFormat(ErrorCodeFormat code,const QString &message = QString::null);
    
    QString message();
    ErrorCodeFormat errorCode();
    
  private:
    ErrorCodeFormat mCode;
};

#endif
