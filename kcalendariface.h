/*
    This file is a generic DCOP interface, shared between KDE applications.
    Copyright (c) 2003 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCALENDARIFACE_H
#define KCALENDARIFACE_H

/** @file
* This file is a generic DCOP interface, shared between KDE applications.
* It handles Calendar requests.
*/

#include <dcopobject.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qstringlist.h>
// yes, this is this very header - but it tells dcopidl to include it
// in _stub.cpp and _skel.cpp files, to get the definition of the structs.
#include "kcalendariface.h"

typedef QPair<QDateTime, QDateTime> QDateTimePair;

/** Interface class for calendar requests. */
class KCalendarIface : public DCOPObject
{
    K_DCOP
  public:
    KCalendarIface() : DCOPObject("CalendarIface") {}

  k_dcop:

    /** This is a struct.
     *
     */
    struct ResourceRequestReply {
        bool vCalInOK;
        QString vCalOut;
        bool vCalOutOK; bool isFree;
        QDateTime start; QDateTime end;
    };
    virtual KCalendarIface::ResourceRequestReply resourceRequest(
                         const QValueList< QDateTimePair >& busy,
                         const QCString& resource,
                         const QString& vCalIn ) = 0;

    virtual void openEventEditor( const QString& text ) = 0;
    virtual void openEventEditor( const QString& summary,
                                  const QString& description,
                                  const QString& attachment ) = 0;
    virtual void openEventEditor( const QString& summary,
                                  const QString& description,
                                  const QString& attachment,
                                  const QStringList& attendees ) = 0;
    virtual void openEventEditor( const QString& summary,
                                  const QString& description,
                                  const QString& uri,
                                  const QString& file,
                                  const QStringList& attendees,
                                  const QString& attachmentMimetype ) = 0;

    virtual void openTodoEditor( const QString& text ) = 0;
    virtual void openTodoEditor( const QString& summary,
                                 const QString& description,
                                 const QString& attachment ) = 0;
    virtual void openTodoEditor( const QString& summary,
                                 const QString& description,
                                 const QString& attachment,
                                 const QStringList& attendees ) = 0;
    virtual void openTodoEditor( const QString& summary,
                                 const QString& description,
                                 const QString& uri,
                                 const QString& file,
                                 const QStringList& attendees,
                                 const QString& attachmentMimetype,
                                 bool isTask ) = 0;

    virtual void openJournalEditor( const QDate& date ) = 0;
    virtual void openJournalEditor( const QString& text,
                                    const QDate& date ) = 0;
    virtual void openJournalEditor( const QString& text ) = 0;
   //TODO:
   // virtual void openJournalEditor( const QString& summary,
   //                                 const QString& description,
   //                                 const QString& attachment ) = 0;

    virtual void showJournalView() = 0;
    virtual void showTodoView() = 0;
    virtual void showEventView() = 0;

    virtual void goDate( const QDate& date ) = 0;
    virtual void goDate( const QString& date ) = 0;

    virtual void showDate( const QDate &date ) = 0;
};

inline QDataStream& operator<<( QDataStream& str, const KCalendarIface::ResourceRequestReply& reply )
{
    str << reply.vCalInOK << reply.vCalOut << reply.vCalOutOK << reply.isFree << reply.start << reply.end;
    return str;
}

inline QDataStream& operator>>( QDataStream& str, KCalendarIface::ResourceRequestReply& reply )
{
    str >> reply.vCalInOK >> reply.vCalOut >> reply.vCalOutOK >> reply.isFree >> reply.start >> reply.end;
    return str;
}

#endif
