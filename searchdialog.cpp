// $Id$
// SearchDialog for KOrganizer
// (c) 1998 by Preston Brown

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>

#include "kdateedit.h"

#include "searchdialog.h"
#include "searchdialog.moc"

SearchDialog::SearchDialog(Calendar *calendar)
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
  QGroupBox *rangeGroup = new QGroupBox(1,Horizontal,i18n("Date Range"),
                                        topFrame);
  layout->addWidget(rangeGroup);

  QWidget *rangeWidget = new QWidget(rangeGroup);
  QHBoxLayout *rangeLayout = new QHBoxLayout(rangeWidget,0,spacingHint());

  rangeLayout->addWidget(new QLabel(i18n("Date range from"),rangeWidget));
  mStartDate = new KDateEdit(rangeWidget);
  rangeLayout->addWidget(mStartDate);
  rangeLayout->addWidget(new QLabel(i18n("to"),rangeWidget));
  mEndDate = new KDateEdit(rangeWidget);
  mEndDate->setDate(QDate::currentDate().addDays(365));
  rangeLayout->addWidget(mEndDate);

  mInclusiveCheck = new QCheckBox(i18n("Events have to be completely included"),
                                  rangeGroup);
  mInclusiveCheck->setChecked(false);

  // Subjects to search
  QGroupBox *subjectGroup = new QGroupBox(1,Vertical,i18n("Search In:"),
                                          topFrame);
  layout->addWidget(subjectGroup);

  mSummaryCheck = new QCheckBox(i18n("Summaries"),subjectGroup);
  mSummaryCheck->setChecked(true);
  mDescriptionCheck = new QCheckBox(i18n("Descriptions"),subjectGroup);
  mCategoryCheck = new QCheckBox(i18n("Categories"),subjectGroup);

  // Results list view
  listView = new KOListView(mCalendar,topFrame);
//  listView->setMinimumWidth(300);
//  listView->setMinimumHeight(200);
  listView->showDates();
  layout->addWidget(listView);

  connect(this,SIGNAL(user1Clicked()),SLOT(doSearch()));

  // Propagate edit and delete event signals from event list view
  connect(listView,SIGNAL(showEventSignal(Event *)),
	  SIGNAL(showEventSignal(Event *)));
  connect(listView,SIGNAL(editEventSignal(Event *)),
	  SIGNAL(editEventSignal(Event *)));
  connect(listView,SIGNAL(deleteEventSignal(Event *)),
	  SIGNAL(deleteEventSignal(Event *)));
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
  } else {
    mMatchedEvents.clear();
  }

  listView->selectEvents(mMatchedEvents);
}

void SearchDialog::search(const QRegExp &re)
{
  QList<Event> events = mCalendar->getEvents(mStartDate->getDate(),
                                               mEndDate->getDate(),
                                               mInclusiveCheck->isChecked());

  mMatchedEvents.clear();
  Event *ev;
  for(ev=events.first();ev;ev=events.next()) {
    if (mSummaryCheck->isChecked()) {
      if (re.match(ev->summary()) != -1) {
        mMatchedEvents.append(ev);
        continue;
      }
    }
    if (mDescriptionCheck->isChecked()) {
      if (re.match(ev->description()) != -1) {
        mMatchedEvents.append(ev);
        continue;
      }
    }
    if (mCategoryCheck->isChecked()) {
      if (re.match(ev->categoriesStr()) != -1) {
        mMatchedEvents.append(ev);
        continue;
      }
    }
  }
}
