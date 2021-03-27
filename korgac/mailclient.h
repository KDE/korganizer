/*
  SPDX-FileCopyrightText: 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <KCalendarCore/IncidenceBase>

#include <QObject>

/**
 * MailClient is in kdepimlibs/akonadi/calendar now, but it's private API and I don't
 * feel like making public right now, hence this copy.
 *
 * Theres probably non calendaring-specific APIs to send e-mails, so I'd like to keep
 * MailClient private in kdepimlibs.
 */
namespace KIdentityManagement
{
class Identity;
}

namespace KOrg
{
class MailClient : public QObject
{
    Q_OBJECT
public:
    MailClient();
    ~MailClient();

    /**
     * Return the error description string.
     * Empty if there are no errors.
     */
    Q_REQUIRED_RESULT const QString errorMsg();

    Q_REQUIRED_RESULT bool mailAttendees(const KCalendarCore::IncidenceBase::Ptr &,
                                         const KIdentityManagement::Identity &identity,
                                         bool bccMe,
                                         const QString &attachment = QString(),
                                         const QString &mailTransport = QString());

    Q_REQUIRED_RESULT bool mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &,
                                         const KIdentityManagement::Identity &identity,
                                         const QString &from,
                                         bool bccMe,
                                         const QString &attachment = QString(),
                                         const QString &sub = QString(),
                                         const QString &mailTransport = QString());

    Q_REQUIRED_RESULT bool mailTo(const KCalendarCore::IncidenceBase::Ptr &,
                                  const KIdentityManagement::Identity &identity,
                                  const QString &from,
                                  bool bccMe,
                                  const QString &recipients,
                                  const QString &attachment = QString(),
                                  const QString &mailTransport = QString());

    /**
      Sends mail with specified from, to and subject field and body as text.
      If bcc is set, send a blind carbon copy to the sender

      @param identity is the Identity of the sender
      @param from is the address of the sender of the message
      @param to a list of addresses to receive the message
      @param cc a list of addresses to receive message carbon copies
      @param subject is the subject of the message
      @param body is the boody of the message
      @param hidden if true and using KMail as the mailer, send the message
      without opening a composer window.
      @param bcc if true, send a blind carbon copy to the message sender
      @param attachment optional attachment (raw data)
      @param mailTransport defines the mail transport method. See here the
      kdepimlibs/mailtransport library.
    */
    Q_REQUIRED_RESULT bool send(const KIdentityManagement::Identity &identity,
                                const QString &from,
                                const QString &to,
                                const QString &cc,
                                const QString &subject,
                                const QString &body,
                                bool hidden = false,
                                bool bccMe = false,
                                const QString &attachment = QString(),
                                const QString &mailTransport = QString());

private:
    QString errorString;
};
}

