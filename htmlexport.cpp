// $Id$

#include "htmlexport.h"

#include <qfile.h>
#include <qtextstream.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "calendar.h"
#include "event.h"
#include "todo.h"
#include "kocore.h"

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
  // Write HTML header
  *ts << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n";

  *ts << "<HTML><HEAD>" << endl;
  *ts << "  <META http-equiv=\"Content-Type\" content=\"text/html; charset=" + KGlobal::locale()->charset() +  "\">\n";
  *ts << "  <TITLE>" << i18n("KOrganizer To-Do List") << "</TITLE>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << "    body { background-color:white; color:black }\n";
  *ts << "    td { text-align:center; background-color:#eee }\n";
  *ts << "    th { text-align:center; background-color:#228; color:white }\n";
  *ts << "    td.sum { text-align:left }\n";
  *ts << "    td.sumdone { text-align:left; background-color:#ccc }\n";
  *ts << "    td.done { background-color:#ccc }\n";
  *ts << "    td.subhead { text-align:center; background-color:#ccf }\n";
  *ts << "    td.datehead { text-align:center; background-color:#ccf }\n";
  *ts << "    td.space { background-color:white }\n";
  *ts << "    td.date { text-align:left }\n";
  *ts << "    td.dateholiday { text-align:left; color:red }\n";
  *ts <<   "</style>\n";
  *ts << "</HEAD><BODY>\n";

  // TO DO: Write KOrganizer header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if (eventsEnabled() || monthViewEnabled()) {
    *ts << "<H1>" << i18n("KOrganizer Calendar") << "</H1>\n";
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
    *ts << "<H1>" << i18n("KOrganizer To-Do List") << "</H1>\n";

    // Write HTML page content
    createHtmlTodoList(ts);
  }

  // Write KOrganizer trailer
  *ts << "<P>" << i18n("This page was created by <A HREF=\"http://"
        "korganizer.kde.org\">KOrganizer</A>") << "</P>\n";

  // Write HTML trailer
  *ts << "</BODY></HTML>\n";

  return true;
}

void HtmlExport::createHtmlMonthView(QTextStream *ts)
{
  QDate start = fromDate();
  start.setYMD(start.year(),start.month(),1);  // go back to first day in month

  QDate end(start.year(),start.month(),start.daysInMonth());

  if (KGlobal::locale()->weekStartsMonday()) {
    start = start.addDays(1 - start.dayOfWeek());
  } else {
    if (start.dayOfWeek() != 7) {
      start = start.addDays(-start.dayOfWeek());
    }
  }

  // Write header
  *ts << "<h2>" << (i18n("month_year","%1 %2").arg(KGlobal::locale()->monthName(fromDate().month()))
      .arg(fromDate().year())) << "</h2>\n";
  *ts << "<table border=1>\n";

  // Write table header
  *ts << "  <tr>";
  for(int i=0; i<7; ++i) {
    *ts << "<th>" << KGlobal::locale()->weekDayName(start.addDays(i).dayOfWeek()) << "</th>";
  }
  *ts << "</tr>\n";
  
  // Write days
  while (start <= end) {
    *ts << "<tr>\n";
    for(int i=0;i<7;++i) {
      *ts << "<td valign=top><table border=0>\n";

      QString holiday = KOCore::self()->holiday(start);

      *ts << "<tr><td ";
      if (!holiday.isEmpty() || start.dayOfWeek() == 7) *ts << "class=dateholiday";
      else *ts << "class=date";
      *ts << ">" << QString::number(start.day());

      if (!holiday.isEmpty()) {
        *ts << " <em>" << holiday << "</em>";
      }
      
      *ts << "</td></tr>\n<tr><td valign=top>";
      
      QList<Event> events = mCalendar->getEventsForDate(start,true);
      if (events.count()) {
        *ts << "<table>";
        Event *ev;
        for(ev = events.first(); ev; ev = events.next()) {
          createHtmlEvent(ts,ev,start,false);
        }
        *ts << "</table>";
      } else {
        *ts << "&nbsp;";
      }

      *ts << "</td></tr></table>\n";
      start = start.addDays(1);
    }
    *ts << "</tr>\n";
  }
  
  *ts << "</table>\n";
}

void HtmlExport::createHtmlEventList (QTextStream *ts)
{
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>" << i18n("Start Time") << "</TH>\n";
  *ts << "    <TH>" << i18n("End Time") << "</TH>\n";
  *ts << "    <TH>" << i18n("Event") << "</TH>\n";
  if (categoriesEventEnabled()) {
    *ts << "    <TH>" << i18n("Categories") << "</TH>\n";
  }
  if (attendeesEventEnabled()) {
    *ts << "    <TH>" << i18n("Attendees") << "</TH>\n";
  }

  *ts << "  </TR>\n";

  int columns = 3;
  if (categoriesEventEnabled()) ++columns;
  if (attendeesEventEnabled()) ++columns;

  for (QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1)) {
    kdDebug() << "Getting events for " << dt.toString() << endl;
    QList<Event> events = mCalendar->getEventsForDate(dt,true);
    if (events.count()) {
      *ts << "  <TR><TD COLSPAN=" << QString::number(columns)
          << " CLASS=datehead><I>"
          << KGlobal::locale()->formatDate(dt)
          << "</I></TD></TR>\n";
      Event *ev;
      for(ev = events.first(); ev; ev = events.next()) {
        createHtmlEvent(ts,ev,dt);
      }
    }
  }
  
  *ts << "</TABLE>\n";  
}

