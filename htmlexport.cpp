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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qregexp.h> 

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>

#include "kocore.h"
#include "koprefs.h"
#ifndef KORG_NOKABC
 #include <kabc/stdaddressbook.h>
#endif
#include "htmlexport.h"

bool HtmlExport::save(const QString &fileName)
{
  QFile f(fileName);
  if (!f.open(IO_WriteOnly)) {
    return false;
  }
  QTextStream ts(&f);
  bool success = save(&ts);
  f.close();
  return success;
}

bool HtmlExport::save(QTextStream *ts)
{
  ts->setEncoding(QTextStream::UnicodeUTF8);

  // Write HTML header
  *ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";

  *ts << "<html><head>" << endl;
  *ts << "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
  *ts << "UTF-8\" />\n";
  *ts << "  <title>" << i18n("KOrganizer To-Do List") << "</title>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << styleSheet();
  *ts << "  </style>\n";
  *ts << "</head><body>\n";

  // TO DO: Write KOrganizer header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if (eventsEnabled() || monthViewEnabled()) {
    *ts << "<h1>" << i18n("KOrganizer Calendar") << "</h1>\n";
  }

  // Write Month View
  if (monthViewEnabled()) {
    createHtmlMonthView(ts);
  }

  // Write Event List
  if (eventsEnabled()) {
    // Write HTML page content
    createHtmlEventList(ts);
  }

  // Write Todo List
  if (todosEnabled()) {
    *ts << "<h1>" << i18n("KOrganizer To-Do List") << "</h1>\n";

    // Write HTML page content
    createHtmlTodoList(ts);
  }

  // Write KOrganizer trailer
  QString trailer =
    i18n("This page was created by <a href=\"mailto:%1\">%2</a> with "
         "<a href=\"http://korganizer.kde.org\">KOrganizer</a>")
         .arg( KOPrefs::instance()->email() )
         .arg( KOPrefs::instance()->fullName() );
  *ts << "<p>" << trailer << "</p>\n";

  // Write HTML trailer
  *ts << "</body></html>\n";

  return true;
}

void HtmlExport::createHtmlMonthView(QTextStream *ts)
{
  QDate start = fromDate();
  start.setYMD(start.year(),start.month(),1);  // go back to first day in month

  QDate end(start.year(),start.month(),start.daysInMonth());

  int startmonth = start.month();
  int startyear = start.year();

  while ( start < toDate() ) {
    // Write header
    *ts << "<h2>" << (i18n("month_year","%1 %2").arg(KGlobal::locale()->monthName(start.month()))
        .arg(start.year())) << "</h2>\n";
    if (KGlobal::locale()->weekStartsMonday()) {
      start = start.addDays(1 - start.dayOfWeek());
    } else {
      if (start.dayOfWeek() != 7) {
        start = start.addDays(-start.dayOfWeek());
      }
    }
    *ts << "<table border=\"1\">\n";

    // Write table header
    *ts << "  <tr>";
    for(int i=0; i<7; ++i) {
      *ts << "<th>" << KGlobal::locale()->weekDayName(start.addDays(i).dayOfWeek()) << "</th>";
    }
    *ts << "</tr>\n";

    // Write days
    while (start <= end) {
      *ts << "  <tr>\n";
      for(int i=0;i<7;++i) {
        *ts << "    <td valign=\"top\"><table border=\"0\">";

        QString holiday = KOCore::self()->holiday(start);

        *ts << "<tr><td ";
        if (!holiday.isEmpty() || start.dayOfWeek() == 7) {
          *ts << "class=\"dateholiday\"";
        } else {
          *ts << "class=\"date\"";
        }
        *ts << ">" << QString::number(start.day());

        if (!holiday.isEmpty()) {
          *ts << " <em>" << holiday << "</em>";
        }

        *ts << "</td></tr><tr><td valign=\"top\">";

        QPtrList<Event> events = mCalendar->events(start,true);
        if (events.count()) {
          *ts << "<table>";
          Event *ev;
          for(ev = events.first(); ev; ev = events.next()) {
            if ( checkSecrecy( ev ) ) {
              createHtmlEvent(ts,ev,start,false);
            }
          }
          *ts << "</table>";
        } else {
          *ts << "&nbsp;";
        }

        *ts << "</td></tr></table></td>\n";
        start = start.addDays(1);
      }
      *ts << "  </tr>\n";
    }
    *ts << "</table>\n";
    startmonth += 1;
    if ( startmonth > 12 ) {
      startyear += 1;
      startmonth = 1;
    }
    start.setYMD( startyear, startmonth, 1 );
    end.setYMD(start.year(),start.month(),start.daysInMonth());
  }
}

