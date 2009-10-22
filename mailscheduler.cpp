/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "mailscheduler.h"
#include "incidencechanger.h"
#include "kocore.h"
#include "komailclient.h"
#include "koprefs.h"
#include "akonadicalendar.h"
#include <interfaces/korganizer/akonadicalendaradaptor.h>

#include <KCal/Calendar>
#include <KCal/ICalFormat>
//#include <KCal/Scheduler>
#include <KCal/IncidenceBase>
#include <KCal/AssignmentVisitor>

#include <KPIMIdentities/IdentityManager>

#include <KStandardDirs>

#include <QDir>

using namespace KOrg;

MailScheduler::MailScheduler( KOrg::AkonadiCalendar *calendar, KOrg::IncidenceChangerBase *changer )
  //: Scheduler( calendar )
  : mCalendar( calendar ), mChanger( changer ), mFormat( new ICalFormat() )
{
  mFormat->setTimeSpec( calendar->timeSpec() );
}

MailScheduler::~MailScheduler()
{
}

bool MailScheduler::publish( KCal::IncidenceBase *incidence, const QString &recipients )
{
  QString from = KOPrefs::instance()->email();
  bool bccMe = KOPrefs::instance()->mBcc;
  bool useSendmail = ( KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail );
  QString messageText = mFormat->createScheduleMessage( incidence, KCal::iTIPPublish );

  KOMailClient mailer;
  return mailer.mailTo(
    incidence,
    KOCore::self()->identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, useSendmail );
}

bool MailScheduler::performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method, const QString &recipients )
{
  QString from = KOPrefs::instance()->email();
  bool bccMe = KOPrefs::instance()->mBcc;
  bool useSendmail = ( KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail );
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  return mailer.mailTo(
    incidence,
    KOCore::self()->identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, useSendmail );
}

bool MailScheduler::performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method )
{
  QString from = KOPrefs::instance()->email();
  bool bccMe = KOPrefs::instance()->mBcc;
  bool useSendmail = ( KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail );
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  bool status;
  if ( method == iTIPRequest ||
       method == iTIPCancel ||
       method == iTIPAdd ||
       method == iTIPDeclineCounter ) {
    status = mailer.mailAttendees(
      incidence,
      KOCore::self()->identityManager()->identityForAddress( from ),
      bccMe, messageText, useSendmail );
  } else {
    QString subject;
    Incidence *inc = dynamic_cast<Incidence*>( incidence );
    if ( inc && method == iTIPCounter ) {
      subject = i18n( "Counter proposal: %1", inc->summary() );
    }
    status = mailer.mailOrganizer(
      incidence,
      KOCore::self()->identityManager()->identityForAddress( from ),
      from, bccMe, messageText, subject, useSendmail );
  }
  return status;
}

QList<ScheduleMessage*> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = KStandardDirs::locateLocal( "data", "korganizer/income" );
  kDebug() << "dir:" << incomingDirName;

  QList<ScheduleMessage*> messageList;

  QDir incomingDir( incomingDirName );
  QStringList incoming = incomingDir.entryList( QDir::Files );
  QStringList::ConstIterator it;
  for ( it = incoming.constBegin(); it != incoming.constEnd(); ++it ) {
    kDebug() << "-- File:" << (*it);

    QFile f( incomingDirName + '/' + (*it) );
    bool inserted = false;
    QMap<IncidenceBase *, QString>::Iterator iter;
    for ( iter = mEventMap.begin(); iter != mEventMap.end(); ++iter ) {
      if ( iter.value() == incomingDirName + '/' + (*it) ) {
        inserted = true;
      }
    }
    if ( !inserted ) {
      if ( !f.open( QIODevice::ReadOnly ) ) {
        kDebug() << "Can't open file'" << (*it) << "'";
      } else {
        QTextStream t( &f );
        t.setCodec( "ISO 8859-1" );
        QString messageString = t.readAll();
        messageString.remove( QRegExp( "\n[ \t]" ) );
        messageString = QString::fromUtf8( messageString.toLatin1() );

        AkonadiCalendarAdaptor caladaptor(mCalendar, mChanger);
        ScheduleMessage *mess = mFormat->parseScheduleMessage( &caladaptor, messageString );
        
        if ( mess ) {
          kDebug() << "got message '" << (*it) << "'";
          messageList.append( mess );
          mEventMap[ mess->event() ] = incomingDirName + '/' + (*it);
        } else {
          QString errorMessage;
          if ( mFormat->exception() ) {
            errorMessage = mFormat->exception()->message();
          }
          kDebug() << "Error parsing message:" << errorMessage;
        }
        f.close();
      }
    }
  }
  return messageList;
}

