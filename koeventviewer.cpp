// $Id$

#include <klocale.h>

#include "koevent.h"

#include "koeventviewer.h"
#include "koeventviewer.moc"


KOEventViewer::KOEventViewer(QWidget *parent,const char *name)
  : QTextView(parent,name)
{
//  setTextFormat(PlainText);
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::addTag(const QString & tag,const QString & text)
{
  QString str = "<" + tag + ">" + text + "</" + tag + ">";
  mText.append(str);
}

void KOEventViewer::appendEvent(KOEvent *event)
{
  addTag("h1",event->getSummary());
  
  if (event->doesFloat()) {
    if (event->isMultiDay()) {
      addTag("b",i18n("From: "));
      mText.append(event->getDtStartDateStr());
      addTag("b",i18n(" To: "));
      mText.append(event->getDtEndDateStr());
    } else {
      addTag("b","On: ");
      mText.append(event->getDtStartDateStr());
    }
  } else {
    if (event->isMultiDay()) {
      addTag("b",i18n("From: "));
      mText.append(event->getDtStartDateStr() + " ");
      mText.append(event->getDtStartTimeStr());
      addTag("b",i18n(" To: "));
      mText.append(event->getDtEndDateStr() + " ");
      mText.append(event->getDtEndTimeStr());
    } else {
      addTag("b","On: ");
      mText.append(event->getDtStartDateStr());
      addTag("b",i18n(" From: "));
      mText.append(event->getDtStartTimeStr());
      addTag("b",i18n(" To: "));
      mText.append(event->getDtEndTimeStr());
    }
  }

  if (!event->getDescription().isEmpty()) addTag("p",event->getDescription());

  setText(mText);
}

void KOEventViewer::setTodo(KOEvent *event)
{
  mText = "";

  addTag("h1",event->getSummary());
  
  if (!event->getDescription().isEmpty()) addTag("p",event->getDescription());  

  setText(mText);
}

void KOEventViewer::setEvent(KOEvent *event)
{
  clearEvents();
  appendEvent(event);
}

void KOEventViewer::clearEvents(bool now)
{
  mText = "";
  if(now) setText(mText);
}

KOEventViewerDialog::KOEventViewerDialog(QWidget *parent,const char *name)
  : KDialogBase(parent,name,false,i18n("Event Viewer"),Ok,Ok,false,
                i18n("Edit"))
{
  mEventViewer = new KOEventViewer(this);
  setMainWidget(mEventViewer);

  setMinimumSize(300,200);
}

KOEventViewerDialog::~KOEventViewerDialog()
{
}

void KOEventViewerDialog::setEvent(KOEvent *event)
{
  mEventViewer->setEvent(event);
}

void KOEventViewerDialog::setTodo(KOEvent *event)
{
  mEventViewer->setTodo(event);
}
