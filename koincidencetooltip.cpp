/*
  This file is part of KOrganizer.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively

  Copyright (c) 2003
  Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>

#include <klocale.h>
#include "koincidencetooltip.h"

/**
@author Reinhold Kainhofer
some improvements by Mikolaj Machowski
*/

void KOIncidenceToolTip::add ( QWidget * widget, Incidence *incidence,
        QToolTipGroup * group, const QString & longText )
{
  if ( !widget || !incidence ) return;
  QString tipText;
  ToolTipVisitor v;
  v.act( incidence, &tipText, true );
  QToolTip::add(widget, tipText, group, longText);
}

QString ToolTipVisitor::dateRangeText( Event*event )
{
  QString ret;
  QString tmp;
  if ( event->isMultiDay() ) {

    tmp = "<br>" + i18n("Event start", "<i>From:</i>&nbsp;%1");
    if (event->doesFloat())
      ret += tmp.arg( event->dtStartDateStr().replace(" ", "&nbsp;") );
    else
      ret += tmp.arg( event->dtStartStr().replace(" ", "&nbsp;") );

    tmp = "<br>" + i18n("<i>To:</i>&nbsp;%1");
    if (event->doesFloat())
      ret += tmp.arg( event->dtEndDateStr().replace(" ", "&nbsp;") );
    else
      ret += tmp.arg( event->dtEndStr().replace(" ", "&nbsp;") );

  } else {

    ret += "<br>"+i18n("<i>Date:</i>&nbsp;%1").
        arg( event->dtStartDateStr().replace(" ", "&nbsp;") );
    if ( !event->doesFloat() ) {
      tmp = "<br>" + i18n("time range for event, &nbsp; to prevent ugly line breaks",
        "<i>Time:</i>&nbsp;%1&nbsp;-&nbsp;%2").
        arg( event->dtStartTimeStr().replace(" ", "&nbsp;") ).
        arg( event->dtEndTimeStr().replace(" ", "&nbsp;") );
      ret += tmp;
    }

  }
  return ret;
}

QString ToolTipVisitor::dateRangeText( Todo*todo )
{
  QString ret;
  bool floats( todo->doesFloat() );
  if (todo->hasStartDate())
    // No need to add <i> here. This is separated issue and each line
    // is very visible on its own. On the other hand... Yes, I like it
    // italics here :)
    ret += "<br>" + i18n("<i>Start:</i>&nbsp;%1").arg(
      (floats)
        ?(todo->dtStartDateStr().replace(" ", "&nbsp;"))
        :(todo->dtStartStr().replace(" ", "&nbsp;")) ) ;
  if (todo->hasDueDate())
    ret += "<br>" + i18n("<i>Due:</i>&nbsp;%1").arg(
      (floats)
        ?(todo->dtDueDateStr().replace(" ", "&nbsp;"))
        :(todo->dtDueStr().replace(" ", "&nbsp;")) );
  if (todo->isCompleted())
    ret += "<br>" + i18n("<i>Completed:</i>&nbsp;%1").arg( todo->completedStr().replace(" ", "&nbsp;") );
  else
    ret += "<br>" + i18n("%1 % completed").arg(todo->percentComplete());

  return ret;
}

QString ToolTipVisitor::dateRangeText( Journal* )
{
  QString ret;
  return ret;
}


bool ToolTipVisitor::visit( Event *event )
{
  QString dtRangeText( dateRangeText( event ) );
  return generateToolTip( event, dtRangeText  );
}

bool ToolTipVisitor::visit( Todo *todo )
{
  QString dtRangeText( dateRangeText( todo ) );
  return generateToolTip( todo, dtRangeText  );
}

bool ToolTipVisitor::visit( Journal *journal )
{
  QString dtRangeText( dateRangeText( journal ) );
  return generateToolTip( journal, dtRangeText  );
}

bool ToolTipVisitor::generateToolTip( Incidence* incidence, QString dtRangeText )
{
  QString tipText = "<qt><b>"+ incidence->summary().replace("\n", "<br>")+"</b>";

  tipText += dtRangeText;

  if (!incidence->location().isEmpty()) {
    // Put Location: in italics
    tipText += "<br>"+i18n("<i>Location:</i>&nbsp;%1").
      arg( incidence->location().replace("\n", "<br>") );
  }
  if (!incidence->description().isEmpty()) {
    QString desc(incidence->description());
    if (desc.length()>120) {
      desc = desc.left(120) + "...";
    }
    tipText += "<br>----------<br>" + i18n("<i>Description:</i><br>") + desc.replace("\n", "<br>");
  }
  tipText += "</qt>";
  *mTipText = tipText;
  return true;
}
