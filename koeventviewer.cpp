// $Id$

#include <klocale.h>

#include "event.h"
#include "todo.h"

#include "koeventviewer.h"
#include "koeventviewer.moc"


KOEventViewer::KOEventViewer(QWidget *parent,const char *name)
  : QTextView(parent,name)
{
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::addTag(const QString & tag,const QString & text)
{
  QString str = "<" + tag + ">" + text + "</" + tag + ">";
  mText.append(str);
}

void KOEventViewer::appendEvent(Event *event)
{
  addTag("h1",event->summary());
  
  if (event->doesFloat()) {
    if (event->isMultiDay()) {
      mText.append(i18n("<b>From:</b> %1 <b>To:</b> %2")
                   .arg(event->dtStartDateStr())
                   .arg(event->dtEndDateStr()));
    } else {
      mText.append(i18n("<b>On:</b> %1").arg(event->dtStartDateStr()));
    }
  } else {
    if (event->isMultiDay()) {
      mText.append(i18n("<b>From:</b> %1 <b>To:</b> %2")
                   .arg(event->dtStartStr())
                   .arg(event->dtEndStr()));
    } else {
      mText.append(i18n("<b>On:</b> %1 <b>From:</b> %2 <b>To:</b> %3")
                   .arg(event->dtStartDateStr())
                   .arg(event->dtStartTimeStr())
                   .arg(event->dtEndTimeStr()));
    }
  }

  if (!event->description().isEmpty()) addTag("p",event->description());

  formatCategories(event);
  formatAttendees(event);

  if (event->recurrence()->doesRecur()) {
    addTag("p","<em>" + i18n("This is a recurring event.") + "</em>");
  }

  formatReadOnly(event);

  setText(mText);
}

void KOEventViewer::appendTodo(Todo *event)
{
  addTag("h1",event->summary());
  
  if (event->hasDueDate()) {
    mText.append(i18n("<b>Due on:</b> %1").arg(event->dtDueStr()));
  }

  if (!event->description().isEmpty()) addTag("p",event->description());  

  formatCategories(event);
  formatAttendees(event);

  mText.append(i18n("<p><b>Status:</b> %1<br><b>Priority:</b> %2</p>")
               .arg(event->statusStr())
               .arg(QString::number(event->priority())));

  formatReadOnly(event);

  setText(mText);
}

void KOEventViewer::formatCategories(Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    if (event->categories().count() == 1) {
      addTag("h2",i18n("Category"));
    } else {
      addTag("h2",i18n("Categories"));
    }
    addTag("p",event->categoriesStr());
  }
}

void KOEventViewer::formatAttendees(Incidence *event)
{
  QList<Attendee> attendees = event->attendees();
  if (attendees.count()) {
    addTag("h2",i18n("Attendees"));
    Attendee *a;
    mText.append("<ul>");
    for(a=attendees.first();a;a=attendees.next()) {
      QString str = a->getName();
      if (!a->getEmail().isEmpty()) str += " &lt;" + a->getEmail() + "&gt;";
      addTag("li",str);
    }
    mText.append("</ul>");
  }
}

void KOEventViewer::formatReadOnly(Incidence *event)
{
  if (event->isReadOnly()) {
    addTag("p","<em>(" + i18n("read-only") + ")</em>");
  }
}


void KOEventViewer::setTodo(Todo *event)
{
  clearEvents();
  appendTodo(event);
}

void KOEventViewer::setEvent(Event *event)
{
  clearEvents();
  appendEvent(event);
}

void KOEventViewer::clearEvents(bool now)
{
  mText = "";
  if (now) setText(mText);
}