bool MailScheduler::deleteTransaction( IncidenceBase *incidence )
{
  bool status;
  QFile f( mEventMap[incidence] );
  mEventMap.remove( incidence );
  if ( !f.exists() ) {
    status = false;
  } else {
    status = f.remove();
  }
  return status;
}

QString MailScheduler::freeBusyDir()
{
  return KStandardDirs::locateLocal( "data", "korganizer/freebusy" );
}

bool MailScheduler::acceptTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method, KCal::ScheduleMessage::Status status, const QString &email )
{
  class SchedulerAdaptor : public KCal::Scheduler
  {
    public:
      SchedulerAdaptor(MailScheduler* s, AkonadiCalendarAdaptor *c) : KCal::Scheduler(c), m_scheduler(s), m_calendar(c) {}
      virtual ~SchedulerAdaptor() {}
      virtual bool publish ( KCal::IncidenceBase *incidence, const QString &recipients ) {
        return m_scheduler->publish( incidence, recipients );
      }
      virtual bool performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method ) {
        return m_scheduler->performTransaction( incidence, method );
      }
      virtual bool performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method, const QString &recipients ) {
        return m_scheduler->performTransaction( incidence, method, recipients );
      }
      virtual bool acceptTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method, KCal::ScheduleMessage::Status status, const QString &email ) {
        return m_scheduler->acceptTransaction( incidence, method, status, email );
      }
      virtual bool acceptCounterProposal( KCal::Incidence *incidence ) {
        return m_scheduler->acceptCounterProposal( incidence );
      }
      virtual QList<ScheduleMessage*> retrieveTransactions() {
        return m_scheduler->retrieveTransactions();
      }
      virtual QString freeBusyDir() {
        return m_scheduler->freeBusyDir();
      }
    private:
      MailScheduler* m_scheduler;
      AkonadiCalendarAdaptor *m_calendar;
  };

  AkonadiCalendarAdaptor caladaptor(mCalendar, mChanger);
  SchedulerAdaptor scheduleradaptor(this, &caladaptor);
  return scheduleradaptor.acceptTransaction(incidence, method, status, email);
}

//AKONADI_PORT review following code
bool MailScheduler::acceptCounterProposal( KCal::Incidence *incidence )
{
  if ( !incidence ) {
    return false;
  }

  Akonadi::Item::Id akonadiId = mCalendar->itemIdForIncidenceUid( incidence->uid() );
  Akonadi::Item exInc = mCalendar->incidence( akonadiId );
  if ( ! exInc.isValid() ) {
    exInc = mCalendar->incidenceFromSchedulingID( incidence->uid() );
    //exInc = exIncItem.isValid() && exIncItem.hasPayload<Incidence::Ptr>() ? exIncItem.payload<Incidence::Ptr>() : Incidence::Ptr();
  }

  incidence->setRevision( incidence->revision() + 1 );
  if ( exInc.isValid() && exInc.hasPayload<Incidence::Ptr>() ) {
    Incidence::Ptr exIncPtr = exInc.payload<Incidence::Ptr>();
    incidence->setRevision( qMax( incidence->revision(), exIncPtr->revision() + 1 ) );
    // some stuff we don't want to change, just to be safe
    incidence->setSchedulingID( exIncPtr->schedulingID() );
    incidence->setUid( exIncPtr->uid() );

    mCalendar->beginChange( exInc );

    Q_ASSERT( exIncPtr.get() && incidence );
    KCal::AssignmentVisitor v;
    v.assign( exIncPtr.get(), incidence );

    exIncPtr->updated();
    mCalendar->endChange( exInc );
  } else {
#ifdef AKONADI_PORT_DISABLED
    mCalendar->addIncidence( Incidence::Ptr(incidence->clone()) );
#endif
  }

  return true;
}
