// $Id$

#include <qlayout.h>
#include <qtextview.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "calobject.h"
#include "calprinter.h"
#include "calendarview.h"

#include "kowhatsnextview.h"
#include "kowhatsnextview.moc"


KOWhatsNextView::KOWhatsNextView(CalObject *calendar, QWidget *parent,
                                 const char *name)
  : KOBaseView(calendar, parent, name)
{
  mView = new QTextView(this);

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mView);
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

QList<Incidence> KOWhatsNextView::getSelected()
{
  QList<Incidence> eventList;

  return eventList;
}


void KOWhatsNextView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  calPrinter->preview(CalPrinter::Day, fd, td);
}

void KOWhatsNextView::updateView()
{
  mText = i18n("<h1>What's next?</h1>");


  QList<KOEvent> events = mCalendar->getEvents(QDate::currentDate(),
                                               QDate::currentDate().addDays(1));
  if (events.count() > 0) {
    mText += i18n("<h2>Events:</h2>");
    mText += i18n("<table>");
    KOEvent *ev = events.first();
    while(ev) {
      appendEvent(ev);
      ev = events.next();
    }
    mText += i18n("</table>");
  }

  QList<Todo> todos = mCalendar->getTodoList();
  if (todos.count() > 0) {  
    mText += i18n("<h2>Todo:</h2>");
    mText += i18n("<table>");
    Todo *ev = todos.first();
    while(ev) {
      if (ev->priority() == 1 ||
          (ev->hasDueDate() && ev->dtDue().date() == QDate::currentDate()))
        appendTodo(ev);
      ev = todos.next();
    }
    mText += i18n("</table>");
  }

  mView->setText(mText);
}

void KOWhatsNextView::selectDates(const QDateList dateList)
{
  updateView();
}

void KOWhatsNextView::selectEvents(QList<KOEvent> eventList)
{
}

void KOWhatsNextView::changeEventDisplay(KOEvent *event, int action)
{
  switch(action) {
    case CalendarView::EVENTADDED:
      break;
    case CalendarView::EVENTEDITED:
      break;
    case CalendarView::EVENTDELETED:
      break;
    default:
      kdDebug() << "KOWhatsNextView::changeEventDisplay(): Illegal action " << action << endl;
  }
}

void KOWhatsNextView::appendEvent(KOEvent *ev)
{
  mText += "<tr><td><b>";
  if (!ev->doesFloat()) {
    mText += ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
  }
  mText += "</b></td><td>";
  mText += ev->summary();
  mText += "</td></tr>";
}

void KOWhatsNextView::appendTodo(Todo *ev)
{
  mText += "<tr><td><b>";
  mText += QString::number(ev->priority());
  mText += "</b></td><td>";
  mText += ev->summary();
  mText += "</td></tr>";
}
