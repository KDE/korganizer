/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <libkdepim/kdateedit.h>

#include "koglobals.h"
#include "koprefs.h"

#include "searchdialog.h"
#include "searchdialog.moc"

SearchDialog::SearchDialog(Calendar *calendar,QWidget *parent)
  : KDialogBase(Plain,i18n("Find Events"),User1|Close,User1,parent,0,false,false,
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
  connect(searchEdit, SIGNAL(textChanged ( const QString & )),this,SLOT(searchTextChanged( const QString & )));

  // Date range
  QGroupBox *rangeGroup = new QGroupBox(1,Horizontal,i18n("Date Range"),
                                        topFrame);
  layout->addWidget(rangeGroup);

  QWidget *rangeWidget = new QWidget(rangeGroup);
  QHBoxLayout *rangeLayout = new QHBoxLayout(rangeWidget,0,spacingHint());

  rangeLayout->addWidget(new QLabel(i18n("From:"),rangeWidget));
  mStartDate = new KDateEdit(rangeWidget);
  rangeLayout->addWidget(mStartDate);
  rangeLayout->addWidget(new QLabel(i18n("To:"),rangeWidget));
  mEndDate = new KDateEdit(rangeWidget);
  mEndDate->setDate(QDate::currentDate().addDays(365));
  rangeLayout->addWidget(mEndDate);

  mInclusiveCheck = new QCheckBox(i18n("Events have to be completely included"),
                                  rangeGroup);
  mInclusiveCheck->setChecked(false);

  // Subjects to search
  QGroupBox *subjectGroup = new QGroupBox(1,Vertical,i18n("Search In"),
                                          topFrame);
  layout->addWidget(subjectGroup);

  mSummaryCheck = new QCheckBox(i18n("Summaries"),subjectGroup);
  mSummaryCheck->setChecked(true);
  mDescriptionCheck = new QCheckBox(i18n("Descriptions"),subjectGroup);
  mCategoryCheck = new QCheckBox(i18n("Categories"),subjectGroup);

  // Results list view
  listView = new KOListView(mCalendar,topFrame);
  listView->showDates();
  layout->addWidget(listView);

  if ( KOPrefs::instance()->mCompactDialogs ) {
    KOGlobals::fitDialogToScreen( this, true );
  }

  connect(this,SIGNAL(user1Clicked()),SLOT(doSearch()));

  // Propagate edit and delete event signals from event list view
  connect(listView,SIGNAL(showEventSignal(Event *)),
	  SIGNAL(showEventSignal(Event *)));
  connect(listView,SIGNAL(editEventSignal(Event *)),
	  SIGNAL(editEventSignal(Event *)));
  connect(listView,SIGNAL(deleteEventSignal(Event *)),
	  SIGNAL(deleteEventSignal(Event *)));
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::searchTextChanged( const QString &_text )
{
  enableButton( KDialogBase::User1, !_text.isEmpty() );
}

void SearchDialog::doSearch()
{
  QRegExp re;

  re.setWildcard(true); // most people understand these better.
  re.setCaseSensitive(false);
  re.setPattern(searchEdit->text());
  if (!re.isValid()) {
    KMessageBox::sorry(this,
                       i18n("Invalid search expression, cannot perform "
                            "the search. Please enter a search expression "
                            "using the wildcard characters '*' and '?' "
                            "where needed."));
    return;
  }

  search(re);

  listView->showEvents(mMatchedEvents);

  if (mMatchedEvents.count() == 0) {
    KMessageBox::information(this,
        i18n("No events were found matching your search expression."));
  }
}

void SearchDialog::updateView()
{
  QRegExp re;
  re.setWildcard(true); // most people understand these better.
  re.setCaseSensitive(false);
  re.setPattern(searchEdit->text());
  if (re.isValid()) {
    search(re);
  } else {
    mMatchedEvents.clear();
  }

  listView->showEvents(mMatchedEvents);
}

void SearchDialog::search(const QRegExp &re)
{
  Event::List events = mCalendar->events( mStartDate->date(),
                                          mEndDate->date(),
                                          mInclusiveCheck->isChecked() );

  mMatchedEvents.clear();
  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    Event *ev = *it;
    if (mSummaryCheck->isChecked()) {
#if QT_VERSION >= 300
      if (re.search(ev->summary()) != -1) {
#else
      if (re.match(ev->summary()) != -1) {
#endif
        mMatchedEvents.append(ev);
        continue;
      }
    }
    if (mDescriptionCheck->isChecked()) {
#if QT_VERSION >= 300
      if (re.search(ev->description()) != -1) {
#else
      if (re.match(ev->description()) != -1) {
#endif
        mMatchedEvents.append(ev);
        continue;
      }
    }
    if (mCategoryCheck->isChecked()) {
#if QT_VERSION >= 300
      if (re.search(ev->categoriesStr()) != -1) {
#else
      if (re.match(ev->categoriesStr()) != -1) {
#endif
        mMatchedEvents.append(ev);
        continue;
      }
    }
  }
}
