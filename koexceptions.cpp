// $Id$

#include <klocale.h>

#include "koexceptions.h"

KOException::KOException(const QString &message)
{
  mMessage = message;
}

KOException::~KOException()
{
}

QString KOException::message()
{
  if (mMessage.isEmpty()) return i18n("KOrganizer Error");
  else return mMessage;
}
    

KOErrorFormat::KOErrorFormat(ErrorCodeFormat code,const QString &message) :
  KOException(message)
{
  mCode = code;
}
    
QString KOErrorFormat::message()
{
  QString message = "";

  switch (mCode) {
    case LoadError:
      message = i18n("Load Error");
      break;
    case ParseError:
      message = i18n("Parse Error");
      break;
    case CalVersion1:
      message = i18n("vCalendar Version 1.0 detected");
      break;
    case CalVersion2:
      message = i18n("iCalendar Version 2.0 detected");
      break;
    case Restriction:
      message = i18n("Restriction violation");
    default:
      break;
  }
  
  if (!mMessage.isEmpty()) message += ": " + mMessage;
  
  return message;
}

KOErrorFormat::ErrorCodeFormat KOErrorFormat::errorCode()
{
  return mCode;
}
