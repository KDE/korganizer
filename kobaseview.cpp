/*
 $Id$
*/

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "calendar.h"
#include "calprinter.h"

#include "kobaseview.h"
#include "kobaseview.moc"

KOBaseView::KOBaseView(Calendar *cal, QWidget *parent, const char *name)
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
void KOBaseView::printPreview(CalPrinter *, const QDate &,
                              const QDate &)
{
  KMessageBox::sorry(this, i18n("Unfortunately, we don't handle printing for\n"
			    "that view yet.\n"));
}

void KOBaseView::print(CalPrinter *)
{
  KMessageBox::sorry(this, i18n("Unfortunately, we don't handle printing for\n"
			    "that view yet.\n"));
}
