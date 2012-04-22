/*
  This file is part of KOrganizer.

  Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KORG_KOEVENTVIEWERDIALOG_H
#define KORG_KOEVENTVIEWERDIALOG_H

#include "korganizer_export.h"

#include <KDialog>

namespace CalendarSupport {
  class Calendar;
  class IncidenceViewer;
}

namespace Akonadi {
  class Item;
}

/**
  Viewer dialog for events.
*/
class KORGANIZER_EVENTVIEWER_EXPORT KOEventViewerDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit KOEventViewerDialog( CalendarSupport::Calendar *calendar, QWidget *parent = 0 );
    virtual ~KOEventViewerDialog();

    void setIncidence( const Akonadi::Item &incidence, const QDate &date );

    void addText( const QString &text );

  private Q_SLOTS:
    void editIncidence();
    void showIncidenceContext();

  private:
    CalendarSupport::IncidenceViewer *mEventViewer;
};

#endif
