/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORALARMS_H
#define KOEDITORALARMS_H

#include <qwidget.h>
#include <koeditoralarms_base.h>

namespace KCal {
class Incidence;
}
using namespace KCal;

class QListViewItem;
class QListView;

class KOEditorAlarms : public KOEditorAlarms_base
{
    Q_OBJECT
  public:
    KOEditorAlarms( int spacing = 8, QWidget *parent = 0,
                    const char *name = 0 );
    ~KOEditorAlarms();

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( Incidence * );
    /** Write event settings to event object */
    void writeIncidence( Incidence * );

  protected slots:
    void slotAdd();
    void slotEdit();
    void slotRemove();
};

#endif
