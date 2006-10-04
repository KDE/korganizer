/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>

#include <klocale.h>
#include "koincidencetooltip.h"
#include "koagendaitem.h"
#include "kolistview.h"
#include "komonthview.h"
#include "kotodoviewitem.h"

// explicit instantiations
template class KOIncidenceToolTip<KOAgendaItem>;
template class ToolTipVisitor<KOAgendaItem>;
template class ToolTipVisitor<KOListViewItem>;
template class ToolTipVisitor<MonthViewItem>;
template class ToolTipVisitor<KOTodoViewItem>;

/**
@author Reinhold Kainhofer
some improvements by Mikolaj Machowski
*/

/*
 template<class T>
void KOIncidenceToolTip<T>::add ( T* item,
        QToolTipGroup * group, const QString & longText )
{
}

*/

template<class T>
QString ToolTipVisitor<T>::dateRangeText( Event*event )
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
    ret += "<br>"+i18n("<i>Date:</i>&nbsp;");
    if ( event->doesRecur() ) {
      ret += KGlobal::locale()->formatDate( mItem->itemDate(), true );
    } else {
      ret += event->dtStartDateStr().replace(" ", "&nbsp;");
    }
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

template<class T>
QString ToolTipVisitor<T>::dateRangeText( Todo*todo )
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

template<class T>
QString ToolTipVisitor<T>::dateRangeText( Journal*journal )
{
  QString ret;
  if (journal->dtStart().isValid() ) {
    ret += "<br>" + i18n("<i>Date:</i>&nbsp;%1").arg( journal->dtStartDateStr( false ) );
  }
  return ret;
}


template<class T>
bool ToolTipVisitor<T>::visit( Event *event )
{
  QString dtRangeText( dateRangeText( event ) );
  return generateToolTip( event, dtRangeText  );
}

template<class T>
bool ToolTipVisitor<T>::visit( Todo *todo )
{
  QString dtRangeText( dateRangeText( todo ) );
  return generateToolTip( todo, dtRangeText  );
}

template<class T>
bool ToolTipVisitor<T>::visit( Journal *journal )
{
  QString dtRangeText( dateRangeText( journal ) );
  return generateToolTip( journal, dtRangeText  );
}

template<class T>
bool ToolTipVisitor<T>::generateToolTip( Incidence* incidence, QString dtRangeText )
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
