/*
 $Id$
*/

#include <qmessagebox.h>

#include <klocale.h>

#include "calobject.h"
#include "calprinter.h"

#include "kobaseview.h"
#include "kobaseview.moc"

KOBaseView::KOBaseView(CalObject *cal, QWidget *parent, const char *name)
  : QWidget(parent, name), calendar(cal)
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
