// $Id$

#include <qlayout.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kmessagebox.h>

#include "calendar.h"
#include "calprinter.h"
#include "calendarview.h"
#include "koeventviewerdialog.h"

#include "kowhatsnextview.h"
#include "kowhatsnextview.moc"

void WhatsNextTextBrowser::setSource(const QString& name)
{
  if (name.startsWith("event:")) {
    emit showIncidence(name);
    return;
  } else if (name.startsWith("todo:")) {
    emit showIncidence(name);
    return;
  } else {
    QTextBrowser::setSource(name);
  }
}

KOWhatsNextView::KOWhatsNextView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOBaseView(calendar, parent, name)
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

  QList<Event> events = mCalendar->getEvents(QDate::currentDate(),
                                             QDate::currentDate());
  if (events.count() > 0) {
    mText += i18n("<h2>Events:</h2>");
    mText += i18n("<table>");
    Event *ev = events.first();
    while(ev) {
      if (!ev->recurrence()->doesRecur() || ev->recursOn( QDate::currentDate())) {
        appendEvent(ev);
      }
      ev = events.next();
    }
    mText += i18n("</table>");
  }

  QList<Todo> todos = mCalendar->getTodoList();
  if (todos.count() > 0) {  
    mText += i18n("<h2>Todo:</h2>");
    mText += i18n("<ul>");
    Todo *todo = todos.first();
    while(todo) {
      if (!todo->isCompleted() && (todo->priority() == 1 ||
          (todo->hasDueDate() && todo->dtDue().date() == QDate::currentDate())))
        appendTodo(todo);
      todo = todos.next();
    }
    mText += i18n("</ul>");
  }

  mView->setText(mText);
}

void KOWhatsNextView::selectDates(const QDateList dateList)
{
  updateView();
}

void KOWhatsNextView::selectEvents(QList<Event> eventList)
{
}

void KOWhatsNextView::changeEventDisplay(Event *event, int action)
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

void KOWhatsNextView::appendEvent(Event *ev)
{
  mText += "<tr><td><b>";
  if (!ev->doesFloat()) {
    mText += ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
  }
  mText += "</b></td><td><a href=\"event:" + ev->VUID() + "\">";
  mText += ev->summary();
  mText += "</a></td></tr>";
}

void KOWhatsNextView::appendTodo(Todo *ev)
{
  mText += "<li><a href=\"todo:" + ev->VUID() + "\">";
  mText += ev->summary();
  mText += "</a></li>";
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
  if (uid.startsWith("event:")) {
    Event *event = mCalendar->getEvent(uid.mid(6));
    createEventViewer();
    mEventViewer->setEvent(event);
  } else if (uid.startsWith("todo:")) {
    Todo *todo = mCalendar->getTodo(uid.mid(5));
    createEventViewer();
    mEventViewer->setTodo(todo);
  }
  mEventViewer->show();
  mEventViewer->raise();
}
