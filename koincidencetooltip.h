/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Reinhold Kainhofer <reinhhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOINCIDENCETOOLTIP_H
#define KOINCIDENCETOOLTIP_H

#include <qtooltip.h>

namespace KCal
{
class Incidence;
}
using namespace KCal;

class KOAgendaItem;

/**
@author Reinhold Kainhofer
*/
class KOIncidenceToolTip : public QToolTip
{
  public:
    KOIncidenceToolTip(QWidget * widget, QToolTipGroup * group = 0 ):QToolTip (widget, group) {}
/*    ~KOIncidenceToolTip();*/

  public:
    static void add ( QWidget * widget, Incidence *incidence,
        QToolTipGroup * group = 0, const QString & longText = "" );
    static void add( KOAgendaItem *item, Incidence *incidence = 0,
                     QToolTipGroup *group = 0 );

    /* reimplmented from QToolTip */
    void maybeTip( const QPoint &pos );

  private:
    QString mText;
};

#endif
