/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/

// $Id$

#include <qlayout.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qlabel.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/calendar.h>

#include "calprinter.h"
#include "koglobals.h"
#include "koeventviewerdialog.h"

#include "kowhatsnextview.h"
#include "kowhatsnextview.moc"

void WhatsNextTextBrowser::setSource(const QString& n)
{
  kdDebug() << "WhatsNextTextBrowser::setSource(): " << n << endl;

  if (n.startsWith("event:")) {
    emit showIncidence(n);
    return;
  } else if (n.startsWith("todo:")) {
    emit showIncidence(n);
    return;
  } else {
    QTextBrowser::setSource(n);
  }
}

KOWhatsNextView::KOWhatsNextView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  QLabel *dateLabel =
      new QLabel(KGlobal::locale()->formatDate(QDate::currentDate()),this);
  dateLabel->setMargin(2);
  dateLabel->setAlignment(AlignCenter);

  mView = new WhatsNextTextBrowser(this);
  connect(mView,SIGNAL(showIncidence(const QString &)),SLOT(showIncidence(const QString &)));

  mEventViewer = 0;

  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(dateLabel);
  topLayout->addWidget(mView);
}

KOWhatsNextView::~KOWhatsNextView()
{
}

int KOWhatsNextView::maxDatesHint()
{
  return 0;
}

int KOWhatsNextView::currentDateCount()
{
  return 0;
}

QPtrList<Incidence> KOWhatsNextView::selectedIncidences()
{
  QPtrList<Incidence> eventList;

  return eventList;
}


void KOWhatsNextView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  calPrinter->preview(CalPrinter::Day, fd, td);
}

void KOWhatsNextView::updateView()
{
  mText = i18n("<table width=\"100%\">\n");
  mText += i18n("<tr bgcolor=\"#3679AD\"><td>");
  mText += i18n("<font color=\"white\"><h1>What's next?</h1></font>");
  mText += i18n("</td></tr>\n<tr><td>");

  QPtrList<Event> events = calendar()->getEvents(QDate::currentDate(),
                                                 QDate::currentDate());
  if (events.count() > 0) {
    mText += i18n("<p></p><h2>Events:</h2>\n");
    mText += i18n("<table>\n");
    Event *ev = events.first();
    while(ev) {
      if (!ev->recurrence()->doesRecur() || ev->recursOn( QDate::currentDate())) {
        appendEvent(ev);
      }
      ev = events.next();
    }
    mText += i18n("</table>\n");
  }

  QPtrList<Todo> todos = calendar()->getTodoList();
  if (todos.count() > 0) {
    mText += i18n("<h2>Todo:</h2>\n");
    mText += i18n("<ul>\n");
    Todo *todo = todos.first();
    while(todo) {
      if (!todo->isCompleted() && (todo->priority() == 1 ||
          (todo->hasDueDate() && todo->dtDue().date() == QDate::currentDate())))
        appendTodo(todo);
      todo = todos.next();
    }
    mText += i18n("</ul>\n");
  }

  mText += i18n("</td></tr>\n</table>\n");

  kdDebug() << "KOWhatsNextView::updateView: text: " << mText << endl;
  mView->setText(mText);
}

void KOWhatsNextView::showDates(const QDate &, const QDate &)
{
  updateView();
}

void KOWhatsNextView::showEvents(QPtrList<Event>)
{
}

void KOWhatsNextView::changeEventDisplay(Event *, int action)
{
  switch(action) {
    case KOGlobals::EVENTADDED:
      break;
    case KOGlobals::EVENTEDITED:
      break;
    case KOGlobals::EVENTDELETED:
      break;
    default:
      kdDebug() << "KOWhatsNextView::changeEventDisplay(): Illegal action " << action << endl;
  }
}

void KOWhatsNextView::appendEvent(Event *ev)
{
  kdDebug() << "KOWhatsNextView::appendEvent(): " << ev->VUID() << endl;

  mText += "<tr><td><b>";
  if (!ev->doesFloat()) {
    mText += ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
  }
  mText += "</b></td><td><a href=\"event:" + ev->VUID() + "\">";
  mText += ev->summary();
  mText += "</a></td></tr>\n";
}

void KOWhatsNextView::appendTodo(Todo *ev)
{
  mText += "<li><a href=\"todo:" + ev->VUID() + "\">";
  mText += ev->summary();
  mText += "</a></li>\n";
}

void KOWhatsNextView::createEventViewer()
{
  if (!mEventViewer) {
    mEventViewer = new KOEventViewerDialog(this);
  }
}

// TODO: Create this function in CalendarView and remove it from here
void KOWhatsNextView::showIncidence(const QString &uid)
{
  kdDebug() << "KOWhatsNextView::showIncidence(): " << uid << endl;

  if (uid.startsWith("event://")) {
    Event *event = calendar()->getEvent(uid.mid(8));
    if (!event) return;
    createEventViewer();
    mEventViewer->setEvent(event);
  } else if (uid.startsWith("todo://")) {
    Todo *todo = calendar()->getTodo(uid.mid(7));
    if (!todo) return;
    createEventViewer();
    mEventViewer->setTodo(todo);
  }
  mEventViewer->show();
  mEventViewer->raise();
}
