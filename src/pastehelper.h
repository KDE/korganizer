/*
  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Calendar>
#include <KCalendarCore/Incidence>

#include <QDateTime>

namespace PasteHelper
{
enum PasteFlag {
    FlagTodosPasteAtDtStart = 1, /*!< If the cloned incidence is a to-do, the date/time passed
                                    to DndFactory::pasteIncidence() will change dtStart if this
                                    flag is on, changes dtDue otherwise. */
    FlagPasteAtOriginalTime = 2 /*!< If set, incidences will be pasted at the specified date
                                   but will preserve their original time */
};

Q_DECLARE_FLAGS(PasteFlags, PasteFlag)

/*!
  This function clones the incidences that are in the clipboard and sets the clone's
  date/time to the specified \a newDateTime.

  \a newDateTime The new date/time that the incidence will have. If it's an event
  or journal, DTSTART will be set. If it's a to-do, DTDUE is set.
  If you wish another behaviour, like changing DTSTART on to-dos, specify
  \a pasteOptions. If newDateTime is invalid the original incidence's dateTime
  will be used, regardless of \a pasteOptions.

  \a pasteOptions Control how \a newDateTime changes the incidence's dates. \sa PasteFlag.

  Returns the cloned incidence.
*/
KCalendarCore::Incidence::List pasteIncidences(const QDateTime &newDateTime = QDateTime(), PasteFlags pasteOptions = PasteFlags());
}
