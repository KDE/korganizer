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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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
  if ( event->isMultiDay() ) {

    ret += "<br>" + i18n("From: ");
    if (event->doesFloat())
      ret += event->dtStartDateStr();
    else
      ret += event->dtStartStr();
    ret += "<br>"+i18n("To: ") + event->dtEndStr();
    if (event->doesFloat())
      ret += event->dtEndDateStr();
    else
      ret += event->dtEndStr();

  } else {

    if ( event->doesFloat() ) {
      ret += "<br>"+i18n("Date: ")+event->dtStartDateStr();
    } else {
      ret += "<br>" + i18n("Time: ") + event->dtStartTimeStr();
      ret += " - " + event->dtEndTimeStr();
    }

  }
  return ret;
}

QString ToolTipVisitor::dateRangeText( Todo*todo )
{
  QString ret;
  bool floats( todo->doesFloat() );
  if (todo->hasStartDate())
    ret += "<br>" + i18n("Start: ") + ((floats)?(todo->dtStartDateStr()):(todo->dtStartStr()) );
  if (todo->hasDueDate())
    ret += "<br>" + i18n("Due: ") + ( (floats)?(todo->dtDueDateStr()):(todo->dtDueStr()) );
  if (todo->isCompleted())
    ret += "<br>" + i18n("Completed: ") + todo->completedStr();
  else
    ret += "<br>" + i18n("%1 % completed").arg(todo->percentComplete());

  return ret;
}

QString ToolTipVisitor::dateRangeText( Journal*journal )
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
    tipText += "<br>"+i18n("Location: ")+incidence->location().replace("\n", "<br>");
  }
  if (!incidence->description().isEmpty()) {
    tipText += "<br><hr>" +incidence->description().replace("\n", "<br>");
  }
  tipText += "</qt>";
  *mTipText = tipText;
  return true;
}
