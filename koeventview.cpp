/*
 $Id$
*/

#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "calobject.h"
#include "calprinter.h"

#include "koeventview.h"
#include "koeventview.moc"

KOEventView::KOEventView(CalObject *cal,QWidget *parent,const char *name)
  : KOBaseView(cal,parent,name)
{
}

KOEventView::~KOEventView()
{
}

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu;
  
  connect (eventPopup,SIGNAL(editEventSignal(Event *)),
           SIGNAL(editEventSignal(Event *)));
  connect (eventPopup,SIGNAL(showEventSignal(Event *)),
           SIGNAL(showEventSignal(Event *)));
  connect (eventPopup,SIGNAL(deleteEventSignal(Event *)),
           SIGNAL(deleteEventSignal(Event *)));

  return eventPopup;
}

void KOEventView::showEventPopup(QPopupMenu *popup,Event *event)
{
  mCurrentEvent = event;
  if (event) popup->popup(QCursor::pos());
  else kdDebug() << "KOEventView::showEventPopup(): No event selected" << endl;
}

void KOEventView::popupShow()
{
  emit showEventSignal(mCurrentEvent);
}

void KOEventView::popupEdit()
{
  emit editEventSignal(mCurrentEvent);
}

void KOEventView::popupDelete()
{
  emit deleteEventSignal(mCurrentEvent);
}

void KOEventView::defaultEventAction(Event *event)
{
  if (event) {
    if (event->isReadOnly()) emit showEventSignal(event);
    else emit editEventSignal(event);
  }
}