void HtmlExport::createHtmlEvent (QTextStream *ts, Event *event,
                                       QDate date,bool withDescription)
{
  kdDebug() << "HtmlExport::createHtmlEvent(): " << event->summary() << endl;
  *ts << "  <TR>\n";

  if (!event->doesFloat()) {
    if (event->isMultiDay() && (event->dtStart().date() != date)) {
      *ts << "    <TD>&nbsp;</TD>\n";
    } else {
      *ts << "    <TD valign=top>" << event->dtStartTimeStr() << "</TD>\n";
    }
    if (event->isMultiDay() && (event->dtEnd().date() != date)) {
      *ts << "    <TD>&nbsp;</TD>\n";
    } else {
      *ts << "    <TD valign=top>" << event->dtEndTimeStr() << "</TD>\n";
    }
  } else {
    *ts << "    <TD>&nbsp;</TD><TD>&nbsp;</TD>\n";
  }

  *ts << "    <TD CLASS=sum>\n";
  *ts << "      <B>" << event->summary() << "</B>\n";
  if (withDescription && !event->description().isEmpty()) {
    *ts << "      <P>" << breakString(event->description()) << "</P>\n";
  }
  *ts << "    </TD>\n";
  
  if (categoriesEventEnabled()) {
    *ts << "  <TD>\n";
    formatHtmlCategories(ts,event);  
    *ts << "  </TD>\n";  
  }

  if (attendeesEventEnabled()) {
    *ts << "  <TD>\n";
    formatHtmlAttendees(ts,event);
    *ts << "  </TD>\n";
  }
  
  *ts << "  </TR>\n";
}

void HtmlExport::createHtmlTodoList (QTextStream *ts)
{
  Todo *ev,*subev;
  
  QList<Todo> rawTodoList = mCalendar->getTodoList();
  QList<Todo> todoList;

  // Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  for (int i=1; i<=5; ++i) {
    for(ev=rawTodoList.first();ev;ev=rawTodoList.next()) {
      if (ev->priority() == i) todoList.append(ev);
    }
  }
  
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>" << i18n("Task") << "</TH>\n";
  *ts << "    <TH>" << i18n("Priority") << "</TH>\n";
  *ts << "    <TH>" << i18n("Completed") << "</TH>\n";
  if (dueDateEnabled()) {
    *ts << "    <TH>" << i18n("Due Date") << "</TH>\n";
  }
  if (categoriesTodoEnabled()) {
    *ts << "    <TH>" << i18n("Categories") << "</TH>\n";
  }
  if (attendeesTodoEnabled()) {
    *ts << "    <TH>" << i18n("Attendees") << "</TH>\n";
  }
  *ts << "  </TR>\n";

  // Create top-level list.
  for(ev=todoList.first();ev;ev=todoList.next()) {
    if (!ev->relatedTo()) createHtmlTodo(ts,ev);
  }

  // Create sub-level lists
  for(ev=todoList.first();ev;ev=todoList.next()) {
    QList<Incidence> relations = ev->relations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <TR>\n";
      *ts << "    <TD CLASS=subhead COLSPAN=";
      int columns = 3;
      if (dueDateEnabled()) ++columns;
      if (categoriesTodoEnabled()) ++columns;
      if (attendeesTodoEnabled()) ++columns;
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><A NAME=\"sub" << ev->VUID() << "\"></A>"
          << i18n("Sub-Tasks of: ") << "<A HREF=\"#"
          << ev->VUID() << "\"><B>" << ev->summary() << "</B></A></TD>\n";
      *ts << "  </TR>\n";
      
      QList<Todo> sortedList;
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

  *ts << "</TABLE>\n";
}

void HtmlExport::createHtmlTodo (QTextStream *ts,Todo *todo)
{
  kdDebug() << "HtmlExport::createHtmlTodo()" << endl;

  bool completed = todo->isCompleted();
  QList<Incidence> relations = todo->relations();

  *ts << "<TR>\n";

  *ts << "  <TD CLASS=sum";
  if (completed) *ts << "done";
  *ts << ">\n";
  *ts << "    <A NAME=\"" << todo->VUID() << "\"></A>\n";
  *ts << "    <B>" << todo->summary() << "</B>\n";
  if (!todo->description().isEmpty()) {
    *ts << "    <P>" << breakString(todo->description()) << "</P>\n";
  }
  if (relations.count()) {
    *ts << "    <DIV ALIGN=right><A HREF=\"#sub" << todo->VUID()
        << "\">" << i18n("Sub-Tasks") << "</A></DIV>\n";
  }

  *ts << "  </TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << todo->priority() << "\n";
  *ts << "  </TD>\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << i18n("%1 %").arg(todo->percentComplete()) << "\n";
  *ts << "  </TD>\n";

  if (dueDateEnabled()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    if (todo->hasDueDate()) {
      *ts << "    " << todo->dtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </TD>\n";
  }

  if (categoriesTodoEnabled()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    formatHtmlCategories(ts,todo);  
    *ts << "  </TD>\n";  
  }

  if (attendeesTodoEnabled()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    formatHtmlAttendees(ts,todo);
    *ts << "  </TD>\n";
  }

  *ts << "</TR>\n";
}

void HtmlExport::formatHtmlCategories (QTextStream *ts,Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    *ts << "    " << event->categoriesStr() << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatHtmlAttendees (QTextStream *ts,Incidence *event)
{
  QList<Attendee> attendees = event->attendees();
  if (attendees.count()) {
    Attendee *a;
    for(a=attendees.first();a;a=attendees.next()) {
      *ts << "    " << a->name();
      if (!a->email().isEmpty()) *ts << " &lt;" << a->email() << "&gt;";
      *ts << "<BR>" << "\n";
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
      out += tmp + "<br>";
    }
    return out;
  }
}
