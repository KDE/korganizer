// $Id$
// quick event viewer

#include <klocale.h>

#include "kdpquickevt.h"
#include "kdpquickevt.moc"

KDPQuickEventView::KDPQuickEventView(CalObject *cal, 
				     QWidget *parent, char *name )
	: QDialog(parent, name, FALSE)
{
  done        = new QPushButton(i18n("Done"), this);
  done->adjustSize();
  quickdialog = new QuickDialog(cal, this);
  quickdialog->move(2,2);
  connect(quickdialog, SIGNAL(eventChanged()), this, SIGNAL(eventChanged()));
  connect(done, SIGNAL(clicked()), this, SLOT(accept()));
}

KDPQuickEventView::~KDPQuickEventView(){};

void KDPQuickEventView::updateDialog()
{
  quickdialog->updateDialog();
}

void KDPQuickEventView::setSelected( KOEvent *ev )
{
  quickdialog->setSelected(ev);
}

void KDPQuickEventView::resizeEvent(QResizeEvent *)
{
  done->move(width()-done->width()-2, height()-done->height()-2);
  quickdialog->resize(width()-4, height()-4-done->height()-5);
}