void HtmlExport::createHtmlEventList (QTextStream *ts)
{
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Start Time") << "</th>\n";
  *ts << "    <th>" << i18n("End Time") << "</th>\n";
  *ts << "    <th>" << i18n("Event") << "</th>\n";
  if (categoriesEventEnabled()) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
  }
  if (attendeesEventEnabled()) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
  }

  *ts << "  </tr>\n";

  int columns = 3;
  if (categoriesEventEnabled()) ++columns;
  if (attendeesEventEnabled()) ++columns;

  for (QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1)) {
    kdDebug() << "Getting events for " << dt.toString() << endl;
    QPtrList<Event> events = mCalendar->events(dt,true);
    if (events.count()) {
      *ts << "  <tr><td colspan=\"" << QString::number(columns)
          << "\" class=\"datehead\"><i>"
          << KGlobal::locale()->formatDate(dt)
          << "</i></td></tr>\n";
      Event *ev;
      for(ev = events.first(); ev; ev = events.next()) {
	if ( checkSecrecy( ev ) ) {
	  createHtmlEvent(ts,ev,dt);
	}
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createHtmlEvent (QTextStream *ts, Event *event,
                                       QDate date,bool withDescription)
{
  kdDebug() << "HtmlExport::createHtmlEvent(): " << event->summary() << endl;
  *ts << "  <tr>\n";

  if (!event->doesFloat()) {
    if (event->isMultiDay() && (event->dtStart().date() != date)) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">" << event->dtStartTimeStr() << "</td>\n";
    }
    if (event->isMultiDay() && (event->dtEnd().date() != date)) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">" << event->dtEndTimeStr() << "</td>\n";
    }
  } else {
    *ts << "    <td>&nbsp;</td><td>&nbsp;</td>\n";
  }

  *ts << "    <td class=\"sum\">\n";
  *ts << "      <b>" << cleanChars(event->summary()) << "</b>\n";
  if (withDescription && !event->description().isEmpty()) {
    *ts << "      <p>" << breakString(cleanChars(event->description())) << "</p>\n";
  }
  *ts << "    </td>\n";

  if (categoriesEventEnabled()) {
    *ts << "  <td>\n";
    formatHtmlCategories(ts,event);
    *ts << "  </td>\n";
  }

  if (attendeesEventEnabled()) {
    *ts << "  <td>\n";
    formatHtmlAttendees(ts,event);
    *ts << "  </td>\n";
  }

  *ts << "  </tr>\n";
}

void HtmlExport::createHtmlTodoList (QTextStream *ts)
{
  Todo *ev,*subev;

  QPtrList<Todo> rawTodoList = mCalendar->todos();
  QPtrList<Todo> todoList;

  ev = rawTodoList.first();
  while (ev) {
    subev = ev;
    if (ev->relatedTo()) {
      if (ev->relatedTo()->type()=="Todo") {
        if (rawTodoList.find(static_cast<Todo*>(ev->relatedTo()))<0) {
          rawTodoList.append(static_cast<Todo*>(ev->relatedTo()));
        }
      }
    }
    rawTodoList.find(subev);
    ev = rawTodoList.next();
  }

  // Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  for (int i=1; i<=5; ++i) {
    for(ev=rawTodoList.first();ev;ev=rawTodoList.next()) {
      if (ev->priority()==i && checkSecrecy( ev )) todoList.append(ev);
    }
  }

  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18n("Task") << "</th>\n";
  *ts << "    <th>" << i18n("Priority") << "</th>\n";
  *ts << "    <th>" << i18n("Completed") << "</th>\n";
  if (dueDateEnabled()) {
    *ts << "    <th>" << i18n("Due Date") << "</th>\n";
  }
  if (categoriesTodoEnabled()) {
    *ts << "    <th>" << i18n("Categories") << "</th>\n";
  }
  if (attendeesTodoEnabled()) {
    *ts << "    <th>" << i18n("Attendees") << "</th>\n";
  }
  *ts << "  </tr>\n";

  // Create top-level list.
  for(ev=todoList.first();ev;ev=todoList.next()) {
    if (!ev->relatedTo()) createHtmlTodo(ts,ev);
  }

  // Create sub-level lists
  for(ev=todoList.first();ev;ev=todoList.next()) {
    QPtrList<Incidence> relations = ev->relations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <tr>\n";
      *ts << "    <td class=\"subhead\" colspan=";
      int columns = 3;
      if (dueDateEnabled()) ++columns;
      if (categoriesTodoEnabled()) ++columns;
      if (attendeesTodoEnabled()) ++columns;
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><a name=\"sub" << ev->uid() << "\"></a>"
          << i18n("Sub-Tasks of: ") << "<a href=\"#"
          << ev->uid() << "\"><b>" << cleanChars(ev->summary()) << "</b></a></td>\n";
      *ts << "  </tr>\n";

      QPtrList<Todo> sortedList;
      Incidence *ev2;
      // Sort list by priorities. This is brute force and should be
      // replaced by a real sorting algorithm.
      for (int i=1; i<=5; ++i) {
        for(ev2=relations.first();ev2;ev2=relations.next()) {
          Todo *ev3 = dynamic_cast<Todo *>(ev2);
          if (ev3 && ev3->priority() == i) sortedList.append(ev3);
        }
      }

      for(subev=sortedList.first();subev;subev=sortedList.next()) {
        createHtmlTodo(ts,subev);
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createHtmlTodo (QTextStream *ts,Todo *todo)
{
  kdDebug() << "HtmlExport::createHtmlTodo()" << endl;

  bool completed = todo->isCompleted();
  QPtrList<Incidence> relations = todo->relations();

  *ts << "<tr>\n";

  *ts << "  <td class=\"sum\"";
  if (completed) *ts << "done";
  *ts << ">\n";
  *ts << "    <a name=\"" << todo->uid() << "\"></a>\n";
  *ts << "    <b>" << cleanChars(todo->summary()) << "</b>\n";
  if (!todo->description().isEmpty()) {
    *ts << "    <p>" << breakString(cleanChars(todo->description())) << "</p>\n";
  }
  if (relations.count()) {
    *ts << "    <div align=\"right\"><a href=\"#sub" << todo->uid()
        << "\">" << i18n("Sub-Tasks") << "</a></div>\n";
  }

  *ts << "  </td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";

  *ts << "  <td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";
  *ts << "    " << todo->priority() << "\n";
  *ts << "  </td>\n";

  *ts << "  <td";
  if (completed) *ts << " class=\"done\"";
  *ts << ">\n";
  *ts << "    " << i18n("%1 %").arg(todo->percentComplete()) << "\n";
  *ts << "  </td>\n";

  if (dueDateEnabled()) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    if (todo->hasDueDate()) {
      *ts << "    " << todo->dtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </td>\n";
  }

  if (categoriesTodoEnabled()) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatHtmlCategories(ts,todo);  
    *ts << "  </td>\n";  
  }

  if (attendeesTodoEnabled()) {
    *ts << "  <td";
    if (completed) *ts << " class=\"done\"";
    *ts << ">\n";
    formatHtmlAttendees(ts,todo);
    *ts << "  </td>\n";
  }

  *ts << "</tr>\n";
}

bool HtmlExport::checkSecrecy( Incidence *incidence )
{
  int secrecy = incidence->secrecy();
  if ( secrecy == Incidence::SecrecyPublic ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyPrivate && !excludePrivateEventEnabled() ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyConfidential &&
       !excludeConfidentialEventEnabled() ) {
    return true;
  }
  return false;
}

void HtmlExport::formatHtmlCategories (QTextStream *ts,Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    *ts << "    " << cleanChars(event->categoriesStr()) << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatHtmlAttendees (QTextStream *ts,Incidence *event)
{
  QPtrList<Attendee> attendees = event->attendees();
  if (attendees.count()) {
	  *ts << "<em>";
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self();
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail(event->organizer());
    KABC::Addressee o = addressList.first();
    if (!o.isEmpty() && addressList.size()<2) {
      *ts << "<a href=\"mailto:" << event->organizer() << "\">";
      *ts << cleanChars(o.formattedName()) << "</a>\n";
    }
		else *ts << event->organizer();
#else
	  *ts << event->organizer();
#endif
    *ts << "</em><br />";
    Attendee *a;
    for(a=attendees.first();a;a=attendees.next()) {
      if (!a->email().isEmpty()) {
				*ts << "<a href=\"mailto:" << a->email();
				*ts << "\">" << cleanChars(a->name()) << "</a>";
		  }
      else {
			  *ts << "    " << cleanChars(a->name());
		  }
      *ts << "<br />" << "\n";
    }
  } else {
    *ts << "    &nbsp;\n";
  }
}

QString HtmlExport::breakString(const QString &text)
{
  int number = text.contains("\n");
  if(number < 0) {
    return text;
  } else {
    QString out;
    QString tmpText = text;
    int pos = 0;
    QString tmp;
    for(int i=0;i<=number;i++) {
      pos = tmpText.find("\n");
      tmp = tmpText.left(pos);
      tmpText = tmpText.right(tmpText.length() - pos - 1);
      out += tmp + "<br />";
    }
    return out;
  }
}

QString HtmlExport::cleanChars(const QString &text)
{
  QString txt = text;
  txt = txt.replace( QRegExp("&"), "&amp;" );
  txt = txt.replace( QRegExp("<"), "&lt;" );
  txt = txt.replace( QRegExp(">"), "&gt;" );
  txt = txt.replace( QRegExp("\""), "&quot;" );
  txt = txt.replace( QRegExp("ä"), "&auml;" );
  txt = txt.replace( QRegExp("Ä"), "&Auml;" );
  txt = txt.replace( QRegExp("ö"), "&ouml;" );
  txt = txt.replace( QRegExp("Ö"), "&Ouml;" );
  txt = txt.replace( QRegExp("ü"), "&uuml;" );
  txt = txt.replace( QRegExp("Ü"), "&Uuml;" );
  txt = txt.replace( QRegExp("ß"), "&szlig;" );
  txt = txt.replace( QRegExp("¤"), "&euro;" );
  txt = txt.replace( QRegExp("é"), "&eacute;" );

  return txt;
}

void HtmlExport::setStyleSheet( const QString &styleSheet )
{
  mStyleSheet = styleSheet;
}

QString HtmlExport::styleSheet()
{
  if ( !mStyleSheet.isEmpty() ) return mStyleSheet;

  QString css;

  if ( QApplication::reverseLayout() ) {
    css += "    body { background-color:white; color:black; direction: rtl }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sumdone { background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.dateholiday { color:red }\n";
  } else {
    css += "    body { background-color:white; color:black }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sum { text-align:left }\n";
    css += "    td.sumdone { text-align:left; background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.date { text-align:left }\n";
    css += "    td.dateholiday { text-align:left; color:red }\n";
  }
  
  return css;
}
