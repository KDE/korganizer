// $Id$

#include <klocale.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <kdebug.h>

#include "calformat.h"

CalFormat::CalFormat(Calendar *cal)
{
  mCalendar = cal;
  
  mTopWidget = 0;
  mEnableDialogs = false;
  
  mException = 0;
}

CalFormat::~CalFormat()
{
  delete mException;
}

void CalFormat::setTopwidget(QWidget *topWidget)
{
  mTopWidget = topWidget;
}

void CalFormat::showDialogs(bool enable)
{
  mEnableDialogs = enable;
}

void CalFormat::loadError(const QString &fileName) 
{
  kdDebug() << "CalFormat::loadError()" << endl;

  if (mEnableDialogs) {
    KMessageBox::sorry(mTopWidget,
                       i18n("An error has occurred loading the file:\n"
                            "%1.\n"
                            "Please verify that the file is in vCalendar "
                            "format,\n"
                            "that it exists, and it is readeable, then "
                            "try again or load another file.\n")
                            .arg(fileName),
                       i18n("KOrganizer: Error Loading Calendar"));
  }
}

void CalFormat::clearException()
{
  delete mException;
  mException = 0;
}

void CalFormat::setException(KOErrorFormat *exception)
{
  delete mException;
  mException = exception;
}

KOErrorFormat *CalFormat::exception()
{
  return mException;
}

QString CalFormat::createUniqueId()
{
  int hashTime = QTime::currentTime().hour() + 
                 QTime::currentTime().minute() + QTime::currentTime().second() +
                 QTime::currentTime().msec();
  QString uidStr = QString("KOrganizer-%1.%2")
                           .arg(KApplication::random())
                           .arg(hashTime);
  return uidStr;
}
