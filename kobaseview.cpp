/*
 $Id$
*/

#include <qmessagebox.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kiconloader.h>

#include "calobject.h"
#include "calprinter.h"

#include "kobaseview.h"
#include "kobaseview.moc"

KOBaseView::KOBaseView(CalObject *cal, QWidget *parent, const char *name)
  : QWidget(parent, name), mCalendar(cal)
{
}

KOBaseView::~KOBaseView()
{
}

/*
  The date parameters should be determined by the view itself and not given as
  parameters. At the moment I just move the code from the topwidget to the
  individual views.
*/
void KOBaseView::printPreview(CalPrinter *calPrinter, const QDate &,
                              const QDate &)
{
  QMessageBox::warning(this,i18n("KOrganizer error"),
		       i18n("Unfortunately, we don't handle printing for\n"
			    "that view yet.\n"));
}
  
void KOBaseView::print(CalPrinter *calPrinter)
{
  QMessageBox::warning(this,i18n("KOrganizer error"),
		       i18n("Unfortunately, we don't handle printing for\n"
			    "that view yet.\n"));
}

QPopupMenu *KOBaseView::eventPopup()
{
  QPopupMenu *eventPopup = new QPopupMenu();
  eventPopup->insertItem (i18n("&Show"),this,SLOT(popupShow()));
  eventPopup->insertItem (i18n("&Edit"),this, SLOT(popupEdit()));
  eventPopup->insertItem (SmallIcon("delete"),i18n("&Delete"),
                          this, SLOT(popupDelete()));

  return eventPopup;
}

void KOBaseView::showEventPopup(QPopupMenu *popup,KOEvent *event)
{
  mCurrentEvent = event;
  if (event) popup->popup(QCursor::pos());
  else qDebug("KOBaseView::showEventPopup(): No event selected");
}

void KOBaseView::popupShow()
{
  emit showEventSignal(mCurrentEvent);
}

void KOBaseView::popupEdit()
{
  emit editEventSignal(mCurrentEvent);
}

void KOBaseView::popupDelete()
{
  emit deleteEventSignal(mCurrentEvent);
}

void KOBaseView::defaultEventAction(KOEvent *event)
{
  emit editEventSignal(event);
}
