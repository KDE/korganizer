// $Id$

#include <klocale.h>

#include "event.h"
#include "koeventviewer.h"

#include "koeventviewerdialog.h"
#include "koeventviewerdialog.moc"


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

void KOEventViewerDialog::setEvent(Event *event)
{
  mEventViewer->setEvent(event);
}

void KOEventViewerDialog::setTodo(Todo *event)
{
  mEventViewer->setTodo(event);
}
