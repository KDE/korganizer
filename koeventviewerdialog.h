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

#include "korganizerprivate_export.h"

#include <QDialog>
class QPushButton;

namespace CalendarSupport {
  class IncidenceViewer;
}

namespace Akonadi {
  class Item;
  class ETMCalendar;
}

/**
  Viewer dialog for events.
*/
class KORGANIZERPRIVATE_EXPORT KOEventViewerDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit KOEventViewerDialog( Akonadi::ETMCalendar *calendar, QWidget *parent = 0 );
    virtual ~KOEventViewerDialog();

    void setIncidence( const Akonadi::Item &incidence, const QDate &date );

    void addText( const QString &text );

    QPushButton *editButton() const;


  private Q_SLOTS:
    void editIncidence();
    void showIncidenceContext();

  private:
    CalendarSupport::IncidenceViewer *mEventViewer;
    QPushButton *mUser1Button;
};

#endif
