/*
    This file is part of KOrganizer.
    Copyright (c) 2003
    Reinhold Kainhofer <reinhhold@kainhofer.com>

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
#ifndef KOINCIDENCETOOLTIP_H
#define KOINCIDENCETOOLTIP_H

#include <qtooltip.h>

namespace KCal
{
class Incidence;
class Event;
class Todo;
class Journal;
}
using namespace KCal;

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
};

class ToolTipVisitor : public Incidence::Visitor
{
  public:
    ToolTipVisitor() : mRichText( true ),mTipText(0) {}

    bool act( Incidence *incidence, QString* tipText, bool richText=true)
    {
      mTipText = tipText;
      mRichText = richText;
      return incidence->accept( *this );
    }

  protected:
    bool visit( Event *event );
    bool visit( Todo *todo );
    bool visit( Journal *journal );

    QString dateRangeText( Event*event );
    QString dateRangeText( Todo *todo );
    QString dateRangeText( Journal *journal );

    bool generateToolTip( Incidence* incidence, QString dtRangeText );

  protected:
    bool mRichText;
    QString *mTipText;
};


#endif
