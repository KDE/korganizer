// $Id$
// SearchDialog for KOrganizer
// (c) 1998 by Preston Brown

#include <qlayout.h>

#include <klocale.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>

#include "kdateedit.h"

#include "searchdialog.h"
#include "searchdialog.moc"

SearchDialog::SearchDialog(CalObject *calendar)
  : KDialogBase(Plain,i18n("Find Events"),User1|Close,User1,0,0,false,false,
                i18n("&Find"))
{
  mCalendar = calendar;

  QFrame *topFrame = plainPage();
  QVBoxLayout *layout = new QVBoxLayout(topFrame,0,spacingHint());

  // Search expression
  QHBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  searchLabel = new QLabel(topFrame);
  searchLabel->setText(i18n("Search for:"));
  subLayout->addWidget(searchLabel);

  searchEdit = new QLineEdit(topFrame);
  subLayout->addWidget(searchEdit);
  searchEdit->setText("*"); // Find all events by default
  searchEdit->setFocus();
 
  // Date range
  QHBoxLayout *rangeLayout = new QHBoxLayout;
  layout->addLayout(rangeLayout);
  
  rangeLayout->addWidget(new QLabel(i18n("Date range from"),topFrame));
  mStartDate = new KDateEdit(topFrame);
  rangeLayout->addWidget(mStartDate);
  rangeLayout->addWidget(new QLabel(i18n("to"),topFrame));
  mEndDate = new KDateEdit(topFrame);
  mEndDate->setDate(QDate::currentDate().addDays(365));
  rangeLayout->addWidget(mEndDate);

  // Results list view
  listView = new KOListView(mCalendar,topFrame);
  listView->setMinimumWidth(300);
  listView->setMinimumHeight(200);
  listView->showDates();
  layout->addWidget(listView);
 
  connect(this,SIGNAL(user1Clicked()),SLOT(doSearch()));

  // Propagate edit and delete event signals from event list view
  connect(listView,SIGNAL(showEventSignal(KOEvent *)),
	  SIGNAL(showEventSignal(KOEvent *)));
  connect(listView,SIGNAL(editEventSignal(KOEvent *)),
	  SIGNAL(editEventSignal(KOEvent *)));
  connect(listView,SIGNAL(deleteEventSignal(KOEvent *)),
	  SIGNAL(deleteEventSignal(KOEvent *)));
}

void SearchDialog::doSearch()
{
  QRegExp re;

  re.setWildcard(TRUE); // most people understand these better.
  re.setCaseSensitive(FALSE);
  re = searchEdit->text();
  if (!re.isValid()) {
    KMessageBox::sorry(this,
                       i18n("Invalid search expression, cannot perform\n"
                            "the search. Please enter a search expression\n"
                            "using the wildcard characters '*' and '?'\n"
                            "where needed."));
    return;
  }

  search(re);

  listView->selectEvents(mMatchedEvents);
  
  if (mMatchedEvents.count() == 0) {
    KMessageBox::information(this,
        i18n("No events were found matching your search expression."));
  }
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::updateView()
{
  QRegExp re;
  re.setWildcard(TRUE); // most people understand these better.
  re.setCaseSensitive(FALSE);
  re = searchEdit->text();
  if (re.isValid()) {
    search(re);
  }

  listView->selectEvents(mMatchedEvents);
}

void SearchDialog::search(const QRegExp &re)
{
//  mMatchedEvents = mCalendar(getEvents(mStartDate->getDate()),
//                                       mEndDate->getDate());

  mMatchedEvents = mCalendar->search(re);
}
